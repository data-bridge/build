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

    unsigned losers;

    unsigned longest1;
    unsigned longest2;
    unsigned longest4;

    unsigned hcp;

    bool solid(
      const Valuation& valuation,
      const unsigned longest,
      const ValSuitParams ValSuitParams,
      const unsigned target) const;

    void set(
      const Valuation& valuation,
      const vector<unsigned>& params);

    bool threeSuiter() const;

    Openings twoCWeak() const;
    Openings twoCInt() const;
    Openings twoCStrong() const;

    Openings twoDWeak() const;
    Openings twoDInt() const;
    Openings twoDStrong() const;

    Openings twoHWeak() const;
    Openings twoHInt() const;
    Openings twoHStrong() const;

    Openings twoSWeak() const;
    Openings twoSInt() const;
    Openings twoSStrong() const;

    Openings twoNTWeak() const;
    Openings twoNTInt() const;
    Openings twoNTStrong() const;

    Openings threeCWeak() const;
    Openings threeCInt() const;
    Openings threeCStrong() const;

    Openings threeDWeak() const;
    Openings threeDInt() const;
    Openings threeDStrong() const;

    Openings threeHWeak() const;
    Openings threeHInt() const;
    Openings threeHStrong() const;

    Openings threeSSolid() const;
    Openings threeSWeak() const;
    Openings threeSInt() const;
    Openings threeSStrong() const;

    Openings threeNTWeak() const;
    Openings threeNTInt() const;
    Openings threeNTStrong() const;

    Openings fourCWeak() const;
    Openings fourCInt() const;
    Openings fourCStrong() const;

    Openings fourDWeak() const;
    Openings fourDInt() const;
    Openings fourDStrong() const;

    Openings fourHWeak() const;
    Openings fourHInt() const;
    Openings fourHStrong() const;

    Openings fourSWeak() const;
    Openings fourSInt() const;
    Openings fourSStrong() const;

    Openings fourNT() const;
    Openings fiveC() const;
    Openings fiveD() const;
    Openings fiveH() const;
    Openings fiveS() const;
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

