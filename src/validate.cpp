/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>

#include "ValStats.h"
#include "ValProfile.h"

#include "validate.h"
#include "valint.h"
#include "validateLIN.h"
#include "validatePBN.h"
#include "validateRBN.h"
#include "validateTXT.h"
#include "validateEML.h"
#include "validateREC.h"
#include "parse.h"
#include "Buffer.h"
#include "Bexcept.h"


using namespace std;


bool isRecordComment(
  const string& lineOut,
  const string& lineRef)
{
  // Detect Pavlicek bug with wrong record numbers.

  if (lineOut == "" || lineRef == "")
    return false;

  if (lineOut.at(0) != '%' || lineRef.at(0) != '%')
    return false;
  
  regex re("(\\d+) records (\\d+) deals");
  smatch match;
  unsigned r1, d1, r2, d2;

  if (! regex_search(lineOut, match, re))
    return false;

  if (! str2unsigned(match.str(1), r1)) return false;
  if (! str2unsigned(match.str(2), d1)) return false;

  if (! regex_search(lineRef, match, re))
    return false;

  if (! str2unsigned(match.str(1), r2)) return false;
  if (! str2unsigned(match.str(2), d2)) return false;

  if (r2 <= r1 || d2 <= d1)
    return false;

  return true;
}


enum refFixType
{
  BRIDGE_REF_INSERT = 0,
  BRIDGE_REF_REPLACE = 1,
  BRIDGE_REF_DELETE = 2
};

struct RefFix
{
  unsigned lno; // First line is 1
  refFixType type;
  string value;
  unsigned count;
};


static void readRefFix(
  const string& fname,
  vector<RefFix>& refFix)
{
  regex re("\\.\\w+$");
  string refName = regex_replace(fname, re, string(".ref"));

  // There might not be a .ref file (not an error).
  ifstream refstr(refName.c_str());
  if (! refstr.is_open())
    return;

  string line, s;
  RefFix rf;
  regex rer("^\\s*\"(.*)\"\\s*$");
  smatch match;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (! str2unsigned(s, rf.lno))
      THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      
    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (s == "insert")
    {
      rf.type = BRIDGE_REF_INSERT;
      if (! regex_search(line, match, rer) || match.size() < 1)
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

      rf.value = match.str(1);
    }
    else if (s == "replace")
    {
      rf.type = BRIDGE_REF_REPLACE;
      if (! regex_search(line, match, rer) || match.size() < 1)
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

      rf.value = match.str(1);
    }
    else if (s == "delete")
    {
      rf.type = BRIDGE_REF_DELETE;
      if (getNextWord(line, s))
      {
        if (! str2unsigned(s, rf.count))
          THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      }
      else
        rf.count = 1;
    }
    else
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    refFix.push_back(rf);
  }
  refstr.close();
}


bool valProgress(
  ifstream& fstr,
  ValSide& side)
{
  if (! getline(fstr, side.line))
    return false;

  side.lno++;
  return true;
}


void validate(
  const string& fileOut,
  const string& fileRef,
  const Format formatOrig,
  const Format formatRef,
  const Options& options,
  ValStats& vstats)
{
  ValState valState;
  valState.bufferOut.read(fileOut, formatRef);
  valState.bufferOut.fix(fileOut, formatRef);

  valState.bufferRef.read(fileRef, formatRef);
  valState.bufferRef.fix(fileRef, formatRef);

  ifstream fostr(fileOut.c_str());
  if (! fostr.is_open())
    THROW("Cannot read from: " + fileOut);

  ifstream frstr(fileRef.c_str());
  if (! frstr.is_open())
    THROW("Cannot read from: " + fileRef);

  vector<RefFix> refFix;
  readRefFix(fileRef.c_str(), refFix);

  ValProfile prof;

  ValExample running;
  running.out.line = "";
  running.ref.line = "";
  running.out.lno = 0;
  running.ref.lno = 0;
  unsigned headerStartTXT = 4; // If comments and no dash line

  while (valProgress(frstr, running.ref))
  {
    if (! valState.bufferRef.next(valState.dataRef))
      THROW("bufferRef ends too soon");
 
    if (! valProgress(fostr, running.out))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      if (valState.bufferOut.next(valState.dataOut))
        THROW("bufferOut ends too late");
      break;
    }

    if (! valState.bufferOut.next(valState.dataOut))
      THROW("bufferOut ends too soon");

    if (refFix.size() > 0 && refFix[0].lno == running.ref.lno)
    {
      if (refFix[0].type == BRIDGE_REF_INSERT)
      {
        if (refFix[0].value != running.out.line)
        {
          // This will mess up everything that follows...
          prof.log(BRIDGE_VAL_ERROR, running);
          if (! valProgress(fostr, running.out))
            THROW("Next line is not there");
          continue;
        }

        // Get the next out line to compare with the ref line.
        if (! valProgress(fostr, running.out))
          THROW("Next line is not there");

        if (! valState.bufferOut.next(valState.dataOut))
          THROW("bufferOut ends too soon");

        // As bufferRef already had the line, need to skip.
        if (! valState.bufferRef.next(valState.dataRef))
          THROW("bufferRef ends too soon");
      }
      else if (refFix[0].type == BRIDGE_REF_REPLACE)
      {
        running.ref.line = refFix[0].value;
      }
      else
      {
        for (unsigned i = 0; i < refFix[0].count; i++)
        {
          if (! valProgress(frstr, running.ref))
            THROW("Skip line is not there");
        }
      }

      refFix.erase(refFix.begin());
    }

    if (valState.dataRef.line != running.ref.line)
      THROW("Different lines, '" + running.ref.line + "', '" +
        valState.dataRef.line + "'\n");

    if (valState.dataOut.line != running.out.line)
      THROW("Different lines, '" + running.out.line + "', '" +
        valState.dataOut.line + "'\n");

    if (formatRef == BRIDGE_FORMAT_TXT &&
        running.out.line.substr(0, 5) == "-----")
      headerStartTXT = running.out.lno+2;

    if (running.ref.line == running.out.line)
      continue;

    // General: % line numbers (Pavlicek error).
    if (formatRef != BRIDGE_FORMAT_RBX &&
        isRecordComment(running.out.line, running.ref.line))
    {
      prof.log(BRIDGE_VAL_RECORD_NUMBER, running);
      continue;
    }
    else if (formatRef == BRIDGE_FORMAT_LIN_RP)
    {
      if (validateLIN_RP(frstr, fostr, running, valState, prof))
      {
        // Fix is already recorded in stats.
        continue;
      }
      else if (fostr.eof() || frstr.eof())
        break;

      // If not, fall through to general error.
    }
    else if (formatRef == BRIDGE_FORMAT_PBN)
    {
      if (validatePBN(frstr, fostr, running, valState, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_RBN)
    {
      if (validateRBN(frstr, running, valState, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_RBX)
    {
      if (validateRBX(frstr, running, valState, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_TXT)
    {
      if (validateTXT(frstr, fostr, running, valState, headerStartTXT, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_EML)
    {
      // if (validateEML(frstr, running, valState, prof))
      if (validateEML(valState, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_REC)
    {
      if (validateREC(frstr, fostr, running, valState, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }

    prof.log(BRIDGE_VAL_ERROR, running);
  }

  if (valProgress(fostr, running.out))
  {
    if (! valState.bufferOut.next(valState.dataOut))
      THROW("bufferOut ends too soon");
    prof.log(BRIDGE_VAL_REF_SHORT, running);
  }

  if (valState.bufferOut.next(valState.dataOut))
    THROW("bufferOut ends too late");


  if (options.verboseValDetails)
    prof.print(cout);

  vstats.add(formatOrig, formatRef, prof);

  fostr.close();
  frstr.close();
}


bool refContainsOut(const ValState& valState)
{
  if (valState.dataOut.len > valState.dataRef.len)
    return false;

  if (valState.dataOut.line == 
      valState.dataRef.line.substr(0, valState.dataOut.len))
    return true;
  else
    return false;
}

