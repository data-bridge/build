/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READLIN_H
#define BRIDGE_READLIN_H

#include <string>

class Group;
class Segment;
class Board;
class Buffer;
class Chunk;
struct WriteInfo;

enum Format: unsigned;

using namespace std;


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