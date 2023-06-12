/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READREC_H
#define BRIDGE_READREC_H

#include <string>

#include "../bconst.h"

class Group;
class Segment;
class Board;
class Buffer;
class Chunk;
struct WriteInfo;

using namespace std;


void readRECChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writeRECBoardLevel(
  string& st,
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
