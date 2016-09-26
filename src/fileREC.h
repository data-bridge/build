/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READREC_H
#define BRIDGE_READREC_H

#include <string>
#include <vector>
#include "Group.h"

using namespace std;


void setRECtables();

bool readREC(
  Group& group,
  const string& fname);

bool writeREC(
  Group& group,
  const string& fname);


#endif
