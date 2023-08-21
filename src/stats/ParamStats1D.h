/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This uses Stats1D to make a series of histograms.
*/

#ifndef BRIDGE_PARAMSTATS1D_H
#define BRIDGE_PARAMSTATS1D_H

#include <string>
#include <vector>

#include "Stats1D.h"

using namespace std;


class ParamStats1D
{
  private:

    vector<vector<vector<Stats1D>>> paramStats1D;

    vector<string> dimNames;

    bool emptyFlag;


  public:

    ParamStats1D();

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

    void operator += (const ParamStats1D& ps2);

    // This is a kludge to judge the quality of pass tables.
    string validateProbs(
      const unsigned d1,
      const unsigned d2,
      const unsigned d3,
      const vector<float>& rowProbs) const;

    string str() const;
};

#endif

