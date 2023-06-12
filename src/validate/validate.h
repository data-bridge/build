/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALIDATE_H
#define BRIDGE_VALIDATE_H

#include <string>

#include "../bconst.h"

using namespace std;

class ValStats;


void setValidateTables();

void validate(
  const string& fileOut,
  const string& fileRef,
  const Format fOrig,
  const Format fRef,
  const Options& options,
  ValStats& vstats);

void validate(
  const string& strOut,
  const string& fileOut,
  const string& fileRef,
  const Format fOrig,
  const Format fRef,
  const Options& options,
  ValStats& vstats);

bool refContainsOut(
  const LineData& dataOut,
  const LineData& dataRef);

bool refContainsOutValue(
  const LineData& dataOut,
  const LineData& dataRef);

bool firstContainsSecond(
  const LineData& first,
  const LineData& second);

bool firstContainsSecond(
  const string& first,
  const string& second);

#endif
