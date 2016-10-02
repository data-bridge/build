/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READPBN_H
#define BRIDGE_READPBN_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setPBNTables();

bool readPBNChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool readPBN(
  Group& group,
  const string& fname);

bool writePBN(
  Group& group,
  const string& fname);


#endif
