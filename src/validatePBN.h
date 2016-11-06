/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALPBN_H
#define BRIDGE_VALPBN_H

#include <string>

#include "Buffer.h"

using namespace std;


bool validatePBN(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  Buffer& bufferRef,
  Buffer& bufferOut,
  LineData& bref,
  LineData& bout,
  ValProfile& prof);

#endif
