/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READRBN_H
#define BRIDGE_READRBN_H

#include <string>
#include <vector>

#include "Buffer.h"

using namespace std;

class Group;


void setRBNTables();

void readRBNChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeRBN(
  Group& group,
  const string& fname);

void writeRBNSegmentLevel(
  string& st,
  Segment& segment,
  const Format format);

void writeRBNBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
