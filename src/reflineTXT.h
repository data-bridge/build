/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_REFLINETXT_H
#define BRIDGE_REFLINETXT_H

#include <string>
#include <regex>

#include "referr.h"

using namespace std;


void parseReplaceTXT(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseInsertTXT(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

void parseDeleteTXT(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

#endif
