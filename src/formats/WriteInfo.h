/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_WRITEINFO_H
#define BRIDGE_WRITEINFO_H

#include <string>

using namespace std;


struct WriteInfo
{
  unsigned bno;
  unsigned instNo;
  unsigned ino;
  unsigned numBoards;
  unsigned numInst;
  unsigned numInstActive;
  bool first;
  bool last;

  string namesOld[2];

  int score1;
  int score2;
};

#endif

