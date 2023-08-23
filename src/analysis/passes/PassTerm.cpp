/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <vector>
#include <cassert>

#include "../Valuation.h"
#include "../Composites.h"

#include "PassTerm.h"

enum PassTermType: unsigned
{
  PASSTERM_LOWER = 0,
  PASSTERM_UPPER = 1,
  PASSTERM_EXACT = 2,
  PASSTERM_RANGE = 3,
  PASSTERM_OUTSIDE = 4,
  PASSTERM_SIZE = 5
};

vector<string> PASSTERM_NAMES =
{
  "At least",
  "At most",
  "Exactly",
  "In the range",
  "Outside the range"
};

vector<string> PASSTERM_COMPACT_NAMES =
{
  ">=",
  "<=",
  "==",
  "in",
  "out"
};


typedef bool (PassTerm::*ContainsPtr)(const PassTerm& term2) const;

static vector<ContainsPtr> containsPtrs =
{
  &PassTerm::lowerContains,
  &PassTerm::upperContains,
  &PassTerm::exactContains,
  &PassTerm::rangeContains,
  &PassTerm::outsideContains
};


PassTerm::PassTerm()
{
  PassTerm::reset();
}


void PassTerm::reset()
{
  termType = PASSTERM_SIZE;
  valParam = VC_SIZE;
  limit1 = 0;
  limit2 = 0;
}


void PassTerm::setLower(
  const CompositeParams valParamIn, 
  const unsigned limit)
{
  termType = PASSTERM_LOWER;
  valParam = valParamIn;
  limit1 = limit;
}


void PassTerm::setUpper(
  const CompositeParams valParamIn, 
  const unsigned limit)
{
  termType = PASSTERM_UPPER;
  valParam = valParamIn;
  limit1 = limit;
}


void PassTerm::setExact(
  const CompositeParams valParamIn, 
  const unsigned limit)
{
  termType = PASSTERM_EXACT;
  valParam = valParamIn;
  limit1 = limit;
}


void PassTerm::setRange(
  const CompositeParams valParamIn, 
  const unsigned limit1In,
  const unsigned limit2In)
{
  termType = PASSTERM_RANGE;
  valParam = valParamIn;
  limit1 = limit1In;
  limit2 = limit2In;
}


void PassTerm::setOutside(
  const CompositeParams valParamIn, 
  const unsigned limit1In,
  const unsigned limit2In)
{
  termType = PASSTERM_RANGE;
  valParam = valParamIn;
  limit1 = limit1In;
  limit2 = limit2In;
}


bool PassTerm::lowerContains(const PassTerm& term2) const
{
  if (term2.limit1 < limit1)
    return false;
  else if (term2.termType == PASSTERM_LOWER)
    return true;
  else if (term2.termType == PASSTERM_EXACT)
    return true;
  else if (term2.termType == PASSTERM_RANGE)
    return true;
  else
    return false;
}


bool PassTerm::upperContains(const PassTerm& term2) const
{
  if (term2.limit1 > limit1)
    return false;
  else if (term2.termType == PASSTERM_UPPER)
    return true;
  else if (term2.termType == PASSTERM_EXACT)
    return true;
  else if (term2.termType == PASSTERM_RANGE)
    return true;
  else
    return false;
}


bool PassTerm::exactContains(const PassTerm& term2) const
{
  if (term2.limit1 != limit1)
    return false;
  else if (term2.termType == PASSTERM_EXACT)
    return true;
  else
    return false;
}


bool PassTerm::rangeContains(const PassTerm& term2) const
{
  if (term2.termType != PASSTERM_RANGE)
    return false;
  else if (term2.limit1 < limit1)
    return false;
  else if (term2.limit2 < limit2)
    return true;
  else
    return false;
}


bool PassTerm::outsideContains(const PassTerm& term2) const
{
  if (term2.termType != PASSTERM_OUTSIDE)
    return false;
  else if (term2.limit1 > limit1)
    return false;
  else if (term2.limit2 < limit2)
    return true;
  else
    return false;
}


bool PassTerm::contains(const PassTerm& term2) const
{
  assert(termType < PASSTERM_SIZE);
  return (this->* containsPtrs[termType])(term2);
}


bool PassTerm::uses(const PassTerm& term2) const
{
  return valParam == term2.valParam;
}


bool PassTerm::match(const Valuation& valuation) const
{
  assert(termType < PASSTERM_SIZE);
  const unsigned value = valuation.getCompositeParam(valParam);

  if (termType == PASSTERM_LOWER)
    return (value >= limit1);
  else if (termType == PASSTERM_UPPER)
    return (value <= limit1);
  else if (termType == PASSTERM_EXACT)
    return (value == limit1);
  else if (termType == PASSTERM_RANGE)
    return (value >= limit1 && value <= limit2);
  else if (termType == PASSTERM_OUTSIDE)
    return (value < limit1 || value > limit2);
  else
  {
    assert(false);
    return false;
  }
}


string PassTerm::strCompact() const
{
  assert(termType < PASSTERM_SIZE);

  assert(valParam < CompInfo.size());
  string s = CompInfo[valParam].text + 
    PASSTERM_COMPACT_NAMES[termType] + to_string(limit1);

  if (termType == PASSTERM_RANGE || termType == PASSTERM_OUTSIDE)
    s += "-" + to_string(limit2);

  return s;
}


string PassTerm::str() const
{
  assert(termType < PASSTERM_SIZE);

  assert(valParam < CompInfo.size());
  string s = CompInfo[valParam].text + ": " + 
    PASSTERM_NAMES[termType] + " " +
    to_string(limit1);

  if (termType == PASSTERM_RANGE || termType == PASSTERM_OUTSIDE)
    s += " " + to_string(limit2);

  return s;
}
