/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READRBX_H
#define BRIDGE_READRBX_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setRBXTables();

bool readRBX(
  Group& group,
  const string& fname);

bool readRBXChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeRBX(
  Group& group,
  const string& fname);


#endif
