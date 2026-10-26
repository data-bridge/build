/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SDKNOWLEDGE_H
#define BRIDGE_SDKNOWLEDGE_H

#include <vector>
#include <array>
#include <string>

#include "../include/bridge.h"

using namespace std;

struct SDPoints;


class SDKnowledge
{
  private:

    struct SuitKnowledge
    {
      unsigned min;
      unsigned max;
      unsigned known;
    };

    struct Knowledge
    {
      // What we know about a single player in the context
      // of one of the other two players (dummy not counting).
      array<SuitKnowledge, BRIDGE_SUITS> suits;
      unsigned unknown;
      unsigned minSum;
    };

    // What one player knows about the other players.
    // Overkill, as we only need to keep track of two of them.
    using OtherKnowledge = array<Knowledge, BRIDGE_PLAYERS>;

    // What each of three players (not counting dummy)
    // knows about the other two.
    array<OtherKnowledge, BRIDGE_PLAYERS> knowledges;

    unsigned dummy;


    void setTable();

    void followSuit(
      Knowledge& knowledgePlayer,
      Knowledge& fourthPlayer,
      const unsigned suit) const;

    void discardSuit(
      Knowledge& knowledgePlayer,
      Knowledge& fourthPlayer,
      const unsigned leadToTrick) const;

    void reduceMax(
      Knowledge& knowledgePlayer,
      Knowledge& fourthPlayer) const;

    void completeMirror(
      Knowledge& player1,
      Knowledge& player2) const;


  public:

    SDKnowledge();

    void init(
      const unsigned distribution[BRIDGE_PLAYERS][BRIDGE_SUITS],
      const unsigned dummyIn);

    void set(
      const vector<unsigned>& suitsPlayed,
      const vector<Player>& playedBy,
      vector<SDPoints>& points);
};

#endif

