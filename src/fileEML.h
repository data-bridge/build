/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READEML_H
#define BRIDGE_READEML_H

#include <string>

using namespace std;

class Buffer;
class Chunk;


void setEMLTables();

void readEMLChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag);

void writeEMLBoardLevel(
  string& st,
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
