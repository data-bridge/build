/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READTXT_H
#define BRIDGE_READTXT_H

#include <string>
#include <vector>

class Buffer;
class Chunk;
class Segment;
class Board;
struct WriteInfo;

enum Format: unsigned;

using namespace std;


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
