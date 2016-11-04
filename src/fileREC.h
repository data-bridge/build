/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READREC_H
#define BRIDGE_READREC_H

#include <string>
#include <vector>

using namespace std;

class Group;


bool readRECChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
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
