/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALTXT_H
#define BRIDGE_VALTXT_H

#include "ValProfile.h"

#include <string>

using namespace std;


bool validateTXT(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  const unsigned& emptyState,
  ValProfile& prof);
  // ValFileStats& stats);

#endif
