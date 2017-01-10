/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALPBN_H
#define BRIDGE_VALPBN_H

#include "ValProfile.h"

using namespace std;


bool validatePBN(
  ValState& valState,
  ValProfile& prof);

bool validatePBNChunk(
  const vector<string>& chunkRef,
  const vector<string>& chunkOut,
  ValState& valState,
  ValProfile& prof);

#endif
