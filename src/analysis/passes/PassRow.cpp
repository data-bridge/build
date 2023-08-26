/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <cassert>

#include "../Valuation.h"

#include "../../stats/RowData.h"

#include "Sigmoid.h"
#include "RowProbInfo.h"
#include "PassRow.h"

static Sigmoid sigmoid;


PassRow::PassRow()
{
  PassRow::reset();
}


void PassRow::reset()
{
  terms.clear();
  prob = -1.;
  algoFlag = false;
  sigmoidData.reset();
}


void PassRow::addLower(
  const CompositeParams valParam, 
  const unsigned limit)
{
  terms.emplace_back(PassTerm());
  terms.back().setLower(valParam, limit);
}


void PassRow::addUpper(
  const CompositeParams valParam, 
  const unsigned limit)
{
  terms.emplace_back(PassTerm());
  terms.back().setUpper(valParam, limit);
}


void PassRow::addExact(
  const CompositeParams valParam, 
  const unsigned limit)
{
  terms.emplace_back(PassTerm());
  terms.back().setExact(valParam, limit);
}


void PassRow::addRange(
  const CompositeParams valParam, 
  const unsigned limit1,
  const unsigned limit2)
{
  terms.emplace_back(PassTerm());
  terms.back().setRange(valParam, limit1, limit2);
}


void PassRow::addOutside(
  const CompositeParams valParam, 
  const unsigned limit1,
  const unsigned limit2)
{
  terms.emplace_back(PassTerm());
  terms.back().setOutside(valParam, limit1, limit2);
}


void PassRow::add(const PassRow& row2)
{
  for (auto& term: row2.terms)
    terms.push_back(term);
}


void PassRow::setProb(const float probIn)
{
  prob = probIn;
  algoFlag = false;
}


void PassRow::setProb(const RowProbInfo& rowProbInfo)
{
  if (rowProbInfo.algoFlag)
  {
    algoFlag = true;
    prob = 0.f;
    sigmoidData = rowProbInfo.sigmoidData;
  }
  else
  {
    algoFlag = false;
    prob = rowProbInfo.prob;
  }
}


void PassRow::addProb(const float probIn)
{
  prob += probIn;
}


void PassRow::saturate()
{
  if (algoFlag)
    return;

  if (prob < 0.)
    prob = 0.;
  else if (prob > 1.)
    prob = 1.;
}


size_t PassRow::count() const
{
  return terms.size();
}


float PassRow::getProb() const
{
  assert(! algoFlag);
  return prob;
}


float PassRow::getProb(const float input) const
{
  if (algoFlag)
    return sigmoid.calc(sigmoidData, input) + prob;
  else
    return prob;
}


void PassRow::getRowData(RowData& rowData) const
{
  rowData.text = "";
  const size_t len = terms.size();
  size_t i = 0;
  for (auto& term: terms)
  {
    rowData.text += term.strCompact();
    if (i+1 < len)
      rowData.text += "&";
    i++;
  }

  rowData.prob = prob;
}


bool PassRow::contains(const PassRow& row2) const
{
  if (row2.terms.size() > terms.size())
    return false;

  // Make a record of which of our own terms have been seen.
  vector<bool> seen(terms.size(), false);

  for (auto& term2: row2.terms)
  {
    bool foundFlag = false;
    size_t j = 0;
    for (auto& term: terms)
    {
      if (seen[j])
      {
        j++;
        continue;
      }
      j++;

      if (term.contains(term2))
      {
        seen[j] = true;
        foundFlag = true;
        break;
      }

      if (! foundFlag)
        return false;
    }
  }
  return true;
}


bool PassRow::alreadyUses(const PassRow& row2) const
{
  for (auto& term2: row2.terms)
    for (auto& term: terms)
      if (term.uses(term2))
        return true;

  return false;
}


PassMatch PassRow::match(const Valuation& valuation) const
{
  for (auto& term: terms)
    if (! term.match(valuation))
      return {false, false, 0.};

  if (algoFlag)
  {
    const unsigned cccc = valuation.getCompositeParam(VC_CCCC);
    const float ccccFloat = static_cast<float>(cccc) / 20.f;
    return {true, true, sigmoid.calc(sigmoidData, ccccFloat)};
  }
  else
    return {true, false, prob};
}


string PassRow::strCompact() const
{
  string s;
  const size_t len = terms.size();
  size_t i = 0;
  for (auto& term: terms)
  {
    s += term.str();
    if (i+1 < len)
      s += "&";
    i++;
  }
  return s;
}


string PassRow::str() const
{
  stringstream ss;
  ss << "Probability " << setprecision(3) << prob << "\n";
  for (auto& term: terms)
    ss << term.str();
  return ss.str() + "\n";
}

