/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READPBN_H
#define BRIDGE_READPBN_H

#include <string>
#include <vector>

#include "Buffer.h"

using namespace std;

class Group;


void setPBNTables();

void readPBNChunk(
  Buffer& buffer,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

void writePBNSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const Format format);

void writePBNBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
