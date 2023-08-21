/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This is a somewhat general histogram-type class.
*/

#ifndef BRIDGE_STATS1D_H
#define BRIDGE_STATS1D_H

#include <string>
#include <vector>

#include "StatsInfo.h"

using namespace std;


class Stats1D
{
  private:

    StatsInfo info;

    vector<Count1D> counts;


    bool findLimits(
      unsigned& indexLow,
      unsigned& indexHigh) const;

    string strHeader() const;


  public:

    Stats1D();

    void reset();

    void set(const StatsInfo& infoIn);

    void operator += (const Stats1D& s2);

    void add(
      const unsigned param,
      const bool flag);

    // Kludge to validate pass tables.
    string validateProbs(const vector<float>& rowProbs) const;

    string str() const;
};

#endif

