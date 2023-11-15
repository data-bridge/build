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
enum ValSuitParams: unsigned;


class Opening
{
  private:

    typedef Openings (Opening::*ClassifyPtr)() const;

    static map<string, vector<ClassifyPtr>> classifyMap;

    unsigned spades;
    unsigned hearts;
    unsigned diamonds;
    unsigned clubs;

    bool solidFlag;

    unsigned longest1;
    unsigned longest2;
    unsigned longest4;

    unsigned hcp;

    void set(
      const Valuation& valuation,
      const vector<unsigned>& params);

    bool checkSolid(
      const Valuation& valuation,
      const unsigned longest,
      const ValSuitParams ValSuitParams,
      const unsigned target) const;

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

    Openings classifyThreeDiamondsStrong() const;
    Openings classifyThreeDiamondsWeak() const;
    Openings classifyThreeDiamondsIntermed() const;

    Openings classifyThreeHeartsStrong() const;
    Openings classifyThreeHeartsWeak() const;
    Openings classifyThreeHeartsIntermed() const;

    Openings classifyThreeSpadesStrong() const;
    Openings classifyThreeSpadesWeak() const;
    Openings classifyThreeSpadesIntermed() const;

    Openings classifyThreeNTStrong() const;
    Openings classifyThreeNTWeak() const;
    Openings classifyThreeNTIntermed() const;

    Openings classifyFourClubsStrong() const;
    Openings classifyFourClubsWeak() const;
    Openings classifyFourClubsIntermed() const;

    Openings classifyFourDiamondsStrong() const;
    Openings classifyFourDiamondsWeak() const;
    Openings classifyFourDiamondsIntermed() const;

    Openings classifyFourHeartsStrong() const;
    Openings classifyFourHeartsWeak() const;
    Openings classifyFourHeartsIntermed() const;

    Openings classifyFourSpadesStrong() const;
    Openings classifyFourSpadesWeak() const;
    Openings classifyFourSpadesIntermed() const;

    Openings classifyFourNTStrong() const;
    Openings classifyFourNTWeak() const;
    Openings classifyFourNTIntermed() const;

    Openings classifyFiveClubsStrong() const;
    Openings classifyFiveClubsWeak() const;
    Openings classifyFiveClubsIntermed() const;

    Openings classifyFiveDiamondsStrong() const;
    Openings classifyFiveDiamondsWeak() const;
    Openings classifyFiveDiamondsIntermed() const;


  public:

    static void init();

    Openings classify(
      const string& call,
      const Valuation& valuation,
      const vector<unsigned>& params);
};

#endif

