/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READRBN_H
#define BRIDGE_READRBN_H

#include <string>

#include "../bconst.h"

class Group;
class Buffer;
class Chunk;
class Segment;
class Board;
struct WriteInfo;

using namespace std;


void setRBNTables();

void readRBNChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writeRBNSegmentLevel(
  string& st,
  const Segment& segment,
  const Format format);

void writeRBNBoardLevel(
  string& st,
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
