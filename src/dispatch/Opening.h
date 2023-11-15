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
      
    Openings twoSpadesWeak() const;
    Openings twoSpadesInt() const;
    Openings twoSpadesStrong() const;

    Openings twoNTWeak() const;
    Openings twoNTInt() const;
    Openings twoNTStrong() const;

    Openings threeClubsWeak() const;
    Openings threeClubsInt() const;
    Openings threeClubsStrong() const;

    Openings threeDiamondsWeak() const;
    Openings threeDiamondsInt() const;
    Openings threeDiamondsStrong() const;

    Openings threeHeartsWeak() const;
    Openings threeHeartsInt() const;
    Openings threeHeartsStrong() const;

    Openings threeSpadesSolid() const;
    Openings threeSpadesWeak() const;
    Openings threeSpadesInt() const;
    Openings threeSpadesStrong() const;

    Openings threeNTWeak() const;
    Openings threeNTInt() const;
    Openings threeNTStrong() const;

    Openings fourClubsWeak() const;
    Openings fourClubsInt() const;
    Openings fourClubsStrong() const;

    Openings fourDiamondsWeak() const;
    Openings fourDiamondsInt() const;
    Openings fourDiamondsStrong() const;

    Openings fourHeartsWeak() const;
    Openings fourHeartsInt() const;
    Openings fourHeartsStrong() const;

    Openings fourSpadesWeak() const;
    Openings fourSpadesInt() const;
    Openings fourSpadesStrong() const;

    Openings fourNT() const;
    Openings fiveClubs() const;
    Openings fiveDiamonds() const;
    Openings fiveHearts() const;
    Openings fiveSpades() const;
    Openings fiveNT() const;
    Openings sixC() const;
    Openings sixD() const;
    Openings sixH() const;
    Openings sixS() const;
    Openings sixNT() const;
    Openings sevenC() const;
    Openings sevenD() const;
    Openings sevenH() const;
    Openings sevenS() const;
    Openings sevenNT() const;


  public:

    static void init();

    Openings classify(
      const string& call,
      const Valuation& valuation,
      const vector<unsigned>& params);
};

#endif

