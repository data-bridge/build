/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "../control/Options.h"

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


string strBidData(
  const Board& board,
  const Instance& instance,
  const Vul vul,
  const Player player1,
  const Player player2,
  const vector<vector<unsigned>>& params,
  const unsigned pno)
{
  stringstream ss;

  ss << board.strDeal(BRIDGE_FORMAT_TXT);
  ss << instance.strPlayers(BRIDGE_FORMAT_TXT);
  ss << instance.strAuction(BRIDGE_FORMAT_TXT);

  string sv;
  if (vul == BRIDGE_VUL_NONE)
    sv = "Vul: none";
  else if (vul == BRIDGE_VUL_BOTH)
    sv = "Vul: both";
  else if (vul == BRIDGE_VUL_NORTH_SOUTH)
  {
    if (player1 == BRIDGE_NORTH || player1 == BRIDGE_SOUTH)
      sv = "Vul: We";
    else
      sv = "Vul: They";
  }
  else
  {
    if (player1 == BRIDGE_NORTH || player1 == BRIDGE_SOUTH)
      sv = "Vul: They";
    else
      sv = "Vul: We";
  }
  ss << sv << "\n";

  ss << "HCP: " << params[pno][0] << endl;

  ss << "Hand: " << board.strHand(player1, BRIDGE_FORMAT_TXT)  << endl;

  ss << "Players: " << 
    instance.strPlayer(player1, BRIDGE_FORMAT_TXT) << " - " << 
    instance.strPlayer(player2, BRIDGE_FORMAT_TXT) << "\n";

  ss << "Bid: " << instance.strCall(pno, BRIDGE_FORMAT_TXT) << "\n";

  ss << instance.strResult(BRIDGE_FORMAT_TXT);

  return ss.str();
}


void passStats(
  const Group& group,
  const Options& options,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D)
{
  paramStats1D.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
    LOCAL_NAMES, LOCAL_DATA);

  paramStats2D.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
    LOCAL_NAMES, LOCAL_DATA);

unsigned bno = 0;
  for (auto &segment: group)
  {
    for (auto &bpair: segment)
    {
bno++;
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
        const bool oneDistFlag = valuations[relPlayers[0]].distMatch(
          options.distMatcher);

        if (oneDistFlag)
        {
          paramStats1D.add(0, vul, params[0], onePassFlag);
          paramStats2D.add(0, vul, params[0], onePassFlag);

/*
          if (! onePassFlag && params[0][0] < 10)
          // if (onePassFlag && params[0][0] > 12)
          {
            cout << strBidData(
              board, 
              instance, 
              instance.getVul(), 
              static_cast<Player>(relPlayers[0]),
              static_cast<Player>(relPlayers[2]),
              params, 
              0);
          }
*/

        }

        if (onePassFlag)
        {
          const bool twoPassesFlag = instance.auctionStarts(twoPasses);
          const bool twoDistFlag = valuations[relPlayers[1]].distMatch(
            options.distMatcher);

          if (twoDistFlag)
          {
            paramStats1D.add(1, vul, params[1], twoPassesFlag);
            paramStats2D.add(1, vul, params[1], twoPassesFlag);

/*
            if (! twoPassesFlag && params[1][0] < 10)
            // if (twoPassesFlag && params[1][0] > 12)
            {
              cout << strBidData(
                board, 
                instance, 
                instance.getVul(), 
                static_cast<Player>(relPlayers[1]),
                static_cast<Player>(relPlayers[3]),
                params, 
                1);
            }
 */

          }

          if (twoPassesFlag)
          {
            const bool threePassesFlag = 
              instance.auctionStarts(threePasses);
            const bool threeDistFlag = valuations[relPlayers[2]].distMatch(
              options.distMatcher);

            if (threeDistFlag)
            {
              // TODO TMP to misuse table #2 for 11 HCP.
              paramStats1D.add(2, vul, params[2], threePassesFlag);
              paramStats2D.add(2, vul, params[2], threePassesFlag);
/*
            if (! threePassesFlag && params[2][0] < 10)
            // if (threePassesFlag && params[2][0] > 12)
            {
              cout << strBidData(
                board, 
                instance, 
                instance.getVul(), 
                static_cast<Player>(relPlayers[0]),
                static_cast<Player>(relPlayers[2]),
                params, 
                2);
            }
*/
            }

            if (threePassesFlag)
            {
              const bool fourPassesFlag = 
                instance.auctionStarts(fourPasses);
              const bool fourDistFlag = valuations[relPlayers[3]].distMatch(
                options.distMatcher);

              if (fourDistFlag)
              {
// Kludge to use Pearson instead of Zar points.
// auto params2 = params[3];
// auto spades = valuations[relPlayers[3]].handDist() >> 8;
// params2[2] = params2[0] + spades;

// Kludge to get spades vs controls.
// unsigned spades = static_cast<unsigned>(valuations[relPlayers[3]].handDist() >> 8);
// unsigned hcp = static_cast<unsigned>(params[3][0]);
// unsigned controls = static_cast<unsigned>(params[3][2] - params[3][0] - 9);

/*
                if (hcp == 11)
                {
if (controls >= 4)
  controls = 4;
else
  controls = 3;
params[3][0] = spades;
params[3][2] = controls + 10;
                paramStats1D.add(2, vul, params[3], fourPassesFlag);
                paramStats2D.add(2, vul, params[3], fourPassesFlag);
                }
                else if (hcp == 12)
                {
if (controls >= 5)
  controls = 5;
else
  controls = 4;
params[3][0] = spades;
params[3][2] = controls + 10;
*/
                paramStats1D.add(3, vul, params[3], fourPassesFlag);
                paramStats2D.add(3, vul, params[3], fourPassesFlag);

/* */
                if (! fourPassesFlag && params[3][0] < 10)
                // if (fourPassesFlag && params[3][0] > 12)
                {
                  cout << strBidData(
                    board, 
                    instance, 
                    instance.getVul(), 
                    static_cast<Player>(relPlayers[1]),
                    static_cast<Player>(relPlayers[3]),
                    params, 
                    3);
                }
/* */

                // }
              }
            }
          }
        }
      }
    }
  }
}


void dispatchPasses(
  const Group& group,
  const Options& options,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D,
  ostream& flog)
{
  try
  {
    passStats(group, options, paramStats1D, paramStats2D);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

