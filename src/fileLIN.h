/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READLIN_H
#define BRIDGE_READLIN_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setLINTables();

bool writeLIN(
  Group& group,
  const string& fname);

bool readLINChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeLIN_RP(
  Group& group,
  const string& fname);

bool writeLIN_VG(
  Group& group,
  const string& fname);

bool writeLIN_TRN(
  Group& group,
  const string& fname);

#endif
