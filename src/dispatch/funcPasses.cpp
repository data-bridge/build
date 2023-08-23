/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

// This is an evil hack to share paramStats1D across two modes:
// One where HCP, CCCC etc. are tabulated and cross-referenced,
// and where the statistics are for each positition etc.
// And one where the overall probability of passing (from passing
// tables) is correlated against the binary outcome (pass/bid)
// on each hand.  So we get the contribution of each position to
// the passing result of the overall hand.

#include <iomanip>
#include <sstream>
#include <fstream>
#include <cassert>

#include "../control/Options.h"

#include "../records/Group.h"

#include "../analysis/Distribution.h"
#include "../analysis/Distributions.h"

#include "funcPasses.h"

#include "../util/parse.h"

#include "../stats/ParamStats1D.h"
#include "../stats/ParamStats2D.h"

#include "../include/bridge.h"
#include "../handling/Bexcept.h"

// TODO TMP
#include "../analysis/passes/PassTables.h"

static PassTables passTables;

// Effectively a subset of CompositeParams, but more compactly 
// numbered for memory efficiency.

// TODO Currently adding one at the end that is not really spades,
// but instead the table where we log the counts of each row (line)
// in the manually curated table of pass probabilities and criteria.

enum LocalParams
{
  PASS_HCP = 0,
  PASS_CCCC = 1,
  PASS_ZAR = 2,
  PASS_SPADES = 3,
  PASS_CONTROLS = 4,
  PASS_HCP_SHORTEST = 5,
  PASS_HCP_LONGEST = 6,
  PASS_HCP_LONG12 = 7,
  PASS_TABLE = 8,
  PASS_SIZE = 9
};

// The mapping between the two.

static vector<CompositeParams> LOCAL_TO_COMPOSITE =
{
  VC_HCP,
  VC_CCCC,
  VC_ZAR,
  VC_SPADES,
  VC_CONTROLS,
  VC_HCP_SHORTEST,
  VC_HCP_LONGEST,
  VC_HCP_LONG12,
  VC_SPADES // Not really
};

static vector<StatsInfo> LOCAL_DATA =
{
  { "HCP", 0, 40, 1},
  { "CCCC", 0, 1200, 20}, // TODO Check maxima
  { "ZP", 0, 80, 1}, // TODO Check maxima
  { "Spades", 0, 14, 1},
  { "Controls", 0, 13, 1},
  { "Short HCP", 0, 11, 1},
  { "Long HCP", 0, 11, 1},
  { "Long12 HCP", 0, 21, 1},
  { "Table", 0, 101, 1}
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
  // bool spadesControlFlag;

  // Only consider a specific player name?
  bool playerFlag;
  string playerTag;

  // Calculate 2D statistics?
  bool stats2DFlag;

  // Show hands that may be interesting or anomalous?
  bool handsFlag;
};


void setPassTables()
{
  passTables.read();
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
  const string& boardTag,
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

  vector<string> tokens;
  tokenize(boardTag, tokens, "|");
  assert(tokens.size() == 3);
  ss << "Board: " << tokens[1] << endl;

  ss << "HCP: " << params[pno][PASS_HCP] << endl;

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
  const string& boardTag,
  const unsigned pno,
  const unsigned& matchNumber,
  const bool passFlag,
  const FilterParams& filterParams)
{
  if (! filterParams.handsFlag)
    return;

  if (filterParams.playerFlag &&
    instance.strPlayer(static_cast<Player>(relPlayers[pno]),
      BRIDGE_FORMAT_TXT) != filterParams.playerTag)
    return;

  const string matchTag = DISTRIBUTION_NAMES[matchNumber];

  if (! passFlag && params[pno][PASS_HCP] < 10)
  {
    cout << strBidData(board, instance, relPlayers, params, 
        boardTag, pno, 0, matchTag);
  }
  else if (params[pno][PASS_HCP] >= 10 && params[pno][PASS_HCP] <= 12 &&
    isAboveOneLevel(instance.strCall(pno, BRIDGE_FORMAT_TXT)))
  {
    cout << strBidData(board, instance, relPlayers, params, 
      boardTag, pno, 1, matchTag);
  }
  else if (passFlag && params[pno][PASS_HCP] > 12)
  {
    cout << strBidData(board, instance, relPlayers, params, 
      boardTag, pno, 2, matchTag);
  }
}


void updatePassStatistics(
  const Instance& instance,
  const vector<unsigned>& relPlayers,
  const vector<vector<unsigned>>& params, 
  const unsigned pno,
  const unsigned distNo,
  const vector<VulRelative> vuls,
  const bool passesFlag,
  const FilterParams& filterParams,
  vector<ParamStats1D>& paramStats1D,
  vector<ParamStats2D>& paramStats2D)
{
  if (! filterParams.hcpFlag || 
      params[pno][PASS_HCP] == filterParams.hcpValue)
  {
    if (! filterParams.playerFlag || 
      instance.strPlayer(static_cast<Player>(relPlayers[pno]),
        BRIDGE_FORMAT_TXT) == filterParams.playerTag)
    {
      paramStats1D[distNo].add(pno, vuls[pno], params[pno], passesFlag);
      if (filterParams.stats2DFlag)
        paramStats2D[distNo].add(pno, vuls[pno], params[pno], passesFlag);
    }
  }
}


void passStats(
  const Group& group,
  const Options& options,
  vector<ParamStats1D>& paramStats1D,
  vector<ParamStats2D>& paramStats2D)
{
  FilterParams filterParams;
  filterParams.distFilterFlag = false;
  filterParams.hcpFlag = false;
  filterParams.hcpValue = options.distMatcher.getMaxSpades(); // Kludge
  filterParams.playerFlag = false;
  filterParams.playerTag = "shein";
  filterParams.stats2DFlag = false;
  filterParams.handsFlag = false;

  Distribution distribution;

  for (auto& p: paramStats1D)
    p.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
      LOCAL_NAMES, LOCAL_DATA);

  for (auto& p: paramStats2D)
    p.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
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

        const string boardTag = 
          instance.strRoom(bpair.extNo, BRIDGE_FORMAT_LIN);

        VulRelative vulDealer, vulNonDealer;
        instance.getVulRelative(vulDealer, vulNonDealer);

        const vector<VulRelative> sequentialVuls =
          { vulDealer, vulNonDealer, vulDealer, vulNonDealer };

        vector<unsigned> lengths;
        lengths.resize(BRIDGE_SUITS);
        float probCum = 1.;
        PassTableMatch passTableMatch;

        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          bool seqPassFlag = instance.auctionStarts(sequentialPasses[pos]);

          if (! filterParams.distFilterFlag || 
            valuations[relPlayers[pos]].distMatch(options.distMatcher))
          {
            valuations[relPlayers[pos]].getLengths(lengths);
            const unsigned distNo = distribution.number(lengths);

      /*
      cout << 
        DISTRIBUTION_NAMES[distNo] << "\n";
cout << "board " << boardTag << " pos " << pos << " vul " << 
  sequentialVuls[pos] << endl;
  */
            // TODO TMP Replace PASS_TABLE with the row number in the 
            // manual probability table.  Also log the four probabilities.
            // It is a bit wasteful to do this for every instance.
            passTableMatch = passTables.lookupFull(
              distNo, pos, sequentialVuls[pos], 
              valuations[relPlayers[pos]]);

            probCum *= passTableMatch.prob;
            params[pos][PASS_TABLE] = 
              static_cast<unsigned>(passTableMatch.rowNo);

            updatePassStatistics(instance, relPlayers, params,
              pos, distNo, sequentialVuls, seqPassFlag, filterParams,
              paramStats1D, paramStats2D);

            strTriplet(board, instance, relPlayers, params,
              boardTag, pos, distNo, seqPassFlag, filterParams);
          }

          if (! seqPassFlag)
            break;
        }
      }
    }
  }
}


void passStatsContrib(
  const Group& group,
  [[maybe_unused]] const Options& options,
  vector<ParamStats1D>& paramStats1D)
{
  Distribution distribution;

  for (auto& p: paramStats1D)
    p.init(BRIDGE_PLAYERS, BRIDGE_VUL_SIZE, PASS_SIZE,
      LOCAL_NAMES, LOCAL_DATA);

  const float binFactor = 100.f;
    // static_cast<float>(LOCAL_DATA[PASS_TABLE].factor);

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

        const string boardTag = 
          instance.strRoom(bpair.extNo, BRIDGE_FORMAT_LIN);

        VulRelative vulDealer, vulNonDealer;
        instance.getVulRelative(vulDealer, vulNonDealer);

        const vector<VulRelative> sequentialVuls =
          { vulDealer, vulNonDealer, vulDealer, vulNonDealer };

        vector<unsigned> lengths;
        lengths.resize(BRIDGE_SUITS);

        float probCum = 1.;
        PassTableMatch passTableMatch;

        const bool fourPassesFlag = 
          instance.auctionStarts(sequentialPasses[3]);

        vector<unsigned> distNumbers(BRIDGE_PLAYERS);
        vector<unsigned> rowNumbers(BRIDGE_PLAYERS);

        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          valuations[relPlayers[pos]].getLengths(lengths);
          distNumbers[pos] = distribution.number(lengths);

      /*
      cout << 
        DISTRIBUTION_NAMES[distNumbers[pos]] << "\n";
cout << "board " << boardTag << " pos " << pos << " vul " << 
  sequentialVuls[pos] << endl;
  if (boardTag == "qx|o23|" && pos == 3 &&
    sequentialVuls[pos] == 1)
    {
      cout << "HERE\n";
    }
  */
          passTableMatch = passTables.lookupFull(
            distNumbers[pos], pos, sequentialVuls[pos], 
            valuations[relPlayers[pos]]);

          probCum *= passTableMatch.prob;
          rowNumbers[pos] = 
            static_cast<unsigned>(passTableMatch.rowNo);

          // strTriplet(board, instance, relPlayers, params,
            // boardTag, pos, distNo, seqPassFlag, filterParams);
        }

        unsigned probBin;
        if (probCum == 0.)
          probBin = 0;
        else
        {
          probBin = static_cast<unsigned>(probCum * binFactor + 0.99999f);
          assert(probBin < LOCAL_DATA[PASS_TABLE].length);
        }

        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          paramStats1D[distNumbers[pos]].add(
            pos, sequentialVuls[pos], PASS_TABLE,
            // probBin, fourPassesFlag);
            rowNumbers[pos], fourPassesFlag);
        }
      }
    }
  }
}


void passPostprocess(vector<ParamStats1D>& paramStats1D)
{
  vector<float> rowProbs;

  for (unsigned tableIndex = 0; tableIndex < 16*DIST_SIZE; tableIndex++)
  {
    const unsigned distNo = tableIndex >> 4;
    const unsigned pos = (tableIndex >> 2) & 0x3;
    const unsigned vul = tableIndex & 0x3;
    passTables.getProbVector(distNo, pos, vul, rowProbs);

    cout << 
      DISTRIBUTION_NAMES[distNo] << " " << pos << " " << vul << " " <<
      paramStats1D[distNo].validateProbs(pos, vul, PASS_TABLE, rowProbs) <<
      "\n";
  }
}


void dispatchPasses(
  const Group& group,
  const Options& options,
  vector<ParamStats1D>& paramStats1D,
  [[maybe_unused]] vector<ParamStats2D>& paramStats2D,
  ostream& flog)
{
  try
  {
    // passStats(group, options, paramStats1D, paramStats2D);
    passStatsContrib(group, options, paramStats1D);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

