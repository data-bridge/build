/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This is a somewhat general 2D histogram-type class.
*/

#ifndef BRIDGE_STATS2D_H
#define BRIDGE_STATS2D_H

#include <string>
#include <vector>

#include "StatsInfo.h"

using namespace std;


class Stats2D
{
  private:

    StatsInfo info1;
    StatsInfo info2;

    vector<vector<Count1D>> counts;
    vector<Count1D> counts1;
    vector<Count1D> counts2;


    bool findLimits(
      const vector<Count1D>& countsIn,
      unsigned& indexLow,
      unsigned& indexHigh) const;

    string strHeader(
      const string& shead,
      const unsigned index2Low,
      const unsigned index2High) const;

    string strTable(
      const unsigned tableIndex,
      const unsigned index1Low,
      const unsigned index1High,
      const unsigned index2Low,
      const unsigned index2High) const;


  public:

    Stats2D();

    void reset();

    void set1(const StatsInfo& info1In);
    void set2(const StatsInfo& info2In);

    void add(
      const unsigned param1,
      const unsigned param2,
      const bool flag);

    void operator += (const Stats2D& s2);

    string str() const;
};

#endif

