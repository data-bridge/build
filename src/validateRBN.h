/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALRBN_H
#define BRIDGE_VALRBN_H

#include <string>

#include "Buffer.h"

using namespace std;


bool validateRBN(
  ifstream& frstr,
  ValExample& running,
  Buffer& bufferRef,
  Buffer& bufferOut,
  LineData& bref,
  LineData& bout,
  ValProfile& prof);

bool validateRBX(
  ifstream& frstr,
  ValExample& running,
  Buffer& bufferRef,
  Buffer& bufferOut,
  LineData& bref,
  LineData& bout,
  ValProfile& prof);

#endif
