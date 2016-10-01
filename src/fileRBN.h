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

class Group;


void setRBNTables();

bool readRBN(
  Group& group,
  const string& fname);

bool readRBNChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeRBN(
  Group& group,
  const string& fname);


#endif
