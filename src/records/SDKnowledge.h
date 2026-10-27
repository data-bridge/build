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

#include "Knowledge.h"

#include "../include/bridge.h"

using namespace std;

struct SDPoints;


class SDKnowledge
{
  private:

    // What one player knows about the other players.
    // Overkill, as we only need to keep track of two of them.
    using OtherKnowledge = array<Knowledge, BRIDGE_PLAYERS>;

    // What each of three players (not counting dummy)
    // knows about the other two.
    array<OtherKnowledge, BRIDGE_PLAYERS> knowledges;

    unsigned dummy;


    void setTable();


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

