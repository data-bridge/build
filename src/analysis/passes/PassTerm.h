/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PASSTERM_H
#define BRIDGE_PASSTERM_H

#include <string>

using namespace std;

class Valuation;

enum PassTermType: unsigned;
enum CompositeParams: unsigned;


class PassTerm
{
  private:

    PassTermType termType; // Lower, upper etc.
    CompositeParams valParam; // HCP, Controls etc.
    unsigned limit1;
    unsigned limit2;

    bool lowerContains(const PassTerm& term2) const;

    bool upperContains(const PassTerm& term2) const;

    bool exactContains(const PassTerm& term2) const;

    bool rangeContains(const PassTerm& term2) const;

    bool outsideContains(const PassTerm& term2) const;

  public:

    PassTerm();

    void reset();

    void setLower(
      const CompositeParams valParamIn,
      const unsigned limit);

    void setUpper(
      const CompositeParams valParamIn,
      const unsigned limit);

    void setExact(
      const CompositeParams valParamIn,
      const unsigned limit);

    void setRange(
      const CompositeParams valParamIn,
      const unsigned limit1In,
      const unsigned limit2In);

    void setOutside(
      const CompositeParams valParamIn,
      const unsigned limit1In,
      const unsigned limit2In);

    // >= 7 is contained in >= 5, but >= 4 or <= 8 is not.
    bool contains(const PassTerm& term2) const;

    // Checks for same valuation parameter.
    bool uses(const PassTerm& terms2) const;

    bool match(const Valuation& valuation) const;

    string strCompact() const;

    string str() const;
};

#endif

