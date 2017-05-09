/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_REFLINEGEN_H
#define BRIDGE_REFLINEGEN_H

#include <string>
#include <regex>

#include "referr.h"

using namespace std;


void parseReplaceGen(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseInsertGen(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseDeleteGen(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

#endif
