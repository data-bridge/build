/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALLIN_H
#define BRIDGE_VALLIN_H

#include <string>

using namespace std;


bool validateLIN_RP(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValFileStats& stats);

#endif
