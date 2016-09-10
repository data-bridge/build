/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READRBN_H
#define BRIDGE_READRBN_H

#include <string>
#include <vector>
#include "Group.h"

using namespace std;


void setRBNTables();

bool readRBN(
  Group& group,
  const string& fname);


#endif
