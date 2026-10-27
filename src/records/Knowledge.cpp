/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/

#include <assert.h>

#include "Knowledge.h"


void Knowledge::init(const array<unsigned, BRIDGE_SUITS>& known)
{
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    suits[s].min = 0;
    suits[s].max = known[s];
  }
  unknown = BRIDGE_TRICKS;
  minSum = 0;
}


void Knowledge::increaseKnown(const unsigned suit)
{
  auto& kpsuit = suits[suit];
  if (kpsuit.known < kpsuit.max)
  {
    kpsuit.known++;
    unknown--;
  }
}


void Knowledge::increaseMinBy(
  Knowledge& other,
  const unsigned suit,
  const unsigned delta)
{
  auto& kpsuit = suits[suit];
  if (kpsuit.min < kpsuit.max)
  {
    kpsuit.min += delta;
    minSum += delta;

    auto& opsuit = other.suits[suit];
    opsuit.max -= delta;
  }
}


void Knowledge::fixAtMin(const unsigned leadToTrick)
{
  // This player has exactly the minimum cards of the suit led.
  auto& kplead = suits[leadToTrick];
  const unsigned deltaOwn = kplead.min - kplead.known;
  unknown -= deltaOwn;
  kplead.max = kplead.min;
  kplead.known = kplead.min;
}


void Knowledge::fixAtMax(const unsigned leadToTrick)
{
  // This player has the most they can have.
  auto& oplead = suits[leadToTrick];
  const unsigned deltaOther = oplead.max - oplead.known;
  unknown -= deltaOther;
  minSum += oplead.max - oplead.min;
  oplead.min = oplead.max;
  oplead.known = oplead.max;
}


void Knowledge::reduceMax(Knowledge& other)
{
  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    auto& kpsuit = suits[suit];
    const unsigned otherMin = other.minSum - kpsuit.min;
    const unsigned highestMax = BRIDGE_TRICKS - otherMin;

    if (kpsuit.max > highestMax)
    {
      const unsigned delta = kpsuit.max - highestMax;
      other.increaseMinBy(* this, suit, delta);
    }
  }
}


void Knowledge::adjustToCompleteness(Knowledge& other)
{
  // We are complete.

  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    const unsigned sum1 = suits[suit].min + other.suits[suit].max;
    const unsigned sum2 = suits[suit].max + other.suits[suit].min;
    assert(sum1 == sum2);

    // Reduce our own max values.
    suits[suit].max = suits[suit].min;

    // Increase the other side's min values.
    other.suits[suit].min = sum1 - suits[suit].min;
    other.suits[suit].max = other.suits[suit].min;
  }

  other.minSum = BRIDGE_TRICKS;
}


bool Knowledge::distKnown() const
{
  return (minSum == BRIDGE_TRICKS);
}


bool Knowledge::dealKnown() const
{
  return (unknown == 0);
}


string Knowledge::strHeader() const
{
  string s = "";
  for (unsigned i = 0; i < BRIDGE_SUITS; i++)
    s += "mMk ";

  s += "um";
  return s;
}


string Knowledge::str() const
{
  string s = "";
  for (unsigned i = 0; i < BRIDGE_SUITS; i++)
    s += suits[i].str() + " ";

  s += HEX[unknown];
  s += HEX[minSum];
  return s;
}
