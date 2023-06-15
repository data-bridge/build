/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LINDATA_H
#define BRIDGE_LINDATA_H

#include <string>

using namespace std;


struct LINInstData
{
  string contract;
  string players[4];
  string mp;
};

struct LINData
{
  LINInstData data[2];
  string no;
};

#endif
