/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

// "Blind" opening approximations based on distribution and points,
// not on convention cards.  This goes from the opening (2H etc.) to
// the type of opening (weak two in hearts), not from the hand to
// the opening.
//
// Dataless class, so it is a class just for encapsulation.

#ifndef BRIDGE_OPENING_H
#define BRIDGE_OPENING_H

#include <string>
#include <vector>

using namespace std;

class Valuation;

enum Openings: unsigned;


class Opening
{
  private:

    unsigned spades;
    unsigned hearts;
    unsigned diamonds;
    unsigned clubs;
    unsigned hcp;

    void set(
      const Valuation& valuation,
      const vector<unsigned>& params);

    bool threeSuiter() const;

    Openings classifyTwoHeartsStrong() const;
    Openings classifyTwoHeartsWeak() const;
    Openings classifyTwoHeartsIntermed() const;

    Openings classifyTwoHearts() const;
      
    Openings classifyTwoSpadesStrong() const;
    Openings classifyTwoSpadesWeak() const;
    Openings classifyTwoSpadesIntermed() const;

    Openings classifyTwoSpades() const;

    Openings classifyTwoNT(const Valuation& valuation) const;
      
    Openings classifyThreeClubsStrong(const Valuation& valuation) const;
    Openings classifyThreeClubsWeak() const;
    Openings classifyThreeClubsIntermed(const Valuation& valuation) const;

    Openings classifyThreeClubs(const Valuation& valuation) const;


  public:

    Openings classify(
      const string& call,
      const Valuation& valuation,
      const vector<unsigned>& params);
};

#endif

