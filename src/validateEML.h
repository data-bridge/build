/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALEML_H
#define BRIDGE_VALEML_H

#include <string>

using namespace std;


bool validateEML(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats);

#endif
