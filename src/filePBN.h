/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READPBN_H
#define BRIDGE_READPBN_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setPBNTables();

bool readPBNChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writePBN(
  Group& group,
  const string& fname);

void writePBNSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const formatType f);

void writePBNBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  const writeInfoType& writeInfo,
  const formatType f);

#endif
