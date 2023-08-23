/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_RULESTATS_H
#define BRIDGE_RULESTATS_H

#include <string>
#include <vector>

#include "Rules.h"

using namespace std;

struct RowData;


class RuleStats
{
  private:

    vector<Rules> rulesVector;


  public:

    RuleStats();

    void reset();

    void init(
      const unsigned dim1,
      const unsigned dim2,
      const unsigned dim3,
      const vector<RowData>& rowData);

    void addPosition(
      const unsigned d1,
      const unsigned d2,
      const unsigned d3,
      const unsigned ruleNo,
      const bool flag);

    void addHand(
      const unsigned d1,
      const unsigned d2,
      const unsigned d3,
      const unsigned ruleNo,
      const float passProb);

    void operator += (const RuleStats& rs2);

    string str() const;
};

#endif

