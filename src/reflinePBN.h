/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_REFLINEPBN_H
#define BRIDGE_REFLINEPBN_H

#include <string>
#include <regex>

#include "referr.h"

using namespace std;


void parseReplacePBN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseInsertPBN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseDeletePBN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

#endif
