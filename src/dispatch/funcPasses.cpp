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
#include <regex>
#include <cassert>

#include "../control/Options.h"

#include "../records/Group.h"

#include "../analysis/Distribution.h"
#include "../analysis/Distributions.h"

#include "funcPasses.h"
#include "Opening.h"
#include "Openings.h"

#include "../util/parse.h"

#include "../stats/ParamStats1D.h"
#include "../stats/ParamStats2D.h"
#include "../stats/RuleStats.h"

#include "../include/bridge.h"
#include "../handling/Bexcept.h"

// TODO TMP
#include "../analysis/passes/PassTables.h"

static PassTables passTables;

static Opening opening;

// The mapping between CompositeParams and PassParams.

static vector<CompositeParams> LOCAL_TO_COMPOSITE =
{
  VC_HCP,
  VC_SPADES,
  VC_CCCC_LIGHT,
  VC_QUICK_TRICKS,
  VC_CONTROLS,
  VC_HCP_SHORTEST,
  VC_HCP_LONGEST,
  VC_HCP_LONG12
};

static vector<StatsInfo> LOCAL_DATA =
{
  { "HCP", 0, 40, 1},
  { "Spades", 0, 14, 1},
  { "CCCC light", 0, 160, 4}, // TODO Check maxima
  { "Qtricks", 0, 40, 2},
  { "Controls", 0, 13, 1},
  { "Short HCP", 0, 11, 1},
  { "Long HCP", 0, 11, 1},
  { "Long12 HCP", 0, 21, 1}
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


void setPassTables(vector<AllStats>& allStatsList)
{
  passTables.read(allStatsList);

  opening.init();
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
  else if (
    params[pno][PASS_HCP] >= 10 && 
    // params[pno][PASS_HCP] >= 10 && 
    // params[pno][PASS_HCP] <= 12 &&
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
        // float probCum = 1.;
        // PassTableMatch passTableMatch;

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
            /*
            passTableMatch = passTables.lookupFull(
              distNo, pos, sequentialVuls[pos], 
              valuations[relPlayers[pos]]);

            probCum *= passTableMatch.prob;
            params[pos][PASS_TABLE] = 
              static_cast<unsigned>(passTableMatch.rowNo);
              */

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


void passWrite(
  const Group& group,
  const string& fname)
{
  regex pattern(R"([\\/]([0-9]+)\.lin$)");
  smatch match;
  assert(regex_search(fname, match, pattern) && match.size() > 1);

  regex bpattern(R"(\|([^|]+)\|)");
  smatch bmatch;

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
        assert(regex_search(boardTag, bmatch, bpattern) && 
         bmatch.size() > 1);

        const string wholeTag = match[1].str() + "-" + bmatch[1].str();

        VulRelative vulDealer, vulNonDealer;
        instance.getVulRelative(vulDealer, vulNonDealer);

        const vector<VulRelative> sequentialVuls =
          { vulDealer, vulNonDealer, vulDealer, vulNonDealer };

        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          const bool cumPasses = 
            instance.auctionStarts(sequentialPasses[pos]);

          cout << 
            wholeTag << "," <<
            pos << "," <<
            sequentialVuls[pos] << "," <<
            (cumPasses ? 1 : 0) << "," <<
            valuations[relPlayers[pos]].strCorrData() << "\n";

          if (! cumPasses)
            break;
        }
      }
    }
  }
}


void passWriteOpenings(
  const Group& group,
  const string& fname)
{
  regex pattern(R"([\\/]([0-9]+)\.lin$)");
  smatch match;
  assert(regex_search(fname, match, pattern) && match.size() > 1);

  regex bpattern(R"(\|([^|]+)\|)");
  smatch bmatch;

  Openings op;

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
        assert(regex_search(boardTag, bmatch, bpattern) && 
         bmatch.size() > 1);

        const string wholeTag = match[1].str() + "-" + bmatch[1].str();

        VulRelative vulDealer, vulNonDealer;
        instance.getVulRelative(vulDealer, vulNonDealer);

        const vector<VulRelative> sequentialVuls =
          { vulDealer, vulNonDealer, vulDealer, vulNonDealer };

        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          const bool cumPasses = 
            instance.auctionStarts(sequentialPasses[pos]);

          const string call = instance.strCall(pos, BRIDGE_FORMAT_TXT);
          assert(call == "P" || call.size() > 1);

          if (cumPasses)
            op = OPENING_PASS;
          else if (call[0] == '1')
          {
            // Will be wrong for strong-pass systems.
            op = OPENING_NOT_WEAK;
          }
          else if (call == "2C")
          {
            op = opening.classify(call,
              valuations[relPlayers[pos]], params[pos]);

            if (op == OPENING_UNCLASSIFIED)
            {
              cout << "XXX " << params[pos][PASS_HCP] << "\n";

              FilterParams filterParams;
              filterParams.distFilterFlag = false;
              filterParams.hcpFlag = false;
              filterParams.hcpValue = params[pos][PASS_HCP];
              filterParams.playerFlag = false;
              filterParams.playerTag = "shein";
              filterParams.stats2DFlag = false;
              filterParams.handsFlag = true;

              strTriplet(board, instance, relPlayers, params,
                boardTag, pos, 0, cumPasses, filterParams);
            }
          }
          else if (call == "2D")
          {
            // TODO Not implemented yet
          }
          else
          {
            op = opening.classify(call,
              valuations[relPlayers[pos]], params[pos]);
          }

          cout << 
            wholeTag << "," <<
            pos << "," <<
            sequentialVuls[pos] << "," <<
            op << "," <<
            valuations[relPlayers[pos]].strCorrData() << "\n";

          if (! cumPasses)
            break;
        }
      }
    }
  }
}


void passStatsContrib(
  const Group& group,
  [[maybe_unused]] const Options& options,
  RuleStats& ruleStats)
{
  Distribution distribution;

  /*
  for (unsigned d = 0; d < DIST_SIZE; d++)
    for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
      for (unsigned vul = 0; vul < BRIDGE_VULS; vul++)
        ruleStats.init(d, pos, vul, passTables);
        */

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

// assert(valuations[relPlayers[pos]].getCompositeParam(VC_HCP) * 4 ==
 // valuations[relPlayers[pos]].getCompositeParam(VC_CCCC_LIGHT));

          passTableMatch = passTables.lookupFull(
            distNumbers[pos], pos, sequentialVuls[pos], 
            valuations[relPlayers[pos]]);

          probCum *= passTableMatch.prob;
          rowNumbers[pos] = static_cast<unsigned>(passTableMatch.rowNo);

      /*
cout << "board " << boardTag << " pos " << pos << " vul " << 
  sequentialVuls[pos] << " " << DISTRIBUTION_NAMES[distNumbers[pos]] << 
    ", prob " << passTableMatch.prob << ", probCum " << probCum << endl;

if (probCum < 0.)
{
  cout << valuations[relPlayers[pos]].str() << "\n";
}
  */
          // strTriplet(board, instance, relPlayers, params,
            // boardTag, pos, distNo, seqPassFlag, filterParams);
        }

        bool prevSeqPassFlag = true;
        for (unsigned pos = 0; pos < BRIDGE_PLAYERS; pos++)
        {
          if  (prevSeqPassFlag)
          {
            const bool seqPassFlag = 
              instance.auctionStarts(sequentialPasses[pos]);

/*
cout << "seqPassFlag " << seqPassFlag << 
" distNo " << distNumbers[pos] << " pos " << pos << " vul " <<
sequentialVuls[pos] << " row " << rowNumbers[pos] << endl;
*/
            ruleStats.addPosition(
              distNumbers[pos], pos, sequentialVuls[pos],
              rowNumbers[pos], seqPassFlag);

            prevSeqPassFlag = seqPassFlag;
          }

/*
cout << "adding Hand, " <<
" distNo " << distNumbers[pos] << " pos " << pos << " vul " <<
sequentialVuls[pos] << " row " << rowNumbers[pos] << endl;
*/
          assert(probCum >= 0.);

          ruleStats.addHand(
            distNumbers[pos], pos, sequentialVuls[pos],
            rowNumbers[pos], fourPassesFlag, probCum);
        }
      }
    }
  }
}


/*
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
*/


void dispatchPasses(
  const Group& group,
  [[maybe_unused]] const Options& options,
  const string& fname,
  [[maybe_unused]] vector<ParamStats1D>& paramStats1D,
  [[maybe_unused]] vector<ParamStats2D>& paramStats2D,
  [[maybe_unused]] RuleStats& ruleStats,
  ostream& flog)
{
  try
  {
    // passStats(group, options, paramStats1D, paramStats2D);
    // passWrite(group,  fname);
    passWriteOpenings(group,  fname);
    // passStatsContrib(group, options, ruleStats);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

