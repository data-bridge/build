/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALPBN_H
#define BRIDGE_VALPBN_H

#include <string>

#include "Buffer.h"
#include "validate.h"
#include "ValProfile.h"

using namespace std;


bool validatePBN(
  ValState& valState,
  ValProfile& prof);

#endif
