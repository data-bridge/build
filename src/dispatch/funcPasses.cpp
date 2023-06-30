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

#include "../stats/Stats1D.h"
#include "../stats/Stats2D.h"
#include "../stats/StatsInfo.h"

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
  { "CCCC", 0, 1200, 20}, // TODO Check
  { "ZP", 0, 80, 1} // TODO Check
};


void initPass1DStats(vector<vector<vector<Stats1D>>>& stats)
{
  stats.resize(BRIDGE_PLAYERS);
  for (size_t prel = 0; prel < BRIDGE_PLAYERS; prel++)
  {
    stats[prel].resize(BRIDGE_VUL_SIZE);
    for (size_t vul = 0; vul < BRIDGE_VUL_SIZE; vul++)
    {
      stats[prel][vul].resize(PASS_SIZE);
      for (size_t param = 0; param < PASS_SIZE; param++)
        stats[vul][prel][param].set(LOCAL_DATA[param]);
    }
  }
}


void initPass2DStats(vector<vector<vector<Stats2D>>>& stats)
{
  stats.resize(BRIDGE_PLAYERS);
  for (size_t prel = 0; prel < BRIDGE_PLAYERS; prel++)
  {
    stats[prel].resize(BRIDGE_VUL_SIZE);
    for (size_t vul = 0; vul < BRIDGE_VUL_SIZE; vul++)
    {
      // So currently
      // 0: HCP, CCCC
      // 1: HCP, Zar
      // 2: CCCC, Zar
      stats[prel][vul].resize(PASS_SIZE * (PASS_SIZE-1) / 2);

      size_t p_run = 0;
      for (size_t param1 = 0; param1+1 < PASS_SIZE; param1++)
      {
        for (size_t param2 = param1+1; param2 < PASS_SIZE; param2++)
        {
          stats[vul][prel][p_run].set1(LOCAL_DATA[param1]);
          stats[vul][prel][p_run].set2(LOCAL_DATA[param2]);
          
          p_run++;
        }
      }
    }
  }
}

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


void addPassStats1D(
  vector<Stats1D>& stats1D, 
  vector<unsigned>& params, 
  const bool flag)
{
  for (unsigned localParam = 0; localParam < PASS_SIZE; localParam++)
    stats1D[localParam].add(params[localParam], flag);
}


void addPassStats2D(
  vector<Stats2D>& stats2D, 
  vector<unsigned>& params, 
  const bool flag)
{
  size_t p_run = 0;
  for (size_t param1 = 0; param1+1 < PASS_SIZE; param1++)
  {
    for (size_t param2 = param1+1; param2 < PASS_SIZE; param2++)
    {
      stats2D[p_run].add(params[param1], params[param2], flag);
      p_run++;
    }
  }
}


string strPassStats(
  const vector<vector<vector<Stats1D>>>& stats1D,
  const vector<vector<vector<Stats2D>>>& stats2D)
{
  stringstream ss;
  for (size_t prel = 0; prel < BRIDGE_PLAYERS; prel++)
  {
    for (size_t vul = 0; vul < BRIDGE_VUL_SIZE; vul++)
    {
      ss << "Player pos.   " << prel << "\n";
      ss << "Vulnerability " << VUL_NAMES_PBN[vul] << "\n";
      ss << string('-', 18) << "\n\n";

      for (size_t param = 0; param < PASS_SIZE; param++)
        ss << stats1D[prel][vul][param].str() << "\n";

      size_t p_run = 0;
      for (size_t param1 = 0; param1+1 < PASS_SIZE; param1++)
      {
        for (size_t param2 = param1+1; param2 < PASS_SIZE; param2++)
        {
          ss << stats2D[prel][vul][p_run].str() << "\n";
          p_run++;
        }
      }
    }
  }

  return ss.str();
}


string passStats(const Group& group)
{
  const vector<string> onePass = {"P"};
  const vector<string> twoPasses = {"P", "P"};
  const vector<string> threePasses = {"P", "P", "P"};
  const vector<string> fourPasses = {"P", "P", "P", "P"};

  // Relative player (dealer = 0), vul, localParams.
  vector<vector<vector<Stats1D>>> stats1D;
  initPass1DStats(stats1D);

  // Relative player (dealer = 0), vul, localParams.
  vector<vector<vector<Stats2D>>> stats2D;
  initPass2DStats(stats2D);

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

        const Vul vul = instance.getVul();
        const bool onePassFlag = instance.auctionStarts(onePass);
        addPassStats1D(stats1D[0][vul], params[0], onePassFlag);
        addPassStats2D(stats2D[0][vul], params[0], onePassFlag);

        if (onePassFlag)
        {
          const bool twoPassesFlag = instance.auctionStarts(twoPasses);
          addPassStats1D(stats1D[1][vul], params[1], twoPassesFlag);
          addPassStats2D(stats2D[1][vul], params[1], twoPassesFlag);

          if (twoPassesFlag)
          {
            const bool threePassesFlag = 
              instance.auctionStarts(threePasses);
            addPassStats1D(stats1D[2][vul], params[2], threePassesFlag);
            addPassStats2D(stats2D[2][vul], params[2], threePassesFlag);

            if (threePassesFlag)
            {
              const bool fourPassesFlag = 
                instance.auctionStarts(fourPasses);
              addPassStats1D(stats1D[3][vul], params[3], fourPassesFlag);
              addPassStats2D(stats2D[3][vul], params[3], fourPassesFlag);
            }
          }
        }
      }
    }
  }

  return strPassStats(stats1D, stats2D);

}


void dispatchPasses(
  const Group& group,
  ostream& flog)
{
  try
  {
    flog << passStats(group);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

