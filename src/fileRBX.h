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

bool readRBXChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeRBX(
  Group& group,
  const string& fname);

void writeRBXSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const formatType f);

void writeRBXBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const formatType f);

#endif
