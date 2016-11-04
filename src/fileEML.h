/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_READEML_H
#define BRIDGE_READEML_H

#include <string>
#include <vector>

using namespace std;

class Group;


void setEMLTables();

bool readEMLChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool writeEML(
  Group& group,
  const string& fname);

void writeEMLBoardLevel(
  ofstream& fstr,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format);

#endif
