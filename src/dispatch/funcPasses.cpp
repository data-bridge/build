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

// Effectively a subset of CompositeParams, but more compactly 
// numbered for memory efficiency.

// TODO Currently adding one at the end that is not really spades,
// but instead the table where we log the counts of each row (line)
// in the manually curated table of pass probabilities and criteria.

enum LocalParams
{
  PASS_HCP = 0,
  PASS_SPADES = 1,
  PASS_CCCC_LIGHT = 2,
  PASS_QTRICKS = 3,
  PASS_CONTROLS = 4,
  PASS_HCP_SHORTEST = 5,
  PASS_HCP_LONGEST = 6,
  PASS_HCP_LONG12 = 7,
  PASS_SIZE = 8
};

// The mapping between the two.

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


Opening classifyTwoHearts(
  const Valuation& valuation,
  const vector<unsigned>& relPlayerParam)
{
  const unsigned clubs = valuation.getSuitParam(BRIDGE_CLUBS, VS_LENGTH);
  const unsigned diamonds = 
    valuation.getSuitParam(BRIDGE_DIAMONDS, VS_LENGTH);
  const unsigned hearts = valuation.getSuitParam(BRIDGE_HEARTS, VS_LENGTH);
  const unsigned spades = valuation.getSuitParam(BRIDGE_SPADES, VS_LENGTH);

  if (relPlayerParam[PASS_HCP] >= 16)
  {
    // Catches a few two-suiters as well.
    if (hearts >= 5)
      return OPENING_2H_STRONG_HEARTS;
    else if (spades >= 5 && hearts == 4 &&
        relPlayerParam[PASS_HCP] == 16)
      return OPENING_2H_INTERMED_MAJS;
    else if (spades >= 6)
      return OPENING_2H_STRONG_SPADES;
    else if (spades >= 2 && spades <= 5 &&
        hearts >= 2 && hearts <= 4 &&
        diamonds >= 2 && diamonds <= 6 &&
        clubs >= 2 && clubs <= 6)
      return OPENING_2H_STRONG_BAL;

    unsigned numSuits = 0;
    if (spades >= 4) numSuits++;
    if (hearts >= 4) numSuits++;
    if (diamonds >= 4) numSuits++;
    if (clubs >= 4) numSuits++;

    if (numSuits == 3)
      return OPENING_2H_STRONG_THREE_SUITER;
    else if (diamonds <= 1 && (clubs == 4 || clubs == 5) &&
      hearts <= 4 && spades <= 4)
    {
      // This assumes that it's really an intermediate hand.
      // In practice this happens very rarely anyway.  See below.
      // 4=4=1=4, 3=4=1=5, 4=3=1=5.
      return OPENING_2H_INTERMED_THREE_SUITER_SHORT_D;
    }
    else
      return OPENING_2H_STRONG_MISC;
  }

  if (relPlayerParam[PASS_HCP] <= 10)
  {
    // Catches a few two-suiters as well.
    if (hearts >= 6)
      return OPENING_2H_WEAK_HEARTS;
    else if (hearts == 5)
    {
      if (clubs >= 4 || diamonds >= 4)
        return OPENING_2H_WEAK_WITH_MIN;
      else if (spades >= 4)
        return OPENING_2H_WEAK_WITH_SPADES;
      else
        return OPENING_2H_WEAK_5332;
    }

    const unsigned prod = spades * hearts * diamonds * clubs;
    if (prod == 96 || prod == 108)
      // 4432, 4333.
      return OPENING_2H_WEAK_BAL;

    if (hearts == 4)
    {
      if (spades >= 4)
        return OPENING_2H_WEAK_WITH_SPADES;
      else if (clubs >= 5 || diamonds >= 5)
        return OPENING_2H_WEAK_45_MIN;
      // Might fall through.
    }
    else if (spades >= 6)
      return OPENING_2H_WEAK_SPADES;
    else if (spades == 5)
    {
      if (clubs >= 4 || diamonds >= 4)
        return OPENING_2H_WEAK_SPADES_MIN;
      else
        return OPENING_2H_WEAK_SPADES_5332;
    }
    else if (hearts <= 3 && clubs >= 4 && diamonds >= 4 &&
        clubs + diamonds >= 9)
      return OPENING_2H_WEAK_MINS;
    else if (clubs >= 7 || diamonds >= 7)
      return OPENING_2H_WEAK_MIN;

    // This covers 4 spades with 5+ of a minor, 1=4=4=4,
    // "4=4=1=4 +/- one card", and bluffs of both Majors,
    //  5 hearts with 4+ of a minor, etc.
    return OPENING_2H_WEAK_MISC;
  }

  if (hearts >= 6)
    return OPENING_2H_INTERMED_HEARTS;
  else if (spades >= 6)
    return OPENING_2H_INTERMED_HEARTS;
  else if (hearts == 5 && spades < 4)
    return OPENING_2H_INTERMED_HEARTS;
  else if (hearts >= 4 && spades >= 4)
    return OPENING_2H_INTERMED_MAJS;
  else if (diamonds <= 1 && (clubs == 4 || clubs == 5) &&
      hearts <= 4 && spades <= 4)
  {
    // 4=4=1=4, 3=4=1=5, 4=3=1=5.
    return OPENING_2H_INTERMED_THREE_SUITER_SHORT_D;
  }
  else if ((spades == 3 || spades == 4) && hearts <= 1 && 
      (diamonds == 4 || diamonds == 5) &&
      (clubs == 4 || clubs == 5))
    return OPENING_2H_INTERMED_THREE_SUITER_SHORT_H;
  else if (hearts == 4)
  {
    if (clubs >= 5 || diamonds >= 5)
      return OPENING_2H_INTERMED_45_MIN;
    else
      return OPENING_2H_INTERMED_MISC;
  }
  else if (spades == 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_2H_INTERMED_SPADES_MIN;
  else if (clubs >= 5 && diamonds >= 5)
    return OPENING_2H_INTERMED_MINS;
  else
    return OPENING_2H_INTERMED_MISC;
}


Opening classifyTwoSpades(
  const Valuation& valuation,
  const vector<unsigned>& relPlayerParam)
{
  const unsigned clubs = valuation.getSuitParam(BRIDGE_CLUBS, VS_LENGTH);
  const unsigned diamonds = 
    valuation.getSuitParam(BRIDGE_DIAMONDS, VS_LENGTH);
  const unsigned hearts = valuation.getSuitParam(BRIDGE_HEARTS, VS_LENGTH);
  const unsigned spades = valuation.getSuitParam(BRIDGE_SPADES, VS_LENGTH);

  if (relPlayerParam[PASS_HCP] >= 16)
  {
    if (spades >= 5)
      return OPENING_2S_STRONG_SPADES;
    else if (clubs >= 6)
      return OPENING_2S_STRONG_CLUBS;
    else if (diamonds >= 6)
      return OPENING_2S_STRONG_DIAMONDS;
    else if (hearts >= 6)
      return OPENING_2S_STRONG_HEARTS;

    unsigned numSuits = 0;
    if (spades == 4) numSuits++;
    if (hearts >= 4) numSuits++;
    if (diamonds >= 4) numSuits++;
    if (clubs >= 4) numSuits++;

    if (numSuits == 3)
      return OPENING_2S_STRONG_THREE_SUITER;
  }

  if (relPlayerParam[PASS_HCP] <= 10)
  {
    // Catches a few two-suiters as well.
    if (spades >= 6)
      return OPENING_2S_WEAK_SPADES;
    else if (spades == 5)
    {
      if (clubs >= 4 || diamonds >= 4)
        return OPENING_2S_WEAK_WITH_MIN;
      else if (hearts >= 4)
        return OPENING_2S_WEAK_WITH_HEARTS;
      else
        return OPENING_2S_WEAK_5332;
    }
    else if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
      return OPENING_2S_WEAK_MINS;
    else if (clubs >= 6 || diamonds >= 6)
      return OPENING_2S_WEAK_MINOR;
    else if (hearts >= 6)
      return OPENING_2S_WEAK_HEARTS;
    else if (spades == 4 && (clubs >= 5 || diamonds >= 5 || hearts >= 5))
      return OPENING_2S_WEAK_45;
    else if (spades == 4 && (clubs == 4 || diamonds == 4))
      return OPENING_2S_WEAK_44;
    else if (hearts == 5 && (clubs >= 5 || diamonds >= 5))
      return OPENING_2S_WEAK_HEARTS_MIN;
    else if (hearts == 5 && (spades >= 4 || diamonds >= 4 || clubs >= 4))
      return OPENING_2S_WEAK_HEARTS_OTHER;
    else
      return OPENING_UNCLASSIFIED;
  }

  if (spades >= 6)
    return OPENING_2S_INTERMED_SPADES;
  else if (spades == 5)
  {
    if (relPlayerParam[PASS_HCP] >= 11)
      return OPENING_2S_INTERMED_SPADES;
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (spades == 4)
  {
    if (clubs >= 5 || diamonds >= 5 || hearts >= 5)
      return OPENING_2S_INTERMED_45;

    unsigned numSuits = 1;
    if (hearts >= 4) numSuits++;
    if (diamonds >= 4) numSuits++;
    if (clubs >= 4) numSuits++;

    if (numSuits == 3)
      return OPENING_2S_INTERMED_THREE_SUITER;
    else
      return OPENING_UNCLASSIFIED;
  }
  else if (clubs >= 4 && diamonds >= 4 && clubs + diamonds >= 9)
    return OPENING_2S_INTERMED_MINS;
  else if (clubs >= 6 || diamonds >= 6)
    return OPENING_2S_INTERMED_MIN;
  else if (hearts == 5 && (clubs >= 5 || diamonds >= 5))
    return OPENING_2S_INTERMED_HEARTS_MIN;
  else if (spades <= 1 && hearts >= 3 && diamonds >= 3 && clubs >= 3)
    return OPENING_2S_INTERMED_SHORT_SPADES;
  else
    return OPENING_UNCLASSIFIED;
}


Opening classifyTwoNT(
  const Valuation& valuation,
  const vector<unsigned>& relPlayerParam)
{
  const unsigned longest1 = valuation.getDistParam(VD_L1);
  const unsigned longest2 = valuation.getDistParam(VD_L2);
  const unsigned longest4 = valuation.getDistParam(VD_L4);

  if (relPlayerParam[PASS_HCP] >= 15)
  {
    // Kind-of semi-balanced.
    if (longest1 <= 5 && longest2 <= 4 && longest4 >= 2)
      return OPENING_2NT_STRONG_SBAL;
    else
      return OPENING_2NT_STRONG_OTHER;
  }

  const unsigned clubs = 
    valuation.getSuitParam(BRIDGE_CLUBS, VS_LENGTH);
  const unsigned diamonds = 
    valuation.getSuitParam(BRIDGE_DIAMONDS, VS_LENGTH);

  if (relPlayerParam[PASS_HCP] <= 10)
  {

    if ((clubs >= 5 && diamonds >= 4) ||
        (clubs == 4 && diamonds >= 5))
      return OPENING_2NT_WEAK_MINS;
    else if (clubs >= 6 || diamonds >= 6)
      return OPENING_2NT_WEAK_ONE_MIN;
    else if (longest1 >= 5 && longest2 >= 5)
      return OPENING_2NT_WEAK_TWO_SUITER;
    else if (longest1 >= 7)
      return OPENING_2NT_WEAK_ONE_SUITER;
    else
      return OPENING_2NT_WEAK_OTHER;
  }

  if (clubs >= 6 || diamonds >= 6)
    return OPENING_2NT_OPEN_ONE_MIN;
  else if (longest1 >= 5 && longest2 >= 4)
    return OPENING_2NT_OPEN_TWO_SUITER;
  else
  return OPENING_2NT_OPEN_OTHER;

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

  Opening opening;

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
            opening = OPENING_PASS;
          else if (call[0] == '1')
          {
            // Will be wrong for strong-pass systems.
            opening = OPENING_NOT_WEAK;
          }
          else if (pos <= 1 && call == "2H")
          {
            opening = classifyTwoHearts(
              valuations[relPlayers[pos]], params[pos]);

            if (opening == OPENING_UNCLASSIFIED)
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
          else if (pos <= 1 && call == "2S")
          {
            opening = classifyTwoSpades(
              valuations[relPlayers[pos]], params[pos]);
          }
          else if (pos <= 2 && call == "2NT")
          {
            opening = classifyTwoNT(
              valuations[relPlayers[pos]], params[pos]);
          }
          else
            opening = OPENING_UNCLASSIFIED;

          cout << 
            wholeTag << "," <<
            pos << "," <<
            sequentialVuls[pos] << "," <<
            opening << "," <<
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

