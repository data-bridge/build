/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALLIN_H
#define BRIDGE_VALLIN_H

#include "ValProfile.h"

using namespace std;


bool validateLIN(
  ValState& valState,
  ValProfile& prof);

bool validateLIN_RP(
  ValState& valState,
  ValProfile& prof);

bool validateLINTrailingNoise(ValState& valState);

#endif
