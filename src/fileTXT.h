/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READTXT_H
#define BRIDGE_READTXT_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setTXTtables();

bool readTXT(
  Group& group,
  const string& fname);

bool readTXTChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeTXT(
  Group& group,
  const string& fname);


#endif
