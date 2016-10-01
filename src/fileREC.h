/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READREC_H
#define BRIDGE_READREC_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setRECtables();

bool readREC(
  Group& group,
  const string& fname);

bool readRECChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeREC(
  Group& group,
  const string& fname);


#endif
