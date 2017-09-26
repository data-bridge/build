/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READLIN_H
#define BRIDGE_READLIN_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

using namespace std;

class Group;
class Buffer;
class Chunk;


void setLINTables();

void readLINChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writeLINSegmentLevel(
  string& st,
  const Segment& segment,
  const Format format);

void writeLINBoardLevel(
  string& st,
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
