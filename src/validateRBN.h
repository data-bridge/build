/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALRBN_H
#define BRIDGE_VALRBN_H

#include <string>

using namespace std;


bool validateRBN(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats);

bool validateRBX(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats);

#endif
