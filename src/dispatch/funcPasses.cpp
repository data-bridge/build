/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "../records/Group.h"

#include "funcPasses.h"

#include "../stats/ParamStats1D.h"
#include "../stats/ParamStats2D.h"

#include "../include/bridge.h"
#include "../handling/Bexcept.h"


// Effectively a subset of CompositeParams, but more compactly 
// numbered for memory efficiency.

enum LocalParams
{
  PASS_HCP = 0,
  PASS_CCCC = 1,
  PASS_ZAR = 2,
  PASS_SIZE = 3
};

// The mapping between the two.

static vector<CompositeParams> LOCAL_TO_COMPOSITE =
{
  VC_HCP,
  VC_CCCC,
  VC_ZAR
};

static vector<StatsInfo> LOCAL_DATA =
{
  { "HCP", 0, 40, 1},
  { "CCCC", 0, 1200, 20}, // TODO Check maxima
  { "ZP", 0, 80, 1} // TODO Check maxima
};

static vector<string> LOCAL_NAMES =
{
  "Player pos.", "Vulnerability", "Params"
};

const static vector<string> onePass = {"P"};
const static vector<string> twoPasses = {"P", "P"};
const static vector<string> threePasses = {"P", "P", "P"};
const static vector<string> fourPasses = {"P", "P", "P", "P"};


void setPassParams(
  vector<vector<unsigned>>& params,
  const vector<unsigned>& relPlayers,
  const vector<Valuation>& valuations)
{
  params.resize(BRIDGE_PLAYERS);
  for (unsigned prel = 0; prel < BRIDGE_PLAYERS; prel++)
  {
    params[prel].resize(PASS_SIZE);
    for (unsigned localParam = 0; localParam < PASS_SIZE; localParam++)
    {
      params[prel][localParam] = 
        valuations[relPlayers[prel]].getCompositeParam(
          LOCAL_TO_COMPOSITE[localParam]);
    }
  }
}


void passStats(
  const Group& group,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D)
{
  paramStats1D.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
    LOCAL_NAMES, LOCAL_DATA);

  paramStats2D.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
    LOCAL_NAMES, LOCAL_DATA);

  for (auto &segment: group)
  {
    for (auto &bpair: segment)
    {
      const Board& board = bpair.board;
      const unsigned dealer = static_cast<unsigned>(board.getDealer());
      const vector<Valuation>& valuations = board.getValuations();

      // 0 is the dealer.
      const vector<unsigned> relPlayers =
        { dealer, (dealer + 1) % 4, (dealer + 2) % 4, (dealer + 3) % 4 };

      // First dimension is the player -- 0 is the dealer.
      // Second dimension is the local parameter.
      vector<vector<unsigned>> params;
      setPassParams(params, relPlayers, valuations);

      for (unsigned i = 0; i < board.countAll(); i++)
      {
        const Instance& instance = board.getInstance(i);
        if (board.skipped(i))
          continue;

        // Relative to dealer, so "NS" is the dealer.
        // TODO Make an instance.getRelativeVul(dealer)?
        Vul vul = instance.getVul();
        if (dealer == BRIDGE_EAST || dealer == BRIDGE_WEST)
        {
          if (vul == BRIDGE_VUL_NORTH_SOUTH)
            vul = BRIDGE_VUL_EAST_WEST;
          else if (vul == BRIDGE_VUL_EAST_WEST)
            vul = BRIDGE_VUL_NORTH_SOUTH;
        }

        const bool onePassFlag = instance.auctionStarts(onePass);
        paramStats1D.add(0, vul, params[0], onePassFlag);
        paramStats2D.add(0, vul, params[0], onePassFlag);

        if (onePassFlag)
        {
          const bool twoPassesFlag = instance.auctionStarts(twoPasses);
          paramStats1D.add(1, vul, params[1], twoPassesFlag);
          paramStats2D.add(1, vul, params[1], twoPassesFlag);

          if (twoPassesFlag)
          {
            const bool threePassesFlag = 
              instance.auctionStarts(threePasses);
            paramStats1D.add(2, vul, params[2], threePassesFlag);
            paramStats2D.add(2, vul, params[2], threePassesFlag);

            if (threePassesFlag)
            {
              const bool fourPassesFlag = 
                instance.auctionStarts(fourPasses);
              paramStats1D.add(3, vul, params[3], fourPassesFlag);
              paramStats2D.add(3, vul, params[3], fourPassesFlag);
            }
          }
        }
      }
    }
  }
}


void dispatchPasses(
  const Group& group,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D,
  ostream& flog)
{
  try
  {
    passStats(group, paramStats1D, paramStats2D);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

