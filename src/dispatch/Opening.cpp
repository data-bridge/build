/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <vector>
#include <map>

#include "Opening.h"
#include "Openings.h"

#include "../analysis/Valuation.h"


enum OPENING_STRENGTH
{
  STRENGTH_WEAK = 0,
  STRENGTH_INTERMED = 1,
  STRENGTH_STRONG = 2,
  STRENGTH_SIZE = 3
};

map<string, vector<Opening::ClassifyPtr>> Opening::classifyMap;


void Opening::init()
{
  classifyMap =
  {
    { "2H", 
      { &Opening::classifyTwoHeartsWeak, 
        &Opening::classifyTwoHeartsIntermed,
        &Opening::classifyTwoHeartsStrong }
    },

    { "2S", 
      { &Opening::classifyTwoSpadesWeak, 
        &Opening::classifyTwoSpadesIntermed,
        &Opening::classifyTwoSpadesStrong }
    },

    { "2NT", 
      { &Opening::classifyTwoNTWeak, 
        &Opening::classifyTwoNTIntermed,
        &Opening::classifyTwoNTStrong }
    },

    { "3C", 
      { &Opening::classifyThreeClubsWeak, 
        &Opening::classifyThreeClubsIntermed,
        &Opening::classifyThreeClubsStrong }
    },

    { "3D", 
      { &Opening::classifyThreeDiamondsWeak, 
        &Opening::classifyThreeDiamondsIntermed,
        &Opening::classifyThreeDiamondsStrong }
    },

    { "3H", 
      { &Opening::classifyThreeHeartsWeak, 
        &Opening::classifyThreeHeartsIntermed,
        &Opening::classifyThreeHeartsStrong }
    }

  };
}


void Opening::set(
  const Valuation& valuation,
  const vector<unsigned>& params)
{
  clubs = valuation.getSuitParam(BRIDGE_CLUBS, VS_LENGTH);
  diamonds = valuation.getSuitParam(BRIDGE_DIAMONDS, VS_LENGTH);
  hearts = valuation.getSuitParam(BRIDGE_HEARTS, VS_LENGTH);
  spades = valuation.getSuitParam(BRIDGE_SPADES, VS_LENGTH);

  longest1 = valuation.getDistParam(VD_L1);
  longest2 = valuation.getDistParam(VD_L2);
  longest4 = valuation.getDistParam(VD_L4);

  hcp = params[PASS_HCP];
}


bool Opening::threeSuiter() const
{
  unsigned numSuits = 0;
  if (spades >= 4) numSuits++;
  if (hearts >= 4) numSuits++;
  if (diamonds >= 4) numSuits++;
  if (clubs >= 4) numSuits++;

  return (numSuits == 3);
}


Openings Opening::classifyTwoHeartsStrong() const
{
  if (hearts >= 5)
  {
    // Catches a few two-suiters as well.
    return OPENING_2H_STRONG_HEARTS;
  }
  else if (spades >= 5 && hearts == 4 && hcp == 16)
  {
    // Assume this is not really strong.
    return OPENING_2H_INTERMED_MAJS;
  }
  else if (spades >= 6)
    return OPENING_2H_STRONG_SPADES;
  else if (spades >= 2 && spades <= 5 &&
      hearts >= 2 && hearts <= 4 &&
      diamonds >= 2 && diamonds <= 6 &&
      clubs >= 2 && clubs <= 6)
    return OPENING_2H_STRONG_BAL;

  if (Opening::threeSuiter())
    return OPENING_2H_STRONG_THREE_SUITER;
  else if (diamonds <= 1 && (clubs == 4 || clubs == 5) &&
    hearts <= 4 && spades <= 4)
  {
    // This assumes that it's really an intermediate hand.
    // In practice this happens very rarely anyway.  See below.
    // 4=4=1=4, 3=4=1=5, 4=3=1=5.
    return OPENING_2H_INTERMED_THREE_SUITER_SHORT_D;
  }
  else
    return OPENING_2H_STRONG_MISC;
}


Openings Opening::classifyTwoHeartsWeak() const
{
  if (hearts >= 6)
  {
    // Catches a few two-suiters as well.
    return OPENING_2H_WEAK_HEARTS;
  }
  else if (hearts == 5)
  {
    if (clubs >= 4 || diamonds >= 4)
      return OPENING_2H_WEAK_WITH_MIN;
    else if (spades >= 4)
      return OPENING_2H_WEAK_WITH_SPADES;
    else
      return OPENING_2H_WEAK_5332;
  }

  const unsigned prod = spades * hearts * diamonds * clubs;
  if (prod == 96 || prod == 108)
  {
    // 4432, 4333
    return OPENING_2H_WEAK_BAL;
  }

  if (hearts == 4)
  {
    if (spades >= 4)
      return OPENING_2H_WEAK_WITH_SPADES;
    else if (clubs >= 5 || diamonds >= 5)
      return OPENING_2H_WEAK_45_MIN;
    // Might fall through.
  }
  else if (spades >= 6)
    return OPENING_2H_WEAK_SPADES;
  else if (spades == 5)
  {
    if (clubs >= 4 || diamonds >= 4)
      return OPENING_2H_WEAK_SPADES_MIN;
    else
      return OPENING_2H_WEAK_SPADES_5332;
  }
  else if (hearts <= 3 && clubs >= 4 && diamonds >= 4 &&
      clubs + diamonds >= 9)
    return OPENING_2H_WEAK_MINS;
  else if (clubs >= 7 || diamonds >= 7)
    return OPENING_2H_WEAK_MIN;

  // This covers 4 spades with 5+ of a minor, 1=4=4=4,
  // "4=4=1=4 +/- one card", and bluffs of both Majors,
  //  5 hearts with 4+ of a minor, etc.
  return OPENING_2H_WEAK_MISC;
}


Openings Opening::classifyTwoHeartsIntermed() const
{
  if (hearts >= 6)
    return OPENING_2H_INTERMED_HEARTS;
  else if (spades >= 6)
    return OPENING_2S_INTERMED_HEARTS;
  else if (hearts == 5 && spades < 4)
    return OPENING_2H_INTERMED_HEARTS;
  else if (hearts >= 4 && spades >= 4)
    return OPENING_2H_INTERMED_MAJS;
  else if (diamonds <= 1 && (clubs == 4 || clubs == 5) &&
      hearts <= 4 && spades <= 4)
  {
    // 4=4=1=4, 3=4=1=5, 4=3=1=5.
    return OPENING_2H_INTERMED_THREE_SUITER_SHORT_D;
  }
  else if ((spades == 3 || spades == 4) && hearts <= 1 && 
      (diamonds == 4 || diamonds == 5) &&
      (clubs == 4 || clubs == 5))
    return OPENING_2H_INTERMED_THREE_SUITER_SHORT_H;
  else if (hearts == 4)
  {
    if (clubs >= 5 || diamonds >= 5)
      return OPENING_2H_INTERMED_45_MIN;
    else
      return OPENING_2H_INTERMED_MISC;
  }
  else if (spades == 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_2H_INTERMED_SPADES_MIN;
  else if (clubs >= 5 && diamonds >= 5)
    return OPENING_2H_INTERMED_MINS;
  else
    return OPENING_2H_INTERMED_MISC;
}


Openings Opening::classifyTwoHearts() const
{
  if (hcp >= 16)
    return Opening::classifyTwoHeartsStrong();
  else if (hcp <= 10)
    return Opening::classifyTwoHeartsWeak();
  else
    return Opening::classifyTwoHeartsIntermed();
}


Openings Opening::classifyTwoSpadesStrong() const
{
  if (spades >= 5)
    return OPENING_2S_STRONG_SPADES;
  else if (clubs >= 6)
    return OPENING_2S_STRONG_CLUBS;
  else if (diamonds >= 6)
    return OPENING_2S_STRONG_DIAMONDS;
  else if (hearts >= 6)
    return OPENING_2S_STRONG_HEARTS;

  if (Opening::threeSuiter())
    return OPENING_2S_STRONG_THREE_SUITER;
  else
  {
    // Fall through to intermediate strength.
    return OPENING_SIZE;
  }
}


Openings Opening::classifyTwoSpadesWeak() const
{
  if (spades >= 6)
  {
    // Catches a few two-suiters as well.
    return OPENING_2S_WEAK_SPADES;
  }
  else if (spades == 5)
  {
    if (clubs >= 4 || diamonds >= 4)
      return OPENING_2S_WEAK_WITH_MIN;
    else if (hearts >= 4)
      return OPENING_2S_WEAK_WITH_HEARTS;
    else
      return OPENING_2S_WEAK_5332;
  }
  else if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
    return OPENING_2S_WEAK_MINS;
  else if (clubs >= 6 || diamonds >= 6)
    return OPENING_2S_WEAK_MINOR;
  else if (hearts >= 6)
    return OPENING_2S_WEAK_HEARTS;
  else if (spades == 4 && (clubs >= 5 || diamonds >= 5 || hearts >= 5))
    return OPENING_2S_WEAK_45;
  else if (spades == 4 && (clubs == 4 || diamonds == 4))
    return OPENING_2S_WEAK_44;
  else if (hearts == 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_2S_WEAK_HEARTS_MIN;
  else if (hearts == 5 && (spades >= 4 || diamonds >= 4 || clubs >= 4))
    return OPENING_2S_WEAK_HEARTS_OTHER;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyTwoSpadesIntermed() const
{
  if (spades >= 6)
    return OPENING_2S_INTERMED_SPADES;
  else if (spades == 5)
  {
    if (hcp >= 11)
      return OPENING_2S_INTERMED_SPADES;
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (spades == 4)
  {
    if (clubs >= 5 || diamonds >= 5 || hearts >= 5)
      return OPENING_2S_INTERMED_45;

    if (Opening::threeSuiter())
      return OPENING_2S_INTERMED_THREE_SUITER;
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
    return OPENING_2S_INTERMED_MINS;
  else if (clubs >= 6 || diamonds >= 6)
    return OPENING_2S_INTERMED_MIN;
  else if (hearts == 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_2S_INTERMED_HEARTS_MIN;
  else if (spades <= 1 && hearts >= 3 && diamonds >= 3 && clubs >= 3)
    return OPENING_2S_INTERMED_SHORT_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyTwoSpades() const
{
  if (hcp >= 16)
  {
    const Openings op = Opening::classifyTwoHeartsStrong();
    if (op != OPENING_SIZE)
      return op;
    // Otherwise fall through to intermediate openings.
  }

  if (hcp <= 10)
    return Opening::classifyTwoHeartsWeak();
  else
    return Opening::classifyTwoHeartsIntermed();
}


Openings Opening::classifyTwoNTStrong() const
{
  // Kind-of semi-balanced.
  if (longest1 <= 5 && longest2 <= 4 && longest4 >= 2)
    return OPENING_2NT_STRONG_SBAL;
  else
    return OPENING_2NT_STRONG_OTHER;
}


Openings Opening::classifyTwoNTWeak() const
{
  if ((clubs >= 5 && diamonds >= 4) || (clubs == 4 && diamonds >= 5))
    return OPENING_2NT_WEAK_MINS;
  else if (clubs >= 6 || diamonds >= 6)
    return OPENING_2NT_WEAK_ONE_MIN;
  else if (longest1 >= 5 && longest2 >= 5)
    return OPENING_2NT_WEAK_TWO_SUITER;
  else if (longest1 >= 7)
    return OPENING_2NT_WEAK_ONE_SUITER;
  else
    return OPENING_2NT_WEAK_OTHER;
}


Openings Opening::classifyTwoNTIntermed() const
{
  if (clubs >= 6 || diamonds >= 6)
    return OPENING_2NT_OPEN_ONE_MIN;
  else if (longest1 >= 5 && longest2 >= 4)
    return OPENING_2NT_OPEN_TWO_SUITER;
  else
    return OPENING_2NT_OPEN_OTHER;
}


Openings Opening::classifyTwoNT() const
{
  if (hcp >= 15)
    // TODO When this becomes 16?
    return Opening::classifyTwoNTStrong();

  if (hcp <= 10)
    return Opening::classifyTwoNTWeak();
  else
    return Opening::classifyTwoNTIntermed();
}


Openings Opening::classifyThreeClubsStrong() const
{
  if (clubs >= 5 && longest1 >= 5 && longest2 >= 5)
    return OPENING_3C_STRONG_TWO_SUITER;
  else if (Opening::threeSuiter())
    return OPENING_3C_STRONG_THREE_SUITER;
  else
  {
    // Fall through to intermediate strength.
    return OPENING_SIZE;
  }
}


Openings Opening::classifyThreeClubsWeak() const
{
  if (spades >= 4 && hearts >= 4 && spades + hearts >= 9)
    return OPENING_3C_WEAK_MAJORS;
  else if (spades >= 6 || hearts >= 6)
    return OPENING_3C_WEAK_MAJ;
  else if (clubs <= 4 && diamonds >= 6)
    return OPENING_3C_WEAK_DIAMONDS;

  if (clubs >= 5)
  {
    if (diamonds >= 4)
      return OPENING_3C_WEAK_MINS;
    else if (spades >= 5 || hearts >= 5)
      return OPENING_3C_WEAK_WITH_MAJOR;
    else if (clubs >= 6)
      return OPENING_3C_WEAK_CLUBS;
    else
      return OPENING_3C_WEAK_CLUBS;
  }
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyThreeClubsIntermed() const
{
  if (clubs >= 6)
  {
    // Assume untraditional preempt even if intermediate.
    return OPENING_3C_WEAK_CLUBS;
  }
  else if (diamonds >= 6)
    return OPENING_3C_WEAK_DIAMONDS;
  else if (spades >= 7 || hearts >= 7)
    return OPENING_3C_WEAK_MAJ;

  if (clubs == 5 && longest2 >= 5)
    return OPENING_3C_INTERMED_CLUB_OTHER;
  else if (longest1 + longest2 >= 10)
    return OPENING_3C_INTERMED_TWO_SUITER;
  else if (spades + hearts == 9)
    return OPENING_3C_INTERMED_MAJORS;
  else if (clubs == 5 && hcp <= 13)
    return OPENING_3C_WEAK_CLUBS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyThreeClubs() const
{
  if (hcp >= 16)
  {
    const Openings op = Opening::classifyThreeClubsStrong();
    if (op != OPENING_SIZE)
      return op;
    // Otherwise fall through to intermediate openings.
  }

  if (hcp <= 10)
    return Opening::classifyThreeClubsWeak();
  else
    return Opening::classifyThreeClubsIntermed();
}


Openings Opening::classifyThreeDiamondsStrong() const
{
  if (spades >= 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_3D_STRONG_SPADES_MIN;
  else if (hearts >= 6 && diamonds >= 5)
    return OPENING_3D_STRONG_REDS;
  else
    return OPENING_SIZE;
}


Openings Opening::classifyThreeDiamondsWeak() const
{
  if (diamonds >= 6)
    return OPENING_3D_WEAK_DIAMONDS;
  else if (spades >= 6 || hearts >= 6)
    return OPENING_3D_WEAK_MAJ;
  else if (spades >= 5 && hearts >= 5)
    return OPENING_3D_WEAK_MAJORS;

  if (diamonds == 5)
  {
    if (longest1 == 5 && (longest2 < 5 || clubs == 5))
      return OPENING_3D_WEAK_DIAMONDS;
    else if (longest1 == 5 && (spades == 5 || hearts == 5))
      return OPENING_3D_WEAK_WITH_MAJ;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyThreeDiamondsIntermed() const
{
  if (diamonds >= 6)
  {
    if (longest1 == diamonds && longest2 <= 4)
      return OPENING_3D_WEAK_DIAMONDS;
    else if (spades >= 5 || hearts >= 5)
      return OPENING_3D_WEAK_WITH_MAJ;
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (spades >= 7 || hearts >= 7)
    return OPENING_3D_WEAK_MAJ;
  else if (diamonds >= 5 && hearts >= 5)
    return OPENING_3D_INTERMED_REDS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyThreeDiamonds() const
{
  if (hcp >= 16)
  {
    const Openings op = Opening::classifyThreeDiamondsStrong();
    if (op != OPENING_SIZE)
      return op;
    // Otherwise fall through to intermediate openings.
  }

  if (hcp <= 10)
    return Opening::classifyThreeDiamondsWeak();
  else
    return Opening::classifyThreeDiamondsIntermed();
}


Openings Opening::classifyThreeHeartsStrong() const
{
  if (spades >= 5)
  {
    if (clubs >= 5 || diamonds >= 5)
      return OPENING_3H_STRONG_SPADES_MIN;
    else if (spades >= 7)
      return OPENING_3H_STRONG_SPADES;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyThreeHeartsWeak() const
{
  if (hearts >= 6 && spades <= 4)
    return OPENING_3H_WEAK_HEARTS;
  else if (spades >= 7)
    return OPENING_3H_WEAK_SPADES;
  else if (spades >= 5 && hearts >= 5)
    return OPENING_3H_WEAK_MAJORS;
  else if (clubs >= 8 || diamonds >= 8)
    return OPENING_3H_WEAK_MIN;
  else if (clubs >= 5 && diamonds >= 5)
    return OPENING_3H_WEAK_MINORS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::classifyThreeHeartsIntermed() const
{
  return classifyThreeHeartsWeak();
}


Openings Opening::classify(
  const string& call,
  const Valuation& valuation,
  const vector<unsigned>& params)
{
  Opening::set(valuation, params);

  if (call == "2H")
    return Opening::classifyTwoHearts();
  else if (call == "2S")
    return Opening::classifyTwoSpades();
  else if (call == "2NT")
    return Opening::classifyTwoNT();
  else if (call == "3C")
    return Opening::classifyThreeClubs();
  else if (call == "3D")
    return Opening::classifyThreeDiamonds();
  else if (call == "3H")
  {
    if (hcp >= 16)
    {
      const Openings op = (this->*(classifyMap[call][STRENGTH_STRONG]))();
      if (op != OPENING_SIZE)
        return op;
      // Otherwise fall through to intermediate openings.
    }

    if (hcp <= 10)
      return (this->*(classifyMap[call][STRENGTH_WEAK]))();
    else
      return (this->*(classifyMap[call][STRENGTH_INTERMED]))();
  }
  else
    return OPENING_SIZE;
}


