/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READRBX_H
#define BRIDGE_READRBX_H

#include <string>
#include <vector>
#include "Group.h"

using namespace std;


void setRBXtables();

bool readRBX(
  Group& group,
  const string& fname);

bool writeRBX(
  Group& group,
  const string& fname);


#endif
