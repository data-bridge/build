/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <regex>

#include "reflineTXT.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


void parseReplaceTXT(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(refName);
  UNUSED(line);
  UNUSED(rer);
  UNUSED(rep);
  UNUSED(match);
  UNUSED(rf);
  THROW("parseReplaceTXT not yet implemented");
}


void parseInsertTXT(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(refName);
  UNUSED(line);
  UNUSED(rer);
  UNUSED(rep);
  UNUSED(match);
  UNUSED(rf);
  THROW("parseInsertTXT not yet implemented");
}


void parseDeleteTXT(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(refName);
  UNUSED(line);
  UNUSED(rer);
  UNUSED(rep);
  UNUSED(match);
  UNUSED(rf);
  THROW("parseDeleteTXT not yet implemented");
}

