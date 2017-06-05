/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READLIN_H
#define BRIDGE_READLIN_H

#include <string>
#include <vector>

#include "Buffer.h"
#include "Chunk.h"

using namespace std;

class Group;


void setLINTables();

void readLINChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writeLINSegmentLevel(
  string& st,
  Segment& segment,
  const Format format);

void writeLINBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
