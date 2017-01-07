/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READTXT_H
#define BRIDGE_READTXT_H

#include <string>
#include <vector>

#include "Buffer.h"

using namespace std;

class Group;


void setTXTTables();

void readTXTChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeTXT(
  Group& group,
  const string& fname);

void writeTXTSegmentLevel(
  string& st,
  Segment& segment,
  const Format format);

void writeTXTBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
