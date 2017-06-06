/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READREC_H
#define BRIDGE_READREC_H

#include <string>

class Group;
class Buffer;
class Chunk;
struct WriteInfo;

using namespace std;



void readRECChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

bool writeREC(
  Group& group,
  const string& fname);

void writeRECBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
