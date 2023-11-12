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
#include <map>

using namespace std;

class Valuation;

enum Openings: unsigned;


class Opening
{
  private:

    typedef Openings (Opening::*ClassifyPtr)() const;

    static map<string, vector<ClassifyPtr>> classifyMap;

    unsigned spades;
    unsigned hearts;
    unsigned diamonds;
    unsigned clubs;

    unsigned longest1;
    unsigned longest2;
    unsigned longest4;

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

    Openings classifyTwoNTStrong() const;
    Openings classifyTwoNTWeak() const;
    Openings classifyTwoNTIntermed() const;

    Openings classifyTwoNT() const;
      
    Openings classifyThreeClubsStrong() const;
    Openings classifyThreeClubsWeak() const;
    Openings classifyThreeClubsIntermed() const;

    Openings classifyThreeClubs() const;

    Openings classifyThreeDiamondsStrong() const;
    Openings classifyThreeDiamondsWeak() const;
    Openings classifyThreeDiamondsIntermed() const;

    Openings classifyThreeDiamonds() const;

    Openings classifyThreeHeartsStrong() const;
    Openings classifyThreeHeartsWeak() const;
    Openings classifyThreeHeartsIntermed() const;


  public:

    static void init();

    Openings classify(
      const string& call,
      const Valuation& valuation,
      const vector<unsigned>& params);
};

#endif

