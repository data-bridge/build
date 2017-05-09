/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <regex>

#include "reflineRBN.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


void parseReplaceRBN(
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
  THROW("parseReplaceRBN not yet implemented");
}


void parseInsertRBN(
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
  THROW("parseInsertRBN not yet implemented");
}


void parseDeleteRBN(
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
  THROW("parseDeleteRBN not yet implemented");
}

