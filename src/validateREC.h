/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALREC_H
#define BRIDGE_VALREC_H

#include <string>

#include "ValProfile.h"


using namespace std;


bool validateREC(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValProfile& prof);
  // ValFileStats& stats);

#endif
