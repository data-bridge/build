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

      array<unsigned, BRIDGE_SUITS> known;
      for (unsigned s = 0; s < BRIDGE_SUITS; s++)
        known[s] =  BRIDGE_TRICKS -
          (distribution[p][s] + distribution[dummy][s]);

      knowledges[p][o].init(known);
    }
  }
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

      if (suit != leadToTrick)
      {
        // Not following suit.
        knowledgePlayer.fixAtMin(leadToTrick);
        fourthPlayer.fixAtMax(leadToTrick);
      }

      // The actual suit of the key player changes no matter what.
      knowledgePlayer.increaseKnown(suit);
      knowledgePlayer.increaseMinBy(fourthPlayer, suit, 1);

      // A single suit cannot have max higher than 13 minus sum of 
      // other min's.
      knowledgePlayer.reduceMax(fourthPlayer);
      fourthPlayer.reduceMax(knowledgePlayer);

      // If one of the two other players has 4* min==max, 
      // then distribution became known if it wasn't.
      // If also known == min, then single dummy became known.
      if (! points[keyPlayer].distKnown)
      {
        if (knowledgePlayer.distKnown())
        {
          points[keyPlayer].distKnown = true;
          points[keyPlayer].distKnown = true;

          knowledgePlayer.adjustToCompleteness(fourthPlayer);
        }
        else if (fourthPlayer.distKnown())
        {
          points[keyPlayer].distKnown = true;
          points[keyPlayer].distKnown = true;

          fourthPlayer.adjustToCompleteness(knowledgePlayer);
        }
      }

      if (! points[keyPlayer].dealKnown)
      {
        if (knowledgePlayer.dealKnown() || fourthPlayer.dealKnown())
        {
          points[keyPlayer].distKnown = true;
        }
      }
    }
  }
}

