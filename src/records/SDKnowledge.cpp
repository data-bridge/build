/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/

#include <assert.h>
#include <mutex>

#include "SDKnowledge.h"
#include "SDPoints.h"

#include "../handling/Bexcept.h"
#include "../handling/Bdiff.h"

static mutex mtx;

static bool setTables = false;

// [dummy[key player][other player] = remaining fourth player.

static array<array<array<unsigned, BRIDGE_PLAYERS>, 
  BRIDGE_PLAYERS>, BRIDGE_PLAYERS> lastPlayer;


SDKnowledge::SDKnowledge()
{
  if (! setTables)
  {
    mtx.lock();
    if (! setTables)
      SDKnowledge::setTable();
    setTables = true;
    mtx.unlock();
  }
}


void SDKnowledge::setTable()
{
  for (unsigned pdummy = 0; pdummy < BRIDGE_PLAYERS; pdummy++)
  {
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      if (p == pdummy)
        continue;
      for (unsigned q = 0; q < BRIDGE_PLAYERS; q++)
      {
        if (q == pdummy || q == p)
          continue;
        for (unsigned r = 0; r < BRIDGE_PLAYERS; q++)
        {
          if (r != pdummy && r != p && r != q)
          {
            // So only one match.
            lastPlayer[pdummy][p][q] = r;
            break;
          }
        }
      }
    }
  }
}


void SDKnowledge::init(
  const unsigned distribution[BRIDGE_PLAYERS][BRIDGE_SUITS],
  const unsigned dummyIn)
{
  dummy = dummyIn;

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    // Dummy doesn't need to pay attention.
    if (p == dummy)
        continue;

    for (unsigned o = 0; o < BRIDGE_PLAYERS; o++)
    {
      if (o == dummy || o == p)
        continue;

      auto& knowledge = knowledges[p][o];

      for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      {
        knowledge.suits[s].min = 0;
        knowledge.suits[s].max = 13 - 
          (distribution[p][s] + distribution[dummy][s]);
      }
      knowledge.unknown = 13;
      knowledge.minSum = 0;
    }
  }
}


void SDKnowledge::followSuit(
  Knowledge& knowledgePlayer,
  Knowledge& fourthPlayer,
  const unsigned suit) const
{
  auto& kpsuit = knowledgePlayer.suits[suit];
  auto& opsuit = fourthPlayer.suits[suit];

  if (kpsuit.min == kpsuit.max)
  {
    if (kpsuit.known == kpsuit.min)
    {
      kpsuit.known++;
      knowledgePlayer.unknown--;
    }
  }
  else
  {
    kpsuit.min++;
    kpsuit.known++;
    knowledgePlayer.minSum++;
    knowledgePlayer.unknown--;

    if (opsuit.max > 0)
      opsuit.max--;
  }
}


void SDKnowledge::discardSuit(
  Knowledge& knowledgePlayer,
  Knowledge& fourthPlayer,
  const unsigned leadToTrick) const
{
  auto& kplead = knowledgePlayer.suits[leadToTrick];
  auto& oplead = fourthPlayer.suits[leadToTrick];

  // The key player has exactly the known cards of the suit led.
  const unsigned deltaOwn = kplead.min - kplead.known;
  knowledgePlayer.unknown -= deltaOwn;
  kplead.max = kplead.min;
  kplead.known = kplead.min;

  // The other player has the rest.
  const unsigned deltaOther = oplead.max - oplead.known;
  fourthPlayer.unknown -= deltaOther;
  fourthPlayer.minSum += oplead.max - oplead.min;
  oplead.min = oplead.max;
  oplead.known = oplead.max;
  fourthPlayer.unknown -= deltaOther;
}


void SDKnowledge::reduceMax(
  Knowledge& knowledgePlayer,
  Knowledge& fourthPlayer) const
{
  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    auto& kpsuit = knowledgePlayer.suits[suit];
    const unsigned otherMin = knowledgePlayer.minSum - kpsuit.min;
    const unsigned highestMax = 13 - otherMin;

    if (kpsuit.max > highestMax)
    {
      const unsigned delta = kpsuit.max - highestMax;
      kpsuit.max = highestMax;

      auto& opsuit = fourthPlayer.suits[suit];
      opsuit.min += delta;
    }
  }
}


void SDKnowledge::completeMirror(
  Knowledge& player1,
  Knowledge& player2) const
{
  // Player1 just needs the max values to be reduced to min.
  // Player2 needs to be mirrored.

  for (unsigned suit = 0; suit < BRIDGE_SUITS; suit++)
  {
    const unsigned sum1 = player1.suits[suit].min + player2.suits[suit].max;
    const unsigned sum2 = player1.suits[suit].max + player2.suits[suit].min;
    assert(sum1 == sum2);

    player1.suits[suit].max = player1.suits[suit].min;

    player2.suits[suit].min = sum1 - player1.suits[suit].min;
    player2.suits[suit].max = player2.suits[suit].min;
  }

  player2.minSum = 13;
}


void SDKnowledge::set(
  const vector<unsigned>& suitsPlayed,
  const vector<Player>& playedBy,
  vector<SDPoints>& points)
{
  for (unsigned keyPlayer = 0; keyPlayer < BRIDGE_SUITS; keyPlayer++)
  {
    if (keyPlayer == dummy)
      continue;

    if (points[keyPlayer].distKnown)
      continue;

    unsigned leadToTrick;
    for (unsigned i = 0; i < playedBy.size(); i++)
    {
      // So (suit, player) is the card to be played, not from dummy.
      const unsigned player = static_cast<unsigned>(playedBy[i]);
      if (player == dummy)
        continue;

      const unsigned suit = suitsPlayed[i];
      if ((i % 4) == 0)
        leadToTrick = suit;

      // This is the player who is trying to figure out the deal.
      // It doesn't help him to see cards from dummy or his own hand.
      if (keyPlayer == dummy || keyPlayer == player)
        continue;

      auto& knowledgePlayer = knowledges[keyPlayer][player];
      const unsigned lp = lastPlayer[dummy][keyPlayer][player];
      auto& fourthPlayer = knowledges[keyPlayer][lp];

      if (suit == leadToTrick)
      {
        SDKnowledge::followSuit(knowledgePlayer, fourthPlayer, suit);
      }
      else
      {
        // Not following suit.
        SDKnowledge::discardSuit(knowledgePlayer, fourthPlayer, leadToTrick);

        // Also the actual suit of the key player changes.
        SDKnowledge::followSuit(knowledgePlayer, fourthPlayer, suit);
      }

      // A single suit cannot have max higher than 13 minus sum of 
      // other min's.
      SDKnowledge::reduceMax(knowledgePlayer, fourthPlayer);
      SDKnowledge::reduceMax(fourthPlayer, knowledgePlayer);

      // If one of the two other players has 4* min==max, 
      // then distribution became known if it wasn't.
      // If also known == min, then single dummy became known.
      if (! points[keyPlayer].distKnown)
      {
        if (knowledgePlayer.minSum == 13)
        {
          points[keyPlayer].distKnown = true;
          points[keyPlayer].distKnown = true;

          SDKnowledge::completeMirror(knowledgePlayer, fourthPlayer);
        }
        else if (fourthPlayer.minSum == 13)
        {
          points[keyPlayer].distKnown = true;
          points[keyPlayer].distKnown = true;

          SDKnowledge::completeMirror(fourthPlayer, knowledgePlayer);
        }
      }

      if (! points[keyPlayer].dealKnown)
      {
        if (knowledgePlayer.unknown == 0 ||
            fourthPlayer.unknown == 0)
        {
          points[keyPlayer].distKnown = true;
        }
      }
    }
  }
}

