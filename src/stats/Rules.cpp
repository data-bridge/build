/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <cassert>

#include "Rules.h"
#include "RowData.h"


Rules::Rules()
{
  Rules::reset();
}


void Rules::reset()
{
  ruleVector.clear();
}


void Rules::init(
  const string& distName,
  const string& posName,
  const string& vulName,
  const vector<RowData>& rowData)
{
  ruleVector.resize(rowData.size());

  for (unsigned i = 0; i < rowData.size(); i++)
    ruleVector[i].init(distName, posName, vulName, rowData[i]);
}


void Rules::addPosition(
  const unsigned ruleNo,
  const bool flag)
{
  assert(ruleNo < ruleVector.size());

  ruleVector[ruleNo].addPosition(flag);
}


void Rules::addHand(
  const unsigned ruleNo,
  const bool flag,
  const float passProb)
{
  assert(ruleNo < ruleVector.size());

  ruleVector[ruleNo].addHand(flag, passProb);
}


void Rules::operator += (const Rules& r2)
{
  assert(ruleVector.size() == r2.ruleVector.size());

  for (unsigned i = 0; i < ruleVector.size(); i++)
    ruleVector[i] += r2.ruleVector[i];
}


bool Rules::empty() const
{
  for (unsigned i = 0; i < ruleVector.size(); i++)
  {
    if (! ruleVector[i].empty())
      return false;
  }
  return true;
}


string Rules::str() const
{
  if (ruleVector.size() == 0)
    return "";

  string s = ruleVector[0].strHeader();

  for (unsigned i = 0; i < ruleVector.size(); i++)
    s += ruleVector[i].str();

  return s;
}

