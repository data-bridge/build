/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This uses Stats1D to make a series of histograms.
*/

#ifndef BRIDGE_PARAMSTATS2D_H
#define BRIDGE_PARAMSTATS2D_H

#include <string>
#include <vector>

#include "Stats2D.h"

using namespace std;


class ParamStats2D
{
  private:

    vector<vector<vector<Stats2D>>> paramStats2D;

    vector<string> dimNames;

    bool emptyFlag;


  public:

    ParamStats2D();

    void reset();

    void init(
      const unsigned dim1,
      const unsigned dim2,
      const unsigned dim3,
      const vector<string>& dimNamesIn,
      const vector<StatsInfo>& dimData);

    void add(
      const unsigned d1,
      const unsigned d2,
      const vector<unsigned>& params,
      const bool flag);

    bool empty() const;

    void operator += (const ParamStats2D& ps2);

    string str() const;
};

#endif

