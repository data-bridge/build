/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READTXT_H
#define BRIDGE_READTXT_H

#include <string>
#include <vector>

#include "Buffer.h"
#include "Chunk.h"

using namespace std;

class Group;


void setTXTTables();

void readTXTChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writeTXTSegmentLevel(
  string& st,
  const Segment& segment,
  const Format format);

void writeTXTBoardLevel(
  string& st,
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
