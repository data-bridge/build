/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_STATSINFO_H
#define BRIDGE_STATSINFO_H

#include <string>

using namespace std;


struct StatsInfo
{
  string name;
  unsigned low;
  unsigned length;
  unsigned factor;

  void reset()
  {
    name = "";
    low = 0;
    length = 0;
    factor = 0;
  };
};

struct Count1D
{
  unsigned count;
  unsigned hits;

  void reset() 
  {
    count = 0;
    hits = 0;
  };

  void add(const bool flag)
  {
    count++;
    if (flag)
      hits++;
  };

  void operator += (const Count1D& ci2)
  {
    count += ci2.count;
    hits += ci2.hits;
  };
};

#endif

