/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALTXT_H
#define BRIDGE_VALTXT_H

#include "Buffer.h"
#include "ValProfile.h"

#include <string>

using namespace std;


bool validateTXT(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValState& valState,
  const unsigned& emptyState,
  ValProfile& prof);

#endif
