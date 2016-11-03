/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READTXT_H
#define BRIDGE_READTXT_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setTXTTables();

bool readTXTChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeTXT(
  Group& group,
  const string& fname);

void writeTXTSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const Format format);

void writeTXTBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const Format format);

#endif
