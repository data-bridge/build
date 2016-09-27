/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READRBN_H
#define BRIDGE_READRBN_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setRBNtables();

bool readRBN(
  Group& group,
  const string& fname);

bool writeRBN(
  Group& group,
  const string& fname);


#endif
