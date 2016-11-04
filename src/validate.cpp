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
  bool keepLineOut = false;
  bool keepLineRef = false;
  unsigned headerStartTXT = 4; // If comments and no dash line

  while (keepLineRef || valProgress(frstr, running.ref))
  {
    if (! keepLineOut && ! valProgress(fostr, running.out))
    {
      prof.log(BRIDGE_VAL_OUT_SHORT, running);
      break;
    }

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
          keepLineRef = false;
          keepLineOut = false;
          continue;
        }

        // Get the next out line to compare with the ref line.
        if (! valProgress(fostr, running.out))
          THROW("Next line is not there");
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

    if (formatRef == BRIDGE_FORMAT_TXT &&
        running.out.line.substr(0, 5) == "-----")
      headerStartTXT = running.out.lno+2;

    if (running.ref.line == running.out.line)
    {
      keepLineRef = false;
      keepLineOut = false;
      continue;
    }

    // General: % line numbers (Pavlicek error).
    if (formatRef != BRIDGE_FORMAT_RBX &&
        isRecordComment(running.out.line, running.ref.line))
    {
      prof.log(BRIDGE_VAL_RECORD_NUMBER, running);
      continue;
    }
    else if (formatRef == BRIDGE_FORMAT_LIN_RP)
    {
      if (validateLIN_RP(frstr, fostr, running, prof))
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
      if (validatePBN(frstr, fostr, running, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_RBN)
    {
      if (validateRBN(frstr, running, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_RBX)
    {
      if (validateRBX(frstr, running, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_TXT)
    {
      if (validateTXT(frstr, fostr, running, headerStartTXT, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_EML)
    {
      if (validateEML(frstr, running, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_REC)
    {
      if (validateREC(frstr, fostr, running, prof))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }

    prof.log(BRIDGE_VAL_ERROR, running);

    keepLineOut = false;
    keepLineRef = false;
  }

  if (valProgress(fostr, running.out))
    prof.log(BRIDGE_VAL_REF_SHORT, running);

  if (options.verboseValDetails)
    prof.print(cout);

  vstats.add(formatOrig, formatRef, prof);

  fostr.close();
  frstr.close();
}

