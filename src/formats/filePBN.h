/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READPBN_H
#define BRIDGE_READPBN_H

#include <string>

class Segment;
class Board;
class Buffer;
class Chunk;
struct WriteInfo;

enum Format: unsigned;

using namespace std;


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
