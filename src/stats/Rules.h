/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_RULES_H
#define BRIDGE_RULES_H

#include <string>
#include <vector>

#include "Rule.h"

using namespace std;


class Rules
{
  private:

    vector<Rule> ruleVector;


  public:

    Rules();

    void reset();

    void init(
      const string& distName,
      const string& posName,
      const string& vulName,
      const vector<RowData>& rowData);

    void addPosition(
      const unsigned ruleNo,
      const bool flag);

    void addHand(
      const unsigned ruleNo,
      const float passProb);

    void operator += (const Rules& r2);

    string str() const;
};

#endif

