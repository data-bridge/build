/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <cassert>

#include "../analysis/Distributions.h"

#include "../include/bridge.h"

#include "RuleStats.h"


static vector<string> POSITIONS =
{
  "first", "second", "third", "fourth"
};

static vector<string> REL_VULS =
{
  "None", "Both", "We", "They"
};


RuleStats::RuleStats()
{
  RuleStats::reset();
}


void RuleStats::reset()
{
  rulesVector.resize(DIST_SIZE * BRIDGE_PLAYERS * BRIDGE_VULS);
}


void RuleStats::init(
  const unsigned distNo,
  const unsigned posNo,
  const unsigned vulNo,
  const vector<RowData>& rowData)
{
  const unsigned base = BRIDGE_PLAYERS * BRIDGE_VULS * distNo;

  if (posNo == BRIDGE_PLAYERS)
  {
    // All positions, all vuls.
    rulesVector[base].init(DISTRIBUTION_NAMES[distNo], 
      "Any", "Any", rowData);

    for (unsigned i = base+1; i < base + BRIDGE_PLAYERS * BRIDGE_VULS; i++)
      rulesVector[i] = rulesVector[base];
  }
  else if (vulNo == BRIDGE_VULS)
  {
    // Given position, all vuls.
    rulesVector[base].init(DISTRIBUTION_NAMES[distNo], 
      POSITIONS[posNo], "Any", rowData);

    for (unsigned i = base+1; i < base + BRIDGE_VULS; i++)
      rulesVector[i] = rulesVector[base];
  }
  else
  {
    rulesVector[base].init(DISTRIBUTION_NAMES[distNo], 
      POSITIONS[posNo], REL_VULS[vulNo], rowData);
  }
}


void RuleStats::addPosition(
  const unsigned distNo,
  const unsigned posNo,
  const unsigned vulNo,
  const unsigned ruleNo,
  const bool flag)
{
  const unsigned index = BRIDGE_PLAYERS * BRIDGE_VULS * distNo +
    BRIDGE_VULS * posNo + vulNo;

  rulesVector[index].addPosition(ruleNo, flag);
}


void RuleStats::addHand(
  const unsigned distNo,
  const unsigned posNo,
  const unsigned vulNo,
  const unsigned ruleNo,
  const float passProb)
{
  const unsigned index = BRIDGE_PLAYERS * BRIDGE_VULS * distNo +
    BRIDGE_VULS * posNo + vulNo;

  rulesVector[index].addHand(ruleNo, passProb);
}


void RuleStats::operator += (const RuleStats& rs2)
{
  assert(rulesVector.size() == rs2.rulesVector.size());

  for (unsigned i = 0; i < rulesVector.size(); i++)
    rulesVector[i] += rs2.rulesVector[i];
}


string RuleStats::str() const
{
  string s;
  for (unsigned i = 0; i < rulesVector.size(); i++)
    s += rulesVector[i].str();
  return s;
}

