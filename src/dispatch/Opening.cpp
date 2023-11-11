/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>

#include "Opening.h"
#include "Openings.h"

#include "../analysis/Valuation.h"


void Opening::set(
  const Valuation& valuation,
  const vector<unsigned>& params)
{
  clubs = valuation.getSuitParam(BRIDGE_CLUBS, VS_LENGTH);
  diamonds = valuation.getSuitParam(BRIDGE_DIAMONDS, VS_LENGTH);
  hearts = valuation.getSuitParam(BRIDGE_HEARTS, VS_LENGTH);
  spades = valuation.getSuitParam(BRIDGE_SPADES, VS_LENGTH);

  hcp = params[PASS_HCP];
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

  unsigned numSuits = 0;
  if (spades >= 4) numSuits++;
  if (hearts >= 4) numSuits++;
  if (diamonds >= 4) numSuits++;
  if (clubs >= 4) numSuits++;

  if (numSuits == 3)
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

  unsigned numSuits = 0;
  if (spades == 4) numSuits++;
  if (hearts >= 4) numSuits++;
  if (diamonds >= 4) numSuits++;
  if (clubs >= 4) numSuits++;

  if (numSuits == 3)
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

    unsigned numSuits = 1;
    if (hearts >= 4) numSuits++;
    if (diamonds >= 4) numSuits++;
    if (clubs >= 4) numSuits++;

    if (numSuits == 3)
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


Openings Opening::classifyTwoNT(const Valuation& valuation) const
{
  const unsigned longest1 = valuation.getDistParam(VD_L1);
  const unsigned longest2 = valuation.getDistParam(VD_L2);
  const unsigned longest4 = valuation.getDistParam(VD_L4);

  if (hcp >= 15)
  {
    // Kind-of semi-balanced.
    if (longest1 <= 5 && longest2 <= 4 && longest4 >= 2)
      return OPENING_2NT_STRONG_SBAL;
    else
      return OPENING_2NT_STRONG_OTHER;
  }

  if (hcp <= 10)
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

  if (clubs >= 6 || diamonds >= 6)
    return OPENING_2NT_OPEN_ONE_MIN;
  else if (longest1 >= 5 && longest2 >= 4)
    return OPENING_2NT_OPEN_TWO_SUITER;
  else
    return OPENING_2NT_OPEN_OTHER;
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
    return Opening::classifyTwoNT(valuation);
  else
    return OPENING_SIZE;
}


