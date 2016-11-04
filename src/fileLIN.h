/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READLIN_H
#define BRIDGE_READLIN_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setLINTables();

bool readLINChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

void writeLINSegmentLevel(
  string& st,
  Segment& segment,
  const Format format);

void writeLINBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
