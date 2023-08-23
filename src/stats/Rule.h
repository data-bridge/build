/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_RULE_H
#define BRIDGE_RULE_H

#include <string>
#include <vector>

using namespace std;

struct RowData;


class Rule
{
  private:

    string distName;
    string posName;
    string vulName;
    string ruleText;
    float rulePosProb;

    unsigned hitsPos;
    unsigned passesPos;

    unsigned hitsHand;
    unsigned passesHand;
    float cumProbHand;


  public:

    Rule();

    void reset();

    void init(
      const string& distName,
      const string& posName,
      const string& vulName,
      const RowData& rowData);

    void addPosition(const bool flag);

    void addHand(
      const bool flag,
      const float passProb);

    void operator += (const Rule& r2);

    bool empty() const;

    string strHeader() const;

    string str() const;
};

#endif

