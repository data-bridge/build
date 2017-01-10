/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READEML_H
#define BRIDGE_READEML_H

#include <string>
#include <vector>

#include "Buffer.h"

using namespace std;

class Group;


void setEMLTables();

void readEMLChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeEML(
  Group& group,
  const string& fname);

void writeEMLBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
