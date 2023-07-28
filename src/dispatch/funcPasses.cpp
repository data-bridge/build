/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>
#include <cassert>

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

const static vector<vector<string>> sequentialPasses =
{
  {"P"},
  {"P", "P"},
  {"P", "P", "P"},
  {"P", "P", "P", "P"}
};

struct FilterParams
{
  // Only consider a specific player distribution?
  bool distFilterFlag;

  // Only consider a specific HCP value?
  bool hcpFlag;
  unsigned hcpValue;

  // Use spades and controls instead of HCP and ZP?
  bool spadesControlFlag;

  // Only consider a specific player name?
  bool playerFlag;
  string playerTag;
};


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


bool isAboveOneLevel(const string& bid)
{
  if (bid.size() <= 1 || bid == "Pass")
    return false;
  else
    return (bid.at(0) >= '2');
}


string strBidData(
  const Board& board,
  const Instance& instance,
  const vector<unsigned>& relPlayers,
  const vector<vector<unsigned>>& params,
  const unsigned pno,
  const unsigned range,
  const string& matchTag)
{
  stringstream ss;

  const Vul vul = instance.getVul();
  const Player player1 = static_cast<Player>(relPlayers[pno]);
  const Player player2 = static_cast<Player>(relPlayers[(pno+2) % 4]);

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

  if (range == 0)
  {
    // An opening bid from a weak hand.
    ss << "Tag: weak_H" << pno+1 << "_" << matchTag << "\n";
  }
  else if (range == 1)
  {
    // An high opening bid from a middling hand.
    ss << "Tag: mid_H" << pno+1 << "_" << matchTag << "\n";
  }
  else
  {
    // An opening pass from an unusually strong hand.
    ss << "Tag: strong_H" << pno+1 << "_" << matchTag << "\n";
  }

  ss << instance.strResult(BRIDGE_FORMAT_TXT);

  return ss.str();
}


void strTriplet(
  const Board& board,
  const Instance& instance,
  const vector<unsigned>& relPlayers,
  const vector<vector<unsigned>>& params,
  const unsigned pno,
  const string& matchTag,
  const bool passFlag,
  const bool playerFlag,
  const string& playerTag)
{
  if (! passFlag && params[pno][0] < 10)
  {
    if (! playerFlag || 
      instance.strPlayer(static_cast<Player>(relPlayers[pno]),
        BRIDGE_FORMAT_TXT) == playerTag)
      cout << 
        strBidData(board, instance, relPlayers, params, pno, 0, matchTag);
  }
  else if (params[pno][0] >= 10 && params[pno][0] <= 12 &&
    isAboveOneLevel(instance.strCall(pno, BRIDGE_FORMAT_TXT)))
  {
    if (! playerFlag || 
      instance.strPlayer(static_cast<Player>(relPlayers[pno]),
        BRIDGE_FORMAT_TXT) == playerTag)
      cout << 
        strBidData(board, instance, relPlayers, params, pno, 1, matchTag);
  }
  else if (passFlag && params[pno][0] > 12)
  {
    if (! playerFlag || 
      instance.strPlayer(static_cast<Player>(relPlayers[pno]),
        BRIDGE_FORMAT_TXT) == playerTag)
      cout << 
        strBidData(board, instance, relPlayers, params, pno, 2, matchTag);
  }
}


void spadesControlKludge(
  const vector<unsigned>& relPlayers,
  vector<vector<unsigned>>& params,
  const vector<Valuation>& valuations,
  const unsigned pno)
{
  params[pno][0] = static_cast<unsigned>(
    valuations[relPlayers[pno]].handDist() >> 8);
  params[pno][2] = 10 + static_cast<unsigned>(
    valuations[relPlayers[pno]].getCompositeParam(VC_CONTROLS));
}


void updatePassStatistics(
  const Instance& instance,
  const vector<unsigned>& relPlayers,
  vector<vector<unsigned>>& params, // May kludge this
  const vector<Valuation>& valuations,
  const unsigned pno,
  const VulRelative vul,
  const bool passesFlag,
  const FilterParams& filterParams,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D)
{
  if (! filterParams.hcpFlag || params[pno][0] == filterParams.hcpValue)
  {
    // Kludge to get spades vs controls.
    if (filterParams.spadesControlFlag)
      spadesControlKludge(relPlayers, params, valuations, pno);

    if (! filterParams.playerFlag || 
      instance.strPlayer(static_cast<Player>(relPlayers[pno]),
        BRIDGE_FORMAT_TXT) == filterParams.playerTag)
    {
      paramStats1D.add(pno, vul, params[pno], passesFlag);
      paramStats2D.add(pno, vul, params[pno], passesFlag);
    }
  }
}


void passStats(
  const Group& group,
  const Options& options,
  ParamStats1D& paramStats1D,
  ParamStats2D& paramStats2D)
{
  FilterParams filterParams;
  filterParams.distFilterFlag = true;
  filterParams.hcpFlag = false;
  filterParams.hcpValue = 11;
  filterParams. spadesControlFlag = false;
  filterParams.playerFlag = false;
  filterParams.playerTag = "shein";

  paramStats1D.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
    LOCAL_NAMES, LOCAL_DATA);

  paramStats2D.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
    LOCAL_NAMES, LOCAL_DATA);

  const string matchTag = options.distMatcher.strSpec();

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

        VulRelative vulDealer, vulNonDealer;
        instance.getVulRelative(vulDealer, vulNonDealer);

        const vector<VulRelative> sequentialVuls =
          { vulDealer, vulNonDealer, vulDealer, vulNonDealer };

        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          bool seqPassFlag = instance.auctionStarts(sequentialPasses[pos]);

          if (! filterParams.distFilterFlag || 
            valuations[relPlayers[pos]].distMatch(options.distMatcher))
          {
            updatePassStatistics(instance, relPlayers, params, valuations,
              pos, sequentialVuls[pos], seqPassFlag, filterParams,
              paramStats1D, paramStats2D);

            strTriplet(board, instance, relPlayers, params,
              pos, matchTag, seqPassFlag, 
              filterParams.playerFlag, filterParams.playerTag);
          }

          if (! seqPassFlag)
            break;
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

