/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALLIN_H
#define BRIDGE_VALLIN_H

#include <string>

#include "Buffer.h"
#include "validate.h"
#include "ValProfile.h"

using namespace std;


bool validateLIN_RP(
  ValState& valState,
  ValProfile& prof);

#endif
