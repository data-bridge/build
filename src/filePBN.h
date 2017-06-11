/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READPBN_H
#define BRIDGE_READPBN_H

#include <string>

class Segment;
class Board;
class Buffer;
class Chunk;

using namespace std;

class Group;


void setPBNTables();

void readPBNChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writePBNBoardLevel(
  string& st,
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
