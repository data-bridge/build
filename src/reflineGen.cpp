/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <regex>

#include "reflineGen.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


void parseReplaceGen(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  rf.type = BRIDGE_REF_REPLACE_GEN;
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


void parseInsertGen(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  rf.type = BRIDGE_REF_INSERT_GEN;
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


void parseDeleteGen(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(rer);
  UNUSED(rep);
  UNUSED(match);

  rf.type = BRIDGE_REF_DELETE_GEN;
  string line2 = line, s;
  if (! getNextWord(line2, s))
  {
    rf.count = 1;
    rf.partialFlag = false;
  }
  else if (s.at(0) == '{')
  {
    rf.count = 1;
    rf.partialFlag = true;
  }
  else if (! str2unsigned(s, rf.count))
  {
    THROW("Ref file " + refName + ": Bad number in '" + line + "'");
  }
  else if (getNextWord(line2, s))
    rf.partialFlag = true;
  else
    rf.partialFlag = false;
}

