/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include "referr.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


static bool lineToLINList(
  const string& line,
  vector<string>& list);


void readRefFix(
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
  regex rep("^\\s*\"(.*)\"\\s+\\{.*\\}\\s*$");
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
      if (regex_search(line, match, rer) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = false;
      }
      else if (regex_search(line, match, rep) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = false;
      }
      else
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");
    }
    else if (s == "replace")
    {
      rf.type = BRIDGE_REF_REPLACE;
      if (regex_search(line, match, rer) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = false;
      }
      else if (regex_search(line, match, rep) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = true;
      }
      else
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");
    }
    else if (s == "delete")
    {
      rf.type = BRIDGE_REF_DELETE;
      if (! getNextWord(line, s))
      {
        rf.count = 1;
        rf.partialFlag = false;
      }
      else if (! str2unsigned(s, rf.count))
      {
        THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      }
      else if (getNextWord(line, s))
        rf.partialFlag = true;
      else
        rf.partialFlag = false;

    }
    else
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    refFix.push_back(rf);
  }
  refstr.close();
}


static bool lineToLINList(
  const string& line,
  vector<string>& list)
{
  // Split on |
  // Fields should have length 2 alternating with content
  UNUSED(line);
  UNUSED(list);
  return true;
}


string strRefFix(const RefFix& refFix)
{
  string st;
  st = STR(refFix.lno) + " ";
  if (refFix.type == BRIDGE_REF_INSERT)
    st += "insert";
  else if (refFix.type == BRIDGE_REF_REPLACE)
    st += "replace";
  else if (refFix.type == BRIDGE_REF_DELETE)
    st += "delete";
  else
    st += "ERROR";

  st += " ";
  if (refFix.count == 1)
    st += "\"" + refFix.value + "\"";
  else
    st += STR(refFix.count);

  return st;
}


RefErrorsType classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  unsigned& numLINTags)
{
  UNUSED(bufferLine);
  vector<string> list;
  numLINTags = 0;

  // refEentry has .type (FixType) and .value (string) as well
  // TODO
  // Start out with local changes that only affect one qx
  
  switch (refEntry.type)
  {
    case BRIDGE_REF_INSERT:
      // Split new line
      // As below
      return ERR_SIZE;

    case BRIDGE_REF_REPLACE:
      // Split old and new line
      // If there's a single stretch of differences, work on this
      // If not, fail for now
      // rs is a special case with known format
      // ERR_LIN_SV_WRONG
      // ERR_LIN_MB_WRONG
      // ERR_LIN_MB_OVERLONG
      // ERR_LIN_MC_CLAIM_WRONG
      lineToLINList(refEntry.value, list);
      return ERR_SIZE;
      

    case BRIDGE_REF_DELETE:
      // Split old line
      // If there's a single stretch, work on this
      // If not, fail for now
      // mc deletion could be unneeded (in fact, it should be).
      // ERR_LIN_MB_OVERLONG
      return ERR_SIZE;

    default:
      return ERR_SIZE;
  }
}

