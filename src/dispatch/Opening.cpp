/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <vector>
#include <map>
#include <cassert>

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
    { "1C", { &Opening::oneCWeak, 
        &Opening::oneCInt, &Opening::oneCStrong } },

    { "1D", { &Opening::oneDWeak, 
        &Opening::oneDInt, &Opening::oneDStrong } },

    { "1H", { &Opening::oneHWeak, 
        &Opening::oneHInt, &Opening::oneHStrong } },

    { "1S", { &Opening::oneSWeak, 
        &Opening::oneSInt, &Opening::oneSStrong } },

    { "1NT", { &Opening::oneNTWeak, 
        &Opening::oneNTInt, &Opening::oneNTStrong } },

    { "2C", { &Opening::twoCWeak, 
        &Opening::twoCInt, &Opening::twoCStrong } },

    { "2D", { &Opening::twoDWeak, 
        &Opening::twoDInt, &Opening::twoDStrong } },

    { "2H", { &Opening::twoHWeak, 
        &Opening::twoHInt, &Opening::twoHStrong } },

    { "2S", { &Opening::twoSWeak, 
        &Opening::twoSInt, &Opening::twoSStrong } },

    { "2NT", { &Opening::twoNTWeak, 
        &Opening::twoNTInt, &Opening::twoNTStrong } },

    { "3C", { &Opening::threeCWeak, 
        &Opening::threeCInt, &Opening::threeCStrong }
    },

    { "3D", { &Opening::threeDWeak, 
        &Opening::threeDInt, &Opening::threeDStrong } },

    { "3H", { &Opening::threeHWeak, 
        &Opening::threeHInt, &Opening::threeHStrong }
    },

    { "3S", { &Opening::threeSWeak, 
        &Opening::threeSInt, &Opening::threeSStrong } },

    { "3NT", { &Opening::threeNTWeak, 
        &Opening::threeNTInt, &Opening::threeNTStrong } },

    { "4C", { &Opening::fourCWeak, 
        &Opening::fourCInt, &Opening::fourCStrong } },

    { "4D", { &Opening::fourDWeak, 
        &Opening::fourDInt, &Opening::fourDStrong } },

    { "4H", { &Opening::fourHWeak, 
        &Opening::fourHInt, &Opening::fourHStrong } },

    { "4S", { &Opening::fourSWeak, 
        &Opening::fourSInt, &Opening::fourSStrong } },

    { "4NT", { &Opening::fourNT, &Opening::fourNT, &Opening::fourNT } },

    { "5C", { &Opening::fiveC, &Opening::fiveC, &Opening::fiveC } },

    { "5D", { &Opening::fiveD, &Opening::fiveD, &Opening::fiveD} },

    { "5H", { &Opening::fiveH, &Opening::fiveH, &Opening::fiveH} },

    { "5S", { &Opening::fiveS, &Opening::fiveS, &Opening::fiveS} },

    { "5NT", { &Opening::fiveNT, &Opening::fiveNT, &Opening::fiveNT} },

    { "6C", { &Opening::sixC, &Opening::sixC, &Opening::sixC } },

    { "6D", { &Opening::sixD, &Opening::sixD, &Opening::sixD } },

    { "6H", { &Opening::sixH, &Opening::sixH, &Opening::sixH } },

    { "6S", { &Opening::sixS, &Opening::sixS, &Opening::sixS } },

    { "6NT", { &Opening::sixNT, &Opening::sixNT, &Opening::sixNT } },

    { "7C", { &Opening::sevenC, &Opening::sevenC, &Opening::sevenC } },

    { "7D", { &Opening::sevenD, &Opening::sevenD, &Opening::sevenD } },

    { "7H", { &Opening::sevenH, &Opening::sevenH, &Opening::sevenH } },

    { "7S", { &Opening::sevenS, &Opening::sevenS, &Opening::sevenS } },

    { "7NT", { &Opening::sevenNT, &Opening::sevenNT, &Opening::sevenNT } },

  };
}


bool Opening::solid(
  const Valuation& valuation,
  const unsigned longest,
  const ValSuitParams sparam,
  const unsigned target) const
{
  if (spades == longest && 
      valuation.getSuitParam(BRIDGE_SPADES, sparam) == target)
    return true;
  else if (hearts == longest &&
      valuation.getSuitParam(BRIDGE_HEARTS, sparam) == target)
    return true;
  else if (diamonds == longest &&
      valuation.getSuitParam(BRIDGE_DIAMONDS, sparam) == target)
    return true;
  else if (clubs == longest &&
      valuation.getSuitParam(BRIDGE_CLUBS, sparam) == target)
    return true;
  else
    return false;
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

  // Doubled, so 3 is 1.5 losers.
  losers = valuation.getCompositeParam(VC_LOSERS);

  if (longest1 == 6)
  {
    // AKQxxx is not really solid, but it shows up in a number
    // of 3NT openings.
    solidFlag = solid(valuation, longest1, VS_TOP3, 3);
  }
  else if (longest1 == 7)
  {
    // AKQxxxx.
    solidFlag = solid(valuation, longest1, VS_TOP3, 3);
  }
  else if (longest1 >= 8)
  {
    // AK eighth+.
    solidFlag = solid(valuation, longest1, VS_TOP2, 2);
  }
  else
    solidFlag = false;
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


// ----------------- One club ----------------------

Openings Opening::oneCWeak() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (clubs == longest1 && longest2 < longest1)
    return OPENING_1C_WEAK_CLUBS;

  if (hearts < 5 && spades < 5)
  {
    if (prod == 108 || prod == 96 || prod == 90 || 
        prod == 80 || prod == 72)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
      return OPENING_1C_WEAK_SBAL;
    }
    else if (clubs > diamonds)
      return OPENING_1C_WEAK_LONGEST_MIN;
    else if (clubs >= 4)
      return OPENING_1C_WEAK_SHORTEST_MIN;
  }


  // Many forcing-pass openings.
  return OPENING_1C_WEAK_OTHER;
}


Openings Opening::oneCInt() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (clubs == longest1)
  {
    if (longest2 < longest1)
      return OPENING_1C_INTERMED_CLUBS;
    else if (spades == clubs)
      return OPENING_1C_INTERMED_CLUBS;
  }

  if (hearts >= 5 || spades >= 5)
  {
    if (prod == 90 || prod == 80)
    {
      // 5-3-3-2, 5-4-2-2.
      return OPENING_1C_INTERMED_SBAL_MAJ;
    }
    else if (hearts >= 5)
      return OPENING_1C_INTERMED_HEARTS;
    else
      return OPENING_1C_INTERMED_SPADES;
  }
  else if (prod == 108 || prod == 96 || prod == 90 || 
      prod == 80 || prod == 72)
  {
    // 4-3-3-3, 4-4-3-2, 5m-3-3-2, 5m-4-2-2, 6m-3-2-2.
    return OPENING_1C_INTERMED_SBAL;
  }
  else if (clubs > diamonds)
    return OPENING_1C_INTERMED_LONGEST_MIN;
  else if (clubs >= 4)
    return OPENING_1C_INTERMED_SHORTEST_MIN;
  else if (diamonds >= 5)
    return OPENING_1C_INTERMED_DIAMONDS;
  else if (prod == 64 && clubs == 1)
    return OPENING_1C_INTERMED_4441;

  return OPENING_UNCLASSIFIED;
}


Openings Opening::oneCStrong() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (clubs == longest1 && longest2 < longest1)
    return OPENING_1C_STRONG_CLUBS;

  if (hearts < 5 && spades < 5)
  {
    if (prod == 108 || prod == 96 || prod == 90 || 
        prod == 80 || prod == 72)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
      return OPENING_1C_STRONG_SBAL;
    }
    else if (clubs > diamonds)
      return OPENING_1C_STRONG_LONGEST_MIN;
    else if (clubs >= 4)
      return OPENING_1C_STRONG_SHORTEST_MIN;
  }

  return OPENING_1D_STRONG_OTHER;
}


// ----------------- One diamond -------------------

Openings Opening::oneDWeak() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (diamonds == longest1 && hearts < longest1 && spades < longest1)
    return OPENING_1D_WEAK_DIAMONDS;
  else if (hearts >= 5 && hearts == longest1)
    return OPENING_1D_WEAK_HEARTS;
  else if (spades >= 5 && spades == longest1)
    return OPENING_1D_WEAK_SPADES;

  if (hearts < 5 && spades < 5)
  {
    if (prod == 108 || prod == 96 || prod == 90 || 
        prod == 80 || prod == 72)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
      return OPENING_1D_WEAK_SBAL;
    }
    else if (diamonds >= clubs)
      return OPENING_1D_WEAK_LONGEST_MIN;
    else if (diamonds >= 4)
      return OPENING_1D_WEAK_SHORTEST_MIN;
    else if (hcp >= 9)
      return OPENING_1D_INTERMED_CATCHALL;
  }

  return OPENING_1D_WEAK_OTHER;
}


Openings Opening::oneDInt() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (diamonds == longest1 && hearts < longest1 && spades < longest1)
    return OPENING_1D_INTERMED_DIAMONDS;

  if (hearts >= 5 && hearts > diamonds)
    return OPENING_1D_INTERMED_HEARTS;

  if (hearts < 5 && spades < 5)
  {
    if (prod == 108 || prod == 96 || prod == 90 || 
        prod == 80 || prod == 72)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
      return OPENING_1D_INTERMED_SBAL;
    }
    else if (diamonds >= clubs)
      return OPENING_1D_INTERMED_LONGEST_MIN;
    else if (diamonds >= 4 && clubs > diamonds)
      return OPENING_1D_INTERMED_SHORTEST_MIN;
    else
      return OPENING_1D_INTERMED_CATCHALL;
  }
  else if (diamonds >= 4)
      return OPENING_1D_INTERMED_CANAPE;
  else if (spades >= 5)
      return OPENING_1D_INTERMED_SPADES;

  return OPENING_UNCLASSIFIED;
}


Openings Opening::oneDStrong() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (diamonds == longest1 && hearts < longest1 && spades < longest1)
    return OPENING_1D_STRONG_DIAMONDS;

  if (hearts < 5 && spades < 5)
  {
    if (prod == 108 || prod == 96 || prod == 90 || 
        prod == 80 || prod == 72)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
      return OPENING_1D_STRONG_SBAL;
    }
    else if (diamonds >= clubs)
      return OPENING_1D_STRONG_LONGEST_MIN;
  }

  return OPENING_1D_STRONG_OTHER;
}


// ----------------- One heart ---------------------

Openings Opening::oneHWeak() const
{
  if (hearts >= 5)
  {
    if (hearts == longest1)
      return OPENING_1H_WEAK_FIVE;
    else if (hearts == longest2)
    {
      if (clubs == 6 || diamonds == 6)
        return OPENING_1H_WEAK_FIVE;
      else
        return OPENING_1H_WEAK_CANAPE;
    }
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (hearts == 4)
  {
    if (hearts == longest1)
      return OPENING_1H_WEAK_FOUR;
    else if (hearts == longest2)
      return OPENING_1H_WEAK_CANAPE;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
  {
    if (spades >= 4)
      return OPENING_1H_WEAK_SPADES;
    else if (clubs >= 4)
      return OPENING_1H_WEAK_CLUBS;
    else
      return OPENING_1H_WEAK_OTHER;
  }
}


Openings Opening::oneHInt() const
{
  if (hearts >= 5)
  {
    if (hearts == longest1)
      return OPENING_1H_INTERMED_FIVE;
    else if (hearts == longest2)
    {
      if (clubs == 6 || diamonds == 6)
        return OPENING_1H_INTERMED_FIVE;
      else
        return OPENING_1H_INTERMED_CANAPE;
    }
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (hearts == 4)
  {
    if (hearts == longest1)
      return OPENING_1H_INTERMED_FOUR;
    else if (hearts == longest2)
      return OPENING_1H_INTERMED_CANAPE;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
  {
    if (spades >= 4)
      return OPENING_1H_INTERMED_SPADES;
    else if (clubs >= 4)
      return OPENING_1H_INTERMED_CLUBS;
    else
      return OPENING_1H_INTERMED_OTHER;
  }
}


Openings Opening::oneHStrong() const
{
  if (hearts >= 5)
  {
    if (hearts == longest1)
      return OPENING_1H_INTERMED_FIVE;
    else if (hearts == longest2)
    {
      if (clubs == 6 || diamonds == 6)
        return OPENING_1H_INTERMED_FIVE;
      else
        return OPENING_1H_INTERMED_CANAPE;
    }
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (hearts == 4)
  {
    if (hearts == longest1)
      return OPENING_1H_STRONG_FOUR;
    else if (hearts == longest2)
      return OPENING_1H_STRONG_CANAPE;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
  {
    if (spades >= 4)
      return OPENING_1H_STRONG_SPADES;
    else if (clubs >= 4)
      return OPENING_1H_STRONG_CLUBS;
    else
      return OPENING_1H_STRONG_OTHER;
  }
}


// ----------------- One spade ---------------------

Openings Opening::oneSWeak() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (spades >= 5)
  {
    if (spades == longest1)
      return OPENING_1S_WEAK_FIVE;
    else if (spades == longest2)
    {
      if (clubs == 6 || diamonds == 6)
        return OPENING_1S_WEAK_FIVE;
      else
        return OPENING_1S_WEAK_CANAPE;
    }
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (spades == 4)
  {
    if (spades == longest1)
      return OPENING_1S_WEAK_FOUR;
    else if (spades == longest2)
      return OPENING_1S_WEAK_CANAPE;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
  {
    if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
      return OPENING_1S_WEAK_MINS;
    else if (clubs >= 6 || diamonds >= 6)
      return OPENING_1S_WEAK_MIN;
    else if (hearts >= 5)
      return OPENING_1S_WEAK_HEARTS;
    else if (prod == 108 || prod == 96 || prod == 90)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2.
      return OPENING_1S_WEAK_BAL;
    }
    else
      return OPENING_1S_WEAK_OTHER;
  }
}


Openings Opening::oneSInt() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (spades >= 5)
  {
    if (spades == longest1)
      return OPENING_1S_INTERMED_FIVE;
    else if (spades == longest2)
    {
      if (clubs == 6 || diamonds == 6)
        return OPENING_1S_INTERMED_FIVE;
      else
        return OPENING_1S_INTERMED_CANAPE;
    }
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (spades == 4)
  {
    if (spades == longest1)
      return OPENING_1S_INTERMED_FOUR;
    else if (spades == longest2)
      return OPENING_1S_INTERMED_CANAPE;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
  {
    if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
      return OPENING_1S_INTERMED_MINS;
    else if (clubs >= 6 || diamonds >= 6)
      return OPENING_1S_INTERMED_MIN;
    else if (hearts >= 5)
      return OPENING_1S_INTERMED_HEARTS;
    else if (prod == 108 || prod == 96 || prod == 90)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2.
      return OPENING_1S_INTERMED_BAL;
    }
    else
      return OPENING_1S_INTERMED_OTHER;
  }
}


Openings Opening::oneSStrong() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (spades >= 5)
  {
    if (spades == longest1)
      return OPENING_1S_STRONG_FIVE;
    else if (spades == longest2)
    {
      if (clubs == 6 || diamonds == 6)
        return OPENING_1S_STRONG_FIVE;
      else
        return OPENING_1S_STRONG_CANAPE;
    }
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (spades == 4)
  {
    if (spades == longest1)
      return OPENING_1S_STRONG_FOUR;
    else if (spades == longest2)
      return OPENING_1S_STRONG_CANAPE;
    else
      return OPENING_UNCLASSIFIED;
  }
  else
  {
    if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
      return OPENING_1S_STRONG_MINS;
    else if (clubs >= 6 || diamonds >= 6)
      return OPENING_1S_STRONG_MIN;
    else if (hearts >= 5)
      return OPENING_1S_STRONG_HEARTS;
    else if (prod == 108 || prod == 96 || prod == 90)
    {
      // 4-3-3-3, 4-4-3-2, 5-3-3-2.
      return OPENING_1S_STRONG_BAL;
    }
    else
      return OPENING_1S_STRONG_OTHER;
  }
}


// ----------------- One notrump -------------------

Openings Opening::oneNTWeak() const
{
  if (hcp < 8)
    return OPENING_1NT_WEAK_BLUFF;

  const unsigned prod = spades * hearts * diamonds * clubs;

  if (prod == 108 || prod == 96 || prod == 90 || prod == 80 || prod == 72)
  {
    // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
    return OPENING_1NT_WEAK_SBAL;
  }
  else if (prod == 64)
  {
    // 4-4-4-1.
    return OPENING_1NT_WEAK_4441;
  }
  else if (prod == 60)
  {
    // 5-4-3-1.
    return OPENING_1NT_WEAK_5431;
  }
  else if (prod == 54 && (clubs == 6 || diamonds == 6))
  {
    // 6m-3-3-1.
    return OPENING_1NT_WEAK_6331;
  }
  else if (prod == 56 && (clubs == 7 || diamonds == 7))
  {
    // 7m-2-2-2.
    return OPENING_1NT_WEAK_7222;
  }
  else
    return OPENING_1NT_WEAK_ATYPICAL;
}


Openings Opening::oneNTInt() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (prod == 108 || prod == 96 || prod == 90 || prod == 80 || prod == 72)
  {
    // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
    return OPENING_1NT_INTERMED_SBAL;
  }
  else if (prod == 64)
  {
    // 4-4-4-1.
    return OPENING_1NT_INTERMED_4441;
  }
  else if (prod == 60)
  {
    // 5-4-3-1.
    return OPENING_1NT_INTERMED_5431;
  }
  else if (prod == 54 && (clubs == 6 || diamonds == 6))
  {
    // 6m-3-3-1.
    return OPENING_1NT_INTERMED_6331;
  }
  else if (prod == 56 && (clubs == 7 || diamonds == 7))
  {
    // 7m-2-2-2.
    return OPENING_1NT_INTERMED_7222;
  }
  else
    return OPENING_1NT_INTERMED_ATYPICAL;
}


Openings Opening::oneNTStrong() const
{
  const unsigned prod = spades * hearts * diamonds * clubs;

  if (prod == 108 || prod == 96 || prod == 90 || prod == 80 || prod == 72)
  {
    // 4-3-3-3, 4-4-3-2, 5-3-3-2, 5-4-2-2, 6m-3-2-2.
    return OPENING_1NT_STRONG_SBAL;
  }
  else if (prod == 64)
  {
    // 4-4-4-1.
    return OPENING_1NT_STRONG_4441;
  }
  else if (prod == 60)
  {
    // 5-4-3-1.
    return OPENING_1NT_STRONG_5431;
  }
  else if (prod == 54 && (clubs == 6 || diamonds == 6))
  {
    // 6m-3-3-1.
    return OPENING_1NT_STRONG_6331;
  }
  else if (prod == 56 && (clubs == 7 || diamonds == 7))
  {
    // 7m-2-2-2.
    return OPENING_1NT_STRONG_7222;
  }
  else
    return OPENING_1NT_STRONG_ATYPICAL;
}


// ----------------- Two clubs ---------------------


Openings Opening::twoCWeak() const
{
  if (diamonds >= 6 && hearts <= 4 && spades <= 4)
    return OPENING_2C_WEAK_DIAMONDS;
  else if (hearts >= 4 && spades >= 4)
    return OPENING_2C_WEAK_MAJS;
  else if (hearts >= 6 || spades >= 6)
    return OPENING_2C_WEAK_MAJ;
  if (clubs >= 6 && hearts < 4 && spades < 4)
  {
    if (hcp == 10)
      return OPENING_2C_INTERMED_CLUBS;
    else
      return OPENING_2C_WEAK_CLUBS;
  }
  else if ((clubs >= 5 && (hearts >= 4 || spades >= 4)) ||
      (clubs == 4 && (hearts >= 5 || spades >= 5)))
  {
    if (hcp == 10)
      return OPENING_2C_INTERMED_CLUBS_MAJ;
    else
      return OPENING_2C_WEAK_CLUBS_MAJ;
  }
  else if (clubs >= 5 && diamonds >= 4)
  {
    if (hcp == 10)
      return OPENING_2C_INTERMED_MINS;
    else
      return OPENING_2C_WEAK_MINS;
  }
  else if (diamonds == 5)
    return OPENING_2C_WEAK_DIAMONDS;
  else if (clubs == 5)
    return OPENING_2C_WEAK_CLUBS;
  else if (hearts >= 5 || spades >= 5)
  {
    if (diamonds >= 4 || clubs >= 4)
      return OPENING_2C_WEAK_MAJ_MIN;
    else
      return OPENING_2C_WEAK_MAJ;
  }
  else
  {
    // 4-4-3-2, 4-4-4-1, 4-3-3-3.
    return OPENING_2C_WEAK_BAL;
  }
}


Openings Opening::twoCInt() const
{
  if (losers < 10)
    return OPENING_2C_STRONG_OTHER;
  else if (losers >= 14)
    return Opening::twoCWeak();

  if (diamonds >= 6 && hearts <= 4 && spades <= 4)
  {
    if (hcp <= 11)
      return OPENING_2C_WEAK_DIAMONDS;
    else
      return OPENING_2C_INTERMED_DIAMONDS;
  }

  if (clubs >= 6 && hearts < 4 && spades < 4)
    return OPENING_2C_INTERMED_CLUBS;
  else if (hearts >= 4 && spades >= 4)
    return OPENING_2C_INTERMED_MAJS;
  else if (clubs >= 5 && (hearts >= 4 || spades >= 4))
    return OPENING_2C_INTERMED_CLUBS_MAJ;
  else if (clubs == 4 && (hearts >= 5 || spades >= 5))
    return OPENING_2C_INTERMED_CLUBS_MAJ;
  else if (clubs >= 5)
    return OPENING_2C_INTERMED_CLUBS;
  else if ((hearts >= 5 || spades >= 5) && diamonds >= 5)
  {
    return OPENING_2C_INTERMED_DIAMONDS_MAJ;
  }
  else if (hearts >= 6 || spades >= 6)
  {
    if (losers < 12)
      return OPENING_2C_STRONG_OTHER;
    else
      return OPENING_2C_INTERMED_MAJ;
  }
  else if (longest1 <= 5 && longest4 <= 1)
    return OPENING_2C_INTERMED_THREE_SUITER;
  else
    return OPENING_2C_INTERMED_OTHER;
}


Openings Opening::twoCStrong() const
{
  if (longest1 <= 7 && longest4 >= 2)
    return OPENING_2C_STRONG_SBAL;

  if (hcp >= 18 || losers < 10)
    return OPENING_2C_STRONG_OTHER;

  if ((hearts >= 6 || spades >= 6 || diamonds >= 6) && losers <= 12)
    return OPENING_2C_STRONG_OTHER;

  if (clubs >= 6 && hearts < 4 && spades < 4)
    return OPENING_2C_INTERMED_CLUBS;
  else if (clubs >= 5 && (hearts == 4 || spades == 4))
    return OPENING_2C_INTERMED_CLUBS_MAJ;
  else
  {
    // Bit catch-all, often 4-4-4-1, not necessarily super strong.
    return OPENING_2C_STRONG_OTHER;
  }
}


// ----------------- Two diamonds ------------------


Openings Opening::twoDWeak() const
{
  if (diamonds >= 6 && hearts <= 4 && spades <= 4)
    return OPENING_2D_WEAK_DIAMONDS;
  else if (hearts >= 4 && spades >= 4)
    return OPENING_2D_WEAK_MAJS;
  else if (hearts >= 6 || spades >= 6)
    return OPENING_2D_WEAK_MAJ;
  else if (spades == 5 || hearts == 5)
  {
    if (clubs >= 4 || diamonds >= 4)
      return OPENING_2D_WEAK_MAJ_MIN;
    else
      return OPENING_2D_WEAK_MAJ;
  }
  else if (diamonds == 5)
    return OPENING_2D_WEAK_DIAMONDS;

  const unsigned prod = spades * hearts * diamonds * clubs;
  if (Opening::threeSuiter() || prod == 60)
  {
    // 4-4-4-1, 4-4-5-0, any 5-4-3-1 without both Majors.
    return OPENING_2D_WEAK_THREE_SUITER;
  }
  else if (prod == 96 || prod == 108)
    return OPENING_2D_WEAK_BAL;
  else if (clubs >= 6)
    return OPENING_2D_WEAK_CLUBS;
  else
    return OPENING_2D_WEAK_OTHER;
}


Openings Opening::twoDInt() const
{
  if (losers < 10)
    return OPENING_2D_STRONG_OTHER;
  else if (losers >= 14 && hcp <= 12)
  {
    const Openings op = Opening::twoDWeak();
    if (op != OPENING_UNCLASSIFIED)
      return op;
  }

  if (diamonds >= 6 && hearts <= 4 && spades <= 4)
    return OPENING_2D_INTERMED_DIAMONDS;
  else if (clubs >= 6 && hearts <= 4 && spades <= 4)
    return OPENING_2D_INTERMED_CLUBS;
  else if (hearts >= 6 || spades >= 6)
    return OPENING_2D_INTERMED_MAJ;
  else if (diamonds >= 4 && (hearts == 5 || spades == 5))
    return OPENING_2D_INTERMED_DIAMONDS_MAJ;
  else if (clubs >= 4 && (hearts == 5 || spades == 5))
    return OPENING_2D_INTERMED_CLUBS_MAJ;

  const unsigned prod = spades * hearts * diamonds * clubs;

  if (Opening::threeSuiter() ||
      (prod == 60 && spades <= 4 && hearts <= 4))
  {
    // 4-4-4-1, 4-3-1-5 without 5-card Major, 4-4-5-0 ditto.
    return OPENING_2D_INTERMED_THREE_SUITER;
  }
  else if (hearts >= 4 && spades >= 4 && hearts + spades >= 9)
    return OPENING_2D_INTERMED_MAJS;
  else if (diamonds >= 5 && clubs >= 4)
    return OPENING_2D_INTERMED_MINS;
  else if (longest4 >= 2)
  {
    if (clubs == 5)
      return OPENING_2D_INTERMED_SBAL_CLUBS;
    else if (diamonds == 5)
      return OPENING_2D_INTERMED_SBAL_DIAMONDS;
    else if (spades == 5 || hearts == 5)
      return OPENING_2D_INTERMED_SBAL_MAJ;
  }

  return OPENING_2D_INTERMED_OTHER;
}


Openings Opening::twoDStrong() const
{
  if (longest1 <= 7 && longest4 >= 2)
    return OPENING_2D_STRONG_SBAL;
  else if (hcp >= 18 || losers < 10)
    return OPENING_2D_STRONG_OTHER;
  else if (longest1 >= 6 && losers <= 12)
    return OPENING_2D_STRONG_OTHER;

  const unsigned prod = spades * hearts * diamonds * clubs;

  if (Opening::threeSuiter() ||
      (prod == 60 && spades <= 4 && hearts <= 4))
  {
    // 4-4-4-1, 4-3-1-5 without 5-card Major, 4-4-5-0 ditto.
    return OPENING_2D_INTERMED_THREE_SUITER;
  }
  else if (hearts >= 4 && spades >= 4 && hearts + spades >= 9)
    return OPENING_2D_INTERMED_MAJS;
  else if (diamonds >= 5 && clubs >= 4)
    return OPENING_2D_INTERMED_MINS;
  else
    return Opening::twoDInt();
}


// ----------------- Two hearts --------------------

Openings Opening::twoHWeak() const
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


Openings Opening::twoHInt() const
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
  {
    // 4=1=4=4, 3=1=4=5, 4=1=3=5.
    return OPENING_2H_INTERMED_THREE_SUITER_SHORT_H;
  }
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


Openings Opening::twoHStrong() const
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


// ----------------- Two notrump -------------------

Openings Opening::twoSWeak() const
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


Openings Opening::twoSInt() const
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
      return OPENING_2S_INTERMED_OTHER;
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
    return OPENING_2S_INTERMED_OTHER;
}


Openings Opening::twoSStrong() const
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
    return OPENING_2S_STRONG_OTHER;
  }
}


// ----------------- Two notrump -------------------

Openings Opening::twoNTStrong() const
{
  // Kind-of semi-balanced.
  if (longest1 <= 5 && longest2 <= 4 && longest4 >= 2)
    return OPENING_2NT_STRONG_SBAL;

  const unsigned prod = spades * hearts * diamonds * clubs;
  if (prod == 48)
    return OPENING_2NT_STRONG_6421;
  else if (prod == 54)
    return OPENING_2NT_STRONG_6331;
  else if (prod == 56)
    return OPENING_2NT_STRONG_7222;
  else if (prod == 60)
    return OPENING_2NT_STRONG_5431;
  else if (prod == 64)
    return OPENING_2NT_STRONG_4441;
  else if (prod == 72)
    return OPENING_2NT_STRONG_6322;

  if (clubs >= 5 && diamonds >= 5)
    return OPENING_2NT_STRONG_MINS;
  else if (clubs >= 7 || diamonds >= 7)
    return OPENING_2NT_STRONG_MIN;
  else if (longest2 >= 5)
    return OPENING_2NT_STRONG_TWO_SUITER;
  else
    return OPENING_2NT_STRONG_OTHER;
}


Openings Opening::twoNTWeak() const
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


Openings Opening::twoNTInt() const
{
  if (clubs >= 6 || diamonds >= 6)
    return OPENING_2NT_INTERMED_ONE_MIN;
  else if (longest1 >= 5 && longest2 >= 4)
    return OPENING_2NT_INTERMED_TWO_SUITER;
  else
    return OPENING_2NT_INTERMED_OTHER;
}


// ----------------- Three clubs -------------------

Openings Opening::threeCWeak() const
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
    return OPENING_3C_WEAK_OTHER;
}


Openings Opening::threeCInt() const
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


Openings Opening::threeCStrong() const
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


// ----------------- Three diamonds ----------------

Openings Opening::threeDWeak() const
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


Openings Opening::threeDInt() const
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
    return OPENING_3D_INTERMED_OTHER;
}


Openings Opening::threeDStrong() const
{
  if ((spades >= 5 || hearts >= 5) && (clubs >= 5 || diamonds >= 5))
    return OPENING_3D_STRONG_MAJ_MIN;
  else
  {
    // Fall through to intermediate strength.
    return OPENING_SIZE;
  }
}


// ----------------- Three hearts ------------------

Openings Opening::threeHWeak() const
{
  if (hearts >= 6 && spades <= 4)
    return OPENING_3H_WEAK_HEARTS;
  else if (spades >= 6)
    return OPENING_3H_WEAK_SPADES;
  else if (spades >= 5 && hearts >= 5)
    return OPENING_3H_WEAK_MAJORS;
  else if (hearts == 5 && hearts == longest1 && longest2 <= 5)
    return OPENING_3H_WEAK_HEARTS;
  else if (clubs >= 7 || diamonds >= 7)
    return OPENING_3H_WEAK_MIN;
  else if (clubs >= 5 && diamonds >= 5)
    return OPENING_3H_WEAK_MINORS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::threeHInt() const
{
  if (hearts >= 6 && spades <= 4)
    return OPENING_3H_INTERMED_HEARTS;
  else
    return OPENING_3H_INTERMED_OTHER;
}


Openings Opening::threeHStrong() const
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
  else if (hearts >= 6 && hcp <= 17)
    return OPENING_3H_INTERMED_HEARTS;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Three spades ------------------

Openings Opening::threeSSolid() const
{
  if (solidFlag)
  {
    if (spades >= 7)
      return OPENING_3S_SOLID_SPADES;
    else if (hearts >= 7)
      return OPENING_3S_SOLID_HEARTS;
    else if (diamonds >= 7)
      return OPENING_3S_SOLID_DIAMONDS;
    else if (clubs >= 7)
      return OPENING_3S_SOLID_CLUBS;
  }

  return OPENING_SIZE;
}


Openings Opening::threeSWeak() const
{
  const Openings op = Opening::threeSSolid();
  if (op != OPENING_SIZE)
    return op;

  if (spades >= 6)
    return OPENING_3S_WEAK_SPADES;
  else if (spades == 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_3S_WEAK_SPADES_MIN;
  else if (spades == 5 && spades == longest1 && longest2 <= 5)
    return OPENING_3S_WEAK_SPADES;
  else if (diamonds >= 7)
    return OPENING_3S_WEAK_BROKEN_DIAMONDS;
  else if (clubs >= 7)
    return OPENING_3S_WEAK_BROKEN_CLUBS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::threeSInt() const
{
  const Openings op = Opening::threeSSolid();
  if (op != OPENING_SIZE)
    return op;

  if (spades >= 6)
  {
    if (hcp <= 11)
      return OPENING_3S_WEAK_SPADES;
    else
      return OPENING_3S_INTERMED_SPADES;
  }
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::threeSStrong() const
{
  if (spades >= 7)
    return OPENING_3S_STRONG_SPADES;
  else if (spades == 6 && longest2 >= 5)
    return OPENING_3S_STRONG_SPADES_OTHER;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Three notrump -----------------

Openings Opening::threeNTWeak() const
{
  if (clubs >= 7 || diamonds >= 7)
  {
    if (solidFlag)
      return OPENING_3NT_WEAK_SOLID_MINOR;
    else
      return OPENING_3NT_WEAK_BROKEN_MINOR;
  }
  else if (hearts >= 7 || spades >= 7)
  {
    if (solidFlag)
      return OPENING_3NT_WEAK_SOLID_MAJOR;
    else
      return OPENING_3NT_WEAK_BROKEN_MAJOR;
  }
  else if (clubs >= 5 && diamonds >= 5)
    return OPENING_3NT_WEAK_MINORS;
  else if (hearts >= 5 && spades >= 5)
    return OPENING_3NT_WEAK_MAJORS;
  else if (clubs == 6 || diamonds == 6)
  {
    if (solidFlag)
      return OPENING_3NT_WEAK_AKQxxx_MINOR;
    else
      return OPENING_3NT_WEAK_BROKEN_MINOR;
  }
  else if (hearts == 6 || spades == 6)
  {
    if (solidFlag)
      return OPENING_3NT_WEAK_AKQxxx_MAJOR;
    else
      return OPENING_3NT_WEAK_BROKEN_MAJOR;
  }
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::threeNTInt() const
{
  if (longest1 >= 7 && solidFlag)
  {
    if (spades == longest1 || hearts == longest1)
      return OPENING_3NT_INTERMED_SOLID_MAJOR;
    else
      return OPENING_3NT_INTERMED_SOLID_MINOR;
  }
  else if (longest1 == 6 && solidFlag)
  {
    if (spades == longest1 || hearts == longest1)
      return OPENING_3NT_INTERMED_AKQxxx_MAJOR;
    else
      return OPENING_3NT_INTERMED_AKQxxx_MINOR;
  }
  else if (hcp >= 13 && (clubs >= 6 || diamonds >= 6))
    return OPENING_3NT_INTERMED_MINOR;
  else
    return threeNTWeak();
}


Openings Opening::threeNTStrong() const
{
  if (hcp >= 22 && longest1 <= 5 && longest4 >= 2)
    return OPENING_3NT_STRONG_BAL;
  else if (longest1 >= 7 && solidFlag)
  {
    if (spades == longest1 || hearts == longest1)
      return OPENING_3NT_STRONG_SOLID_MAJOR;
    else
      return OPENING_3NT_STRONG_SOLID_MINOR;
  }
  else if (longest1 == 6 && solidFlag)
  {
    if (spades == longest1 || hearts == longest1)
      return OPENING_3NT_STRONG_AKQxxx_MAJOR;
    else
      return OPENING_3NT_STRONG_AKQxxx_MINOR;
  }
  else if (spades >= 7)
    return OPENING_3NT_STRONG_SPADES;
  else if (hearts >= 7)
    return OPENING_3NT_STRONG_HEARTS;
  else if (spades >= 5 && hearts >= 5)
    return OPENING_3NT_STRONG_MAJORS;
  else if (clubs >= 6 || diamonds >= 6)
  {
    if (hcp <= 18)
      return OPENING_3NT_INTERMED_MINOR;
    else
      return OPENING_3NT_STRONG_MINOR;
  }
  else if (hcp >= 20)
    return OPENING_3NT_STRONG_OTHER;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Four clubs --------------------

Openings Opening::fourCWeak() const
{
  if (hearts >= 7)
    return OPENING_4C_WEAK_NAMYATS;
  else if (diamonds >= 7)
    return OPENING_4C_WEAK_DIAMONDS;
  else if (clubs >= 5 && (spades >= 5 || hearts >= 5))
    return OPENING_4C_WEAK_WITH_MAJ;
  else if (clubs >= 6)
    return OPENING_4C_WEAK_CLUBS;
  else if (clubs == 5 && diamonds == 5)
    return OPENING_4C_WEAK_CLUBS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourCInt() const
{
  if (hearts >= 6)
    return OPENING_4C_INTERMED_NAMYATS;
  else if (clubs >= 5 && (spades >= 5 || hearts >= 5))
    return OPENING_4C_INTERMED_WITH_MAJ;
  else if (clubs >= 6)
    return OPENING_4C_INTERMED_CLUBS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourCStrong() const
{
  if (hearts >= 6)
    return OPENING_4C_STRONG_NAMYATS;
  else if (clubs >= 7)
    return OPENING_4C_STRONG_CLUBS;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Four diamonds -----------------

Openings Opening::fourDWeak() const
{
  if (spades >= 7)
    return OPENING_4D_WEAK_NAMYATS;
  else if (hearts >= 7 || (hearts == 6 && diamonds <= 3))
    return OPENING_4D_WEAK_HEARTS;
  else if (diamonds >= 7)
    return OPENING_4D_WEAK_DIAMONDS;
  else if (diamonds >= 5 && (spades >= 5 || hearts >= 5))
    return OPENING_4D_WEAK_WITH_MAJ;
  else if (diamonds >= 6)
    return OPENING_4D_WEAK_DIAMONDS;
  else if (clubs == 5 && diamonds == 5)
    return OPENING_4D_WEAK_DIAMONDS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourDInt() const
{
  if (spades >= 6)
    return OPENING_4D_INTERMED_NAMYATS;
  else if (hearts >= 7)
    return OPENING_4D_INTERMED_HEARTS;
  else if (diamonds >= 5 && (spades >= 5 || hearts >= 5))
    return OPENING_4D_INTERMED_WITH_MAJ;
  else if (spades >= 5 && hearts >= 5)
    return OPENING_4D_INTERMED_MAJS;
  else if (diamonds >= 6)
    return OPENING_4D_INTERMED_DIAMONDS;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourDStrong() const
{
  if (spades >= 6)
    return OPENING_4D_STRONG_NAMYATS;
  else if (diamonds >= 7)
    return OPENING_4D_STRONG_DIAMONDS;
  else if (diamonds >= 5 && (spades >= 5 || hearts >= 5))
    return OPENING_4D_STRONG_WITH_MAJ;
  else
    return OPENING_4D_STRONG_OTHER;
}


// ----------------- Four hearts -------------------

Openings Opening::fourHWeak() const
{
  if (hearts >= 6)
    return OPENING_4H_WEAK_HEARTS;
  else if (spades >= 6 && hearts <= 4)
    return OPENING_4H_WEAK_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourHInt() const
{
  if (hearts >= 6)
    return OPENING_4H_INTERMED_HEARTS;
  else if (spades >= 6 && hearts <= 4)
    return OPENING_4H_INTERMED_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourHStrong() const
{
  if (hearts >= 6)
    return OPENING_4H_STRONG_HEARTS;
  else if (spades >= 6 && hearts <= 4)
    return OPENING_4H_STRONG_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Four spades -------------------

Openings Opening::fourSWeak() const
{
  if (spades >= 6)
    return OPENING_4S_WEAK_SPADES;
  else if (spades == 5)
    return OPENING_4S_WEAK_FIVE_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourSInt() const
{
  if (spades >= 6)
    return OPENING_4S_INTERMED_SPADES;
  else if (spades == 5)
    return OPENING_4S_INTERMED_FIVE_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


Openings Opening::fourSStrong() const
{
  if (spades >= 6)
    return OPENING_4S_STRONG_SPADES;
  else if (spades == 5)
    return OPENING_4S_STRONG_FIVE_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Four notrump ------------------

Openings Opening::fourNT() const
{
  if (clubs >= 5 && diamonds >= 5)
    return OPENING_4NT_MINS;
  else
    return OPENING_4NT_ACES;
}


// ----------------- Five clubs --------------------

Openings Opening::fiveC() const
{
  if (clubs >= 8)
    return OPENING_5C_CLUBS_LONG;
  else if (clubs >= 6 && clubs == longest1 &&
      longest1 + longest2 >= 11)
    return OPENING_5C_CLUBS_11_IN_TWO;
  else
    return OPENING_5C_CLUBS_OTHER;
}


// ----------------- Five diamonds------------------

Openings Opening::fiveD() const
{
  if (diamonds >= 8)
    return OPENING_5D_DIAMONDS_LONG;
  else if (diamonds >= 6 && diamonds == longest1 &&
      longest1 + longest2 >= 11)
    return OPENING_5D_DIAMONDS_11_IN_TWO;
  else
    return OPENING_5D_DIAMONDS_OTHER;
}


// ----------------- Five hearts -------------------

Openings Opening::fiveH() const
{
  if (hearts >= 8)
    return OPENING_5H_HEARTS_LONG;
  else if (hearts >= 6 && hearts == longest1 &&
      longest1 + longest2 >= 12)
    return OPENING_5H_HEARTS_11_IN_TWO;
  else
    return OPENING_5H_HEARTS_OTHER;
}


// ----------------- Five spades -------------------

Openings Opening::fiveS() const
{
  if (spades >= 8)
    return OPENING_5S_SPADES_LONG;
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Five notrump ------------------

Openings Opening::fiveNT() const
{
  return OPENING_UNCLASSIFIED;
}


// ----------------- Six clubs ---------------------

Openings Opening::sixC() const
{
  if (clubs >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_6C_CLUBS;
    else
      return OPENING_6C_CLUBS_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Six diamonds ------------------

Openings Opening::sixD() const
{
  if (diamonds >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_6D_DIAMONDS;
    else
      return OPENING_6D_DIAMONDS_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Six hearts ------------------

Openings Opening::sixH() const
{
  if (hearts >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_6H_HEARTS;
    else
      return OPENING_6H_HEARTS_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Six spades ------------------

Openings Opening::sixS() const
{
  if (spades >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_6S_SPADES;
    else
      return OPENING_6S_SPADES_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Six notrump -----------------

Openings Opening::sixNT() const
{
  return OPENING_UNCLASSIFIED;
}


// ----------------- Seven clubs -------------------

Openings Opening::sevenC() const
{
  if (clubs >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_7C_CLUBS;
    else
      return OPENING_7C_CLUBS_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Seven diamonds ----------------

Openings Opening::sevenD() const
{
  if (diamonds >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_7D_DIAMONDS;
    else
      return OPENING_7D_DIAMONDS_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Seven hearts ----------------

Openings Opening::sevenH() const
{
  if (hearts >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_7H_HEARTS;
    else
      return OPENING_7H_HEARTS_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Seven spades ----------------

Openings Opening::sevenS() const
{
  if (spades >= 8 && longest1 + longest2 >= 11)
  {
    if (longest4 == 0)
      return OPENING_7S_SPADES;
    else
      return OPENING_7S_SPADES_NOVOID;
  }
  else
    return OPENING_UNCLASSIFIED;
}


// ----------------- Seven notrump ---------------

Openings Opening::sevenNT() const
{
  return OPENING_UNCLASSIFIED;
}


Openings Opening::classify(
  const string& call,
  const Valuation& valuation,
  const vector<unsigned>& params)
{
  Opening::set(valuation, params);

  if (classifyMap.find(call) == classifyMap.end())
  {
    cout << "Could not find bid " << call << endl;
    assert(false);
  }

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

