/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <regex>

#include "reflinePBN.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


void parseReplacePBN(
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
  THROW("parseReplacePBN not yet implemented");
}


void parseInsertPBN(
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
  THROW("parseInsertPBN not yet implemented");
}


void parseDeletePBN(
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
  THROW("parseDeletePBN not yet implemented");
}

