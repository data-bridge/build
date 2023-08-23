/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <cassert>

#include "Rule.h"
#include "RowData.h"


Rule::Rule()
{
  Rule::reset();
}


void Rule::reset()
{
  distName = "";
  posName = "";
  vulName = "";
  ruleText = "";
  rulePosProb = 0.f;

  hitsPos = 0;
  passesPos = 0;

  hitsHand = 0;
  passesHand = 0;
  cumProbHand = 0.f;
}


void Rule::init(
  const string& distNameIn,
  const string& posNameIn,
  const string& vulNameIn,
  const RowData& rowData)
{
  distName = distNameIn;
  posName = posNameIn;
  vulName = vulNameIn;
  ruleText = rowData.text;
  rulePosProb = rowData.prob;
}


void Rule::addPosition(const bool flag)
{
  hitsPos++;
  if (flag)
    passesPos++;
}


void Rule::addHand(
  const bool flag,
  const float passProb)
{
  hitsHand++;
  if (flag)
    passesHand++;
  cumProbHand += passProb;
}


void Rule::operator += (const Rule& r2)
{
  assert(distName == r2.distName);
  assert(posName == r2.posName);
  assert(vulName == r2.vulName);
  assert(ruleText == r2.ruleText);

  hitsPos += r2.hitsPos;
  passesPos += r2.passesPos;

  hitsHand += r2.hitsHand;
  passesHand += r2.passesHand;
  cumProbHand = r2. cumProbHand;
}


bool Rule::empty() const
{
  return (hitsPos == 0 && hitsHand == 0);
}


string Rule::strHeader() const
{
  stringstream ss;
  ss << 
    setw(40) << left << "Rules" <<
    setw(8) << right << "Phits" <<
    setw(8) << "Ppasses" <<
    setw(8) << "Pprob" <<
    setw(8) << "Rprob" <<
    setw(8) << "Hhits" <<
    setw(8) << "Hpasses" <<
    setw(8) << "Cummass" << "\n";
  return ss.str();
}


string Rule::str() const
{
  if (Rule::empty())
    return "";

  stringstream ss;

  const string s = 
    distName + "." +
    posName + "." +
    vulName + "." +
    ruleText + ".";

  ss <<
    setw(40) << left << s << 
    setw(8) << right << hitsPos <<
    setw(8) << passesPos <<
    setw(8) << setprecision(3) << fixed << 
      (hitsPos == 0 ? 0.f : 
       static_cast<float>(passesPos) / static_cast<float>(hitsPos)) <<
    setw(8) << setprecision(3) << fixed << rulePosProb <<
    setw(8) << hitsHand << 
    setw(8) << passesHand << 
    setw(8) << setprecision(3) << fixed << cumProbHand << "\n";

  return ss.str();
}

