/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

// In the simplest case, each row consists of a single term or
// a sequence of terms that must all be fulfilled for the row to match.
// But we also want to be able to build up a row from this:
//
// HCP = 10: 0.80
// modify HCP = 10 when controls <= 3: -0.05
// modify HCP = 10 when controls >= 4: +0.10
//
// So we must be able to check whether a row is consistent with
// the conditions for a modification.
//
// Later on we may also modify:
//
// modify HCP = 10 when spades = 2: -0.10
//
// and this should modify HCP = 10, controls <= 3 as well as
// HCP = 10, controls >= 4.  So consistency means that the modifier
// does not contradict the row.  It does not mean that the two are
// identical.


#ifndef BRIDGE_PASSROW_H
#define BRIDGE_PASSROW_H

#include <list>
#include <string>

#include "PassTerm.h"

using namespace std;

class Valuation;

enum CompositeParams: unsigned;

struct PassMatch
{
  bool matchFlag;
  float prob;
};


class PassRow
{
  private:

    list<PassTerm> terms;

    float prob;

    // If algoFlag is set, the probabability in case of a match is
    // not just returned, but calculated.  This may be a sigmoid with
    // two parameters, for example.
    bool algoFlag;
    float algoParam1;
    float algoParam2;

  public:

    PassRow();

    void reset();

    void addLower(
      const CompositeParams valParam,
      const unsigned limit);

    void addUpper(
      const CompositeParams valParam,
      const unsigned limit);

    void addExact(
      const CompositeParams valParam,
      const unsigned limit);

    void addRange(
      const CompositeParams valParam,
      const unsigned limit1In,
      const unsigned limit2In);

    void addOutside(
      const CompositeParams valParam,
      const unsigned limit1,
      const unsigned limit2);

    // Add the conditions of row2 to this row.
    void add(const PassRow& row2);

    // Set a fixed probability.
    void setProb(const float probIn);

    // Set parameters for a calculation.
    void setAlgo(
      const float algoParam1In,
      const float algoParam2In);

    void addProb(const float probIn);

    // Limit a fixed probability to [0, 1].
    void saturate();

    // Return the size of the terms list.
    size_t count() const;

    float getProb() const;

    bool contains(const PassRow& row2) const;

    bool alreadyUses(const PassRow& row2) const;

    PassMatch match(const Valuation& valuation) const;

    string strCompact() const;

    string str() const;
};

#endif

