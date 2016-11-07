/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALEML_H
#define BRIDGE_VALEML_H

#include "Buffer.h"
#include "ValProfile.h"

#include <string>

using namespace std;


bool validateEML(
  ValState& valState,
  ValProfile& prof);

#endif
