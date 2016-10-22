/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALPBN_H
#define BRIDGE_VALPBN_H

#include <string>

using namespace std;


bool validatePBN(
  ifstream& frstr,
  ifstream& fostr,
  ValExample& running,
  ValFileStats& stats);

#endif
