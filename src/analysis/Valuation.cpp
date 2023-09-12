/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#include <map>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <mutex>
#include <cassert>

#include "Valuation.h"
#include "DistMatcher.h"

#include "../handling/Bexcept.h"

#define MAX_HOLDING 8191 // 2^13 - 1

static mutex mtx;
static bool setValuationTables = false;


static CardArray POWER = 
{
  4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1
};

static CardArray BITS =
{
  12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

#define VALUATION_NUM_DISTS 560

static array<SuitListArray, MAX_HOLDING+1> SUIT_LIST;
static array<DistListArray, VALUATION_NUM_DISTS> DIST_LIST;


struct SuitBundle
{
  string name;
  string text;
  int scale;
  int distance;
};

const vector<SuitBundle> SuitInfo =
{
  {"HCP", "HCP", 1, 20},
  {"AHCP", "Adjusted HCP", 1, 20},
  {"CCCC", "CCCC comp.", 20, 1}, // As already scaled by 20
  {"ZAR", "Zar comp.", 1, 10},
  {"FL", "FL points", 1, 20},

  {"CONTROLS", "Controls", 1, 60},
  {"PLAY_TRICKS", "Play tricks", 2, 40},
  {"QUICK_TRICKS", "Quick tricks", 2, 40},
  {"LOSERS", "Losers", 2, 20},

  {"TOPS1", "Aces", 1, 60},
  {"TOPS2", "AK's", 1, 60},
  {"TOPS3", "AKQ's", 1, 60},
  {"TOPS4", "AKQJ's", 1, 40},
  {"TOPS5", "AKQJT's", 1, 40},

  {"LENGTH", "Length", 1, 40},
  {"EFF_LENGTH", "Eff. length", 2, 20},

  {"SPOT_SUM", "Spot sum", 1, 1},
  {"SPOT_SUM3", "Top-3 spot sum", 1, 1}

};

struct DistBundle
{
  string name;
  string text;
  int distance;
};

const vector<DistBundle> DistInfo =
{
  {"MDIFF", "S minus H", 40},
  {"MABSDIFF", "Abs(S-H)", 40},
  {"MMAX", "Max(S,H)", 40},
  {"MMIN", "Min(S,H)", 40},
  {"mMAX", "Max(D,C)", 40},
  {"mMIN", "Min(D,C)", 40},
  {"MPROD", "S * H", 2},
  {"PROD", "S * H * D * C", 1},

  {"L1", "Max. length", 40},
  {"L2", "2nd length", 40},
  {"L3", "3rd length", 40},
  {"L4", "Min. length", 40},
  {"VD_VOID", "Void flag", 60},
  {"LONGEST1", "Longest suit", 60},
  {"LONGEST2", "2nd longest", 60},
  {"SHORTEST", "Shortest", 60},
  
  {"BAL", "Balanced", 1},
  {"SBAL", "Semi-balanced", 1},

  {"ZAR", "Zar", 10}
};

static map<int, unsigned> distCodeMap;

/*
struct CompBundle
{
  string name;
  string text;
  string textShort;
  int scale;
  int distance;
};

const vector<CompBundle> CompInfo =
{
  {"HCP", "HCP", "HCP", 1, 20},
  {"AHCP", "Adjusted HCP", "AHCP", 1, 20},
  {"CCCC", "CCCC points", "CCCC", 20, 1},
  {"ZAR", "Zar points", "ZP", 1, 10},
  {"FL", "FL points", "FL", 1, 20},

  {"CONTROLS", "Controls", "controls", 1, 60},
  {"PLAY_TRICKS", "Play tricks", "Playtricks", 2, 40},
  {"QUICK_TRICKS", "Quick tricks", "Quicktricks", 2, 40},
  {"LOSERS", "Losers", "Losers", 2, 20},

  {"OUTTOPS1", "Outside tops1", "Outtops1", 1, 60},
  {"OUTTOPS2", "Outside tops2", "Outtops2", 1, 60},
  {"OUTTOPS3", "Outside tops3", "Outtops3", 1, 60},
  {"OUTTOPS4", "Outside tops4", "Outtops4", 1, 40},
  {"OUTTOPS5", "Outside tops5", "Outtops5", 1, 40},

  {"BAL", "Balanced", "BAL", 1, 1},
  {"UNBAL", "Unbalanced", "UNBAL", 1, 1},
  {"SBAL", "Semi-balanced", "SBAL", 1, 1},
  {"UNSBAL", "Not semi-BAL", "notSBAL", 1, 1},

  {"EFF_MDIFF", "Eff. S-H", "EffSH", 2, 40},
  {"EFF_MABSDIFF", "Eff. abs(S-H)", "EffSHdiff", 2, 40},
  {"EFF_MMAX", "Eff. max(S,H)", "EffSHmax", 2, 40},
  {"EFF_MMIN", "Eff. min(S,H)", "EffSHmin", 2, 40},
  {"EFF_mMAX", "Eff. max(D,C)", "EffDCmax", 2, 40},
  {"EFF_mMIN", "Eff. min(D,C)", "EffDCmin", 2, 40},

  {"EFF_L1", "Eff. 1st len", "Eff1", 2, 40},
  {"EFF_L2", "Eff. 2nd len", "Eff2", 2, 40},
  {"EFF_L3", "Eff. 3rd len", "Eff3", 2, 40},
  {"EFF_L4", "Eff. 4th len", "Eff4", 2, 40},

  {"MCONC", "Major conc.", "Mconc", 1, 1},
  {"TWOCONC", "Top-2 conc.", "Top2conc", 1, 1},

  {"HCP_SHORTEST", "Short HCP", "ShortHCP", 1, 1},
  {"HCP_LONGEST", "Long HCP", "LongHCP", 1, 1},
  {"HCP_LONG12", "Long12 HCP", "Long12HCP", 1, 1},
  {"SPADES", "Spades", "spades", 1, 1}
};
*/


Valuation::Valuation()
{
  Valuation::reset();
  if (! setValuationTables)
  {
    mtx.lock();
    if (! setValuationTables)
      Valuation::setTables();
    setValuationTables = true;
    mtx.unlock();
  }
}


void Valuation::reset()
{
  setFlag = false;
  detailFlag = false;
}


void Valuation::setTables()
{
  Valuation::setSuitTables();
  Valuation::setDistTables();
}


void Valuation::setSuitTables()
{
  CardArray cards;

  for (unsigned holding = 0; holding <= MAX_HOLDING; holding++)
  {
    // Also SPOT_SUM and SPOT_SUM3.
    Valuation::setSuitLength(SUIT_LIST[holding], cards, holding);

    // Also AHCP.
    Valuation::setSuitHCP(SUIT_LIST[holding], cards);

    // Also sets controls.
    Valuation::setSuitTops(SUIT_LIST[holding], cards);

    // Needs tops and controls.
    Valuation::setSuitCCCC(SUIT_LIST[holding], cards);

    Valuation::setSuitZar(SUIT_LIST[holding], cards);
    Valuation::setSuitFL(SUIT_LIST[holding]);

    Valuation::setSuitPlayTricks(SUIT_LIST[holding], cards);
    Valuation::setSuitQuickTricks(SUIT_LIST[holding], cards);
    Valuation::setSuitLosers(SUIT_LIST[holding], cards);


    // Uses a number of other entries, so comes late in the order.
    Valuation::setSuitEffLength(SUIT_LIST[holding]);
  }
}


struct SuitPair
{
  Denom denom;
  int len;
};

static bool ComparePair(const SuitPair& sp1, const SuitPair& sp2)
{
  return (sp1.len > sp2.len);
}


void Valuation::setDistTables()
{
  unsigned no = 0;
  for (int s = 0; s <= BRIDGE_TRICKS; s++)
  {
    for (int h = 0; h <= BRIDGE_TRICKS-s; h++)
    {
      for (int d = 0; d <= BRIDGE_TRICKS-s-h; d++)
      {
        const int c = BRIDGE_TRICKS-s-h-d;

        const int distCode = (s << 12) | (h << 8) | (d << 4) | c;
        DistListArray& dlist = DIST_LIST[no];
        distCodeMap[distCode] = no++;

        array<SuitPair, BRIDGE_SUITS> v;
        v[0] = {BRIDGE_SPADES, s};
        v[1] = {BRIDGE_HEARTS, h};
        v[2] = {BRIDGE_DIAMONDS, d};
        v[3] = {BRIDGE_CLUBS, c};
        stable_sort(v.begin(), v.end(), ComparePair);

        dlist[VD_MDIFF] = s-h;
        dlist[VD_MABSDIFF] = (s >= h ? s-h : h-s);
        dlist[VD_MMAX] = (s >= h ? s : h);
        dlist[VD_MMIN] = (s < h ? s : h);
        dlist[VD_mMAX] = (d >= c ? d : c);
        dlist[VD_mMIN] = (d < c ? d : c);
        dlist[VD_MPROD] = s*h;
        dlist[VD_PROD] = s*h*d*c;

        dlist[VD_L1] = v[0].len;
        dlist[VD_L2] = v[1].len;
        dlist[VD_L3] = v[2].len;
        dlist[VD_L4] = v[3].len;
        dlist[VD_VOID] = (v[3].len == 0);
        dlist[VD_LONGEST1] = static_cast<int>(v[0].denom);
        dlist[VD_LONGEST2] = static_cast<int>(v[1].denom);
        dlist[VD_SHORTEST] = static_cast<int>(v[3].denom);

        // Don't set BAL and SBAL yet.

        dlist[VD_ZAR] = 2*v[0].len + v[1].len - v[3].len;
        if (v[3].len == 3)
          dlist[VD_ZAR]++; // 4-3-3-3
      }
    }
  }
  
  if (no != VALUATION_NUM_DISTS)
    THROW("Bad number of distributions");
  
  // There is a wonderful formula for calculating directly the
  // index in dlist without a map (not implemented here):
  // https://math.stackexchange.com/questions/2320636/turn-a-restricted-composition-or-partition-into-a-unique-index
}


void Valuation::setSuitLength(
  SuitListArray& list,
  CardArray& cards,
  const unsigned holding)
{
  list[VS_LENGTH] = 0;
  list[VS_SPOT_SUM] = 0;
  list[VS_SPOT_SUM3] = 0;

  for (unsigned bit = 0; bit < BRIDGE_TRICKS; bit++)
  {
    // cards[0] is 1 if we hold the ace, and so on.
    cards[bit] = (static_cast<int>(holding) & POWER[bit]) >> BITS[bit];
    list[VS_LENGTH] += cards[bit];

    if (cards[bit])
    {
      list[VS_SPOT_SUM] += BRIDGE_TRICKS + 1 - bit;
      if (list[VS_LENGTH] <= 3)
        list[VS_SPOT_SUM3] += BRIDGE_TRICKS + 1 - bit;
    }
  }
}


void Valuation::setSuitHCP(
  SuitListArray& list,
  const CardArray& cards)
{
  list[VS_HCP] = 0;

  // 4-3-2-1.
  if (cards[0]) list[VS_HCP] += 4;
  if (cards[1]) list[VS_HCP] += 3;
  if (cards[2]) list[VS_HCP] += 2;
  if (cards[3]) list[VS_HCP] += 1;

  list[VS_AHCP] = list[VS_HCP];
  if (list[VS_LENGTH] == 1)
  {
    // 4-2-1-0 for stiff honor.
    if (cards[1]) list[VS_AHCP]--;
    if (cards[2]) list[VS_AHCP]--;
    if (cards[3]) list[VS_AHCP]--;
  }
}


void Valuation::setSuitTops(
  SuitListArray& list,
  const CardArray& cards)
{
  list[VS_TOP1] = cards[0];
  list[VS_TOP2] = list[VS_TOP1] + cards[1];
  list[VS_TOP3] = list[VS_TOP2] + cards[2];
  list[VS_TOP4] = list[VS_TOP3] + cards[3];
  list[VS_TOP5] = list[VS_TOP4] + cards[4];

  list[VS_CONTROLS] = list[VS_TOP1] + list[VS_TOP2];
}


void Valuation::setSuitCCCC(
  SuitListArray& list,
  const CardArray& cards)
{
  // CCCC by Kaplan and Rubens, Bridge World, Oct. 1983.
  // Summary: www-personal.umich.edu/~mjg/bridge/knrtxt.html.
  // Online evaluator for comparison:
  // www.jeff-goldsmith.org/cgi-bin/knr.cgi
  //
  // Here multiplied by 20 in order always to have an integer.
  // (CCCC points are multiples of 0.05.)
  //
  // There are two later, global adjustments that must be made when
  // knowing the exact distribution:  4-3-3-3 gets -0.5, and the
  // first doubleton doesn't count (so no +1).  This is done when
  // evaluating a specific hand.
  
  list[VS_CCCC] = 0;

  // 3-2-1 points to start with.
  list[VS_CCCC] += 60*cards[0] + 40*cards[1] + 20*cards[2];

  // 4-3-2-1 count * length / 10;
  int adder = 2 * list[VS_HCP];

  if (list[VS_LENGTH] > 8)
  {
    // +2 for no Q, +1 for no J, +0.5 for having the T.
    if (! cards[2]) adder += 4;
    if (! cards[3]) adder += 2;
    if (cards[4]) adder++;
  }
  else if (list[VS_LENGTH] == 8)
  {
    // +2 for no Q, +1 for Q without J, +0.5 for having the T.
    if (cards[2])
      adder += (cards[3] ? 0 : 2);
    else
      adder += 4;
    if (cards[4]) adder++;
  }
  else if (list[VS_LENGTH] == 7)
  {
    // +0.5 for lacking Q or J, +0.5 for having the T.
    if (! cards[2] || ! cards[3]) adder++;
    if (cards[4]) adder++;
  }
  else
  {
    // +1, plus +1 for the T with two+ higher or with the J.
    if (cards[4])
      adder += ((list[VS_TOP3] >= 2 || cards[3]) ? 2 : 1);

    // +0.5 for the 9 with two+ higher, the T or the 8.
    if (cards[5] && (list[VS_TOP3] >= 2 || cards[4] || cards[6]))
      adder++;

    // 4-6 cards with no eight or ten, and exactly three higher.
    if (cards[5] && ! cards[4] && ! cards[6] && list[VS_TOP4] == 3)
      adder++;
  }

  list[VS_CCCC] += adder * list[VS_LENGTH];

  // +3 for void, +2 for singleton, +1 (in principle) for doubleton.
  if (list[VS_LENGTH] < 3)
    list[VS_CCCC] += 20 * (3-list[VS_LENGTH]);
  
  // Stiff king only counts 0.5, not 2.
  if (cards[1] && list[VS_LENGTH] == 1)
    list[VS_CCCC] -= 30;
  
  if (cards[2])
  {
    if (list[VS_LENGTH] >= 3)
    {
      if (list[VS_TOP2] == 0) 
      // Qxx counts 0.75, not 1.
        list[VS_CCCC] -= 5;
    }
    else if (list[VS_LENGTH] == 1)
      // Stiff Q doesn't count at all.
      list[VS_CCCC] -= 20;
    else
      // AQ or KQ counts 0.5, Qx 0.25.
      list[VS_CCCC] -= (list[VS_TOP2] ? 10 : 15);
  }

  if (cards[3])
    // J with two higher honors counts 0.5, with one, 0.25.
    list[VS_CCCC] += (list[VS_TOP3] == 2 ? 10 :
      (list[VS_TOP3] == 1 ? 5 : 0));
  
  if (cards[4])
    // T with one higher as well as the 9 counts 0.25.
    list[VS_CCCC] += (list[VS_TOP4] == 2 ? 5 :
      (list[VS_TOP4] == 1 && cards[5] ? 5 : 0));
}


void Valuation::setSuitZar(
  SuitListArray& list,
  const CardArray& cards)
{
  // For stiff honors, AHCP gets scored as 4-2-1-0.
  // We want to score ZP as 5-2-1-0, so we add one for the stiff ace.
  if (list[VS_LENGTH] == 1)
    list[VS_ZAR] = list[VS_AHCP] + cards[0];
  else
    list[VS_ZAR] = list[VS_HCP] + list[VS_CONTROLS];
}


void Valuation::setSuitFL(SuitListArray& list)
{
  list[VS_FL] = list[VS_HCP];

  // Add length points for good, long suits.
  if (list[VS_HCP] >= 3 && list[VS_LENGTH] >= 5)
    list[VS_FL] += list[VS_LENGTH]-4;
}


void Valuation::setSuitPlayTricks(
  SuitListArray& list,
  const CardArray& cards)
{
  // Several definitions are known.  Here we go with Pavlicek,
  // http://www.rpbridge.net/8j17.htm#3.  Scaled up by 2x.
  if (list[VS_LENGTH] == 0)
    list[VS_PLAY_TRICKS] = 0;
  else if (list[VS_LENGTH] == 1)
  {
    if (cards[0])
      list[VS_PLAY_TRICKS] = 2;
    else if (cards[1])
      list[VS_PLAY_TRICKS] = 1;
    else
      list[VS_PLAY_TRICKS] = 0;
  }
  else if (list[VS_LENGTH] == 2)
  {
    if (list[VS_TOP2] == 2)
      list[VS_PLAY_TRICKS] = 4; // AK
    else if (cards[0])
      list[VS_PLAY_TRICKS] = (list[VS_TOP4] == 2 ? 3 : 2); // AQ, AJ, Ax
    else if (cards[1])
      list[VS_PLAY_TRICKS] = (list[VS_TOP4] == 2 ? 2 : 1); // KQ, KJ, Kx
    else
      list[VS_PLAY_TRICKS] = (cards[2] ? 1 : 0); // Qx, xx

  }
  else if (list[VS_LENGTH] >= 11)
    list[VS_PLAY_TRICKS] = 2 * (list[VS_TOP1] + (list[VS_LENGTH]-1));
  else if (list[VS_LENGTH] >= 9)
    list[VS_PLAY_TRICKS] = 2 * (list[VS_TOP2] + (list[VS_LENGTH]-2));
  else if (list[VS_TOP3] == 3)
    list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH]; // AKQ
  else if (list[VS_TOP4] == 3)
  {
    if (cards[0])
      list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 1; // AKJ, AQJ
    else
      list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 2; // KQJ
  }
  else if (list[VS_TOP2] == 2)
    list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 2; // AKx
  else if (list[VS_TOP5] <= 1)
  {
    if (list[VS_TOP2] == 1)
      list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 4; // Axx, Kxx
    else if (cards[2])
      list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 5; // Qxx
    else
      list[VS_PLAY_TRICKS] = 2 * (list[VS_LENGTH] - 3); // Less
  }
  else if (list[VS_TOP2] == 0)
  {
    if (list[VS_TOP4] == 2)
      list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 4; // QJx
    else
      list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 5; // QTx, JTx
  }
  else if (cards[0] && cards[2] && cards[4])
    list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 2; // AQT
  else
    list[VS_PLAY_TRICKS] = 2 * list[VS_LENGTH] - 3; // KJT, KQx, AQx, AJx

  if (list[VS_LENGTH] == 8 && (list[VS_TOP4] != 2 || list[VS_TOP2] != 0))
  {
    // Length 8, not QJx.
    list[VS_PLAY_TRICKS]++;
  }
}


void Valuation::setSuitQuickTricks(
  SuitListArray& list,
  const CardArray& cards)
{
  // Scaled by 2. Defensive tricks.

  if (list[VS_LENGTH] >= 9)
    list[VS_QUICK_TRICKS] = 0; // Nothing will cash
  else if (list[VS_LENGTH] >= 7)
    list[VS_QUICK_TRICKS] = (cards[0] ? 2 : 0); // Only ace may cash
  else if (cards[0])
    // AK = 2.0, AQ = 1.5, Ax = 1.0
    list[VS_QUICK_TRICKS] = (cards[1] ? 4 : (cards[2] ? 3 : 2));
  else if (cards[1])
    // KQ = 1.0, Kx = 0.5
    list[VS_QUICK_TRICKS] = (cards[2] ? 2 : 1);
  else
    list[VS_QUICK_TRICKS] = 0;
}


void Valuation::setSuitLosers(
  SuitListArray& list,
  const CardArray& cards)
{
  // Scaled by 2.
  if (list[VS_LENGTH] == 0)
    list[VS_LOSERS] = 0;
  else if (list[VS_LENGTH] == 1)
    // 1.0 except if we have the A
    list[VS_LOSERS] = (cards[0] ? 0 : 2);
  else if (list[VS_LENGTH] == 2)
  {
    if (cards[0])
      // AK is 0, AQ is 0.5, Ax is 1.
      list[VS_LOSERS] = (cards[1] ? 0 : (cards[2] ? 1 : 2));
    else if (cards[1])
      // KQ is 1, Kx is 1.5.
      list[VS_LOSERS] = (cards[2] ? 2: 3);
    else
      // Qx or worse is 2.
      list[VS_LOSERS] = 4;
  }
  else
  {
    // Each missing top card is a loser.
    list[VS_LOSERS] = 2 * (3-list[VS_TOP3]);
    
    if (cards[0] && ! cards[1] && ! cards[2] && cards[3] && cards[4])
      // AJT is only 1.
      list[VS_LOSERS] -= 2;
    else if (! cards[0] && cards[1] && ! cards[2] && cards[3] && cards[4])
      // KJT is only 1.5.
      list[VS_LOSERS]--;
    else if (! cards[0] && ! cards[1] && cards[2] && ! cards[3])
      // Qxx is 3.
      list[VS_LOSERS] += 2;
  }
}


void Valuation::setSuitEffLength(SuitListArray& list)
{
  // This is an attempt to quantify the "real" length of a suit.
  // An average suit keeps its length, but lengths can also be
  // judged up or down.  Everything is scaled up by 2 to get integers.
  
  list[VS_EFF_LENGTH] = 2 * list[VS_LENGTH];

  switch (list[VS_LENGTH])
  {
    case 1:
      if (list[VS_CONTROLS])
        // Count a singleton A/K as a 2-card suit.
        list[VS_EFF_LENGTH] += 2;
        break;

    case 4:
      if (list[VS_SPOT_SUM] >= 37)
        list[VS_EFF_LENGTH] += 2; // Good enough for 1L overcall?
      else if (list[VS_HCP] == 0)
        list[VS_EFF_LENGTH] -= 2; // More like a 3c suit
      break;

    case 5:
      if (list[VS_SPOT_SUM] >= 43)
        list[VS_EFF_LENGTH] += 2; // Good enough for general weak two?
      else if (list[VS_SPOT_SUM] >= 38)
        list[VS_EFF_LENGTH]++; // Good enough for 3rd hand weak two?
      else if (list[VS_SPOT_SUM] < 32)
        list[VS_EFF_LENGTH] -= 2; // More like a 4c suit
      break;

    case 6:
      if (list[VS_SPOT_SUM3] >= 33)
        list[VS_EFF_LENGTH] += 2; // Good enough for bad 3L preempt?
      else if (list[VS_SPOT_SUM3] < 30)
        list[VS_EFF_LENGTH] -= 2; // More like a 5c suit
      break;

    case 7:
      if (list[VS_SPOT_SUM3] <= 30)
        list[VS_EFF_LENGTH] -= 2; // More like a 6c suit
      else if (list[VS_TOP3] >= 2)
        list[VS_EFF_LENGTH]++; // Top-heavy 7c suit
      break;
  }
}


void Valuation::lookup(const unsigned cards[BRIDGE_SUITS])
{
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    suitValues[s] = &SUIT_LIST[cards[s] >> 2];

  const int distKey = 
    ((*suitValues[0])[VS_LENGTH] << 12) |
    ((*suitValues[1])[VS_LENGTH] << 8) |
    ((*suitValues[2])[VS_LENGTH] << 4) |
     (*suitValues[3])[VS_LENGTH];

  auto it = distCodeMap.find(distKey);
  if (it == distCodeMap.end())
    THROW("Could not find distKey\n");
  distValues = &DIST_LIST[it->second];
}


void Valuation::calcDetails()
{
  array<SuitPair, BRIDGE_SUITS> v;
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
  {
    v[s].denom = static_cast<Denom>(s);
    v[s].len = (*suitValues[s])[VS_EFF_LENGTH];
  }

  compValues[VC_EFF_MDIFF] = v[0].len - v[1].len;
  compValues[VC_EFF_MABSDIFF] = (v[0].len >= v[1].len ? 
    v[0].len-v[1].len : v[1].len-v[0].len);
  compValues[VC_EFF_MMAX] = (v[0].len >= v[1].len ? v[0].len : v[1].len);
  compValues[VC_EFF_MMIN] = (v[0].len < v[1].len ? v[0].len : v[1].len);
  compValues[VC_EFF_mMAX] = (v[2].len >= v[3].len ? v[2].len : v[3].len);
  compValues[VC_EFF_mMIN] = (v[2].len < v[3].len ? v[2].len : v[3].len);

  stable_sort(v.begin(), v.end(), ComparePair);

  compValues[VC_EFF_L1] = v[0].len;
  compValues[VC_EFF_L2] = v[1].len;
  compValues[VC_EFF_L3] = v[2].len;
  compValues[VC_EFF_L4] = v[3].len;

  // Doesn't really have to be a separate composite parameter,
  // but for some things this is easier.
  compValues[VC_SPADES] = (*suitValues[BRIDGE_SPADES])[VS_LENGTH];

  const unsigned longest1 = 
    static_cast<unsigned>((*distValues)[VD_LONGEST1]);
  if (compValues[VC_HCP] == 0)
  {
    compValues[VC_MCONC] = 0;
    compValues[VC_TWOCONC] = 0;

    compValues[VC_HCP_SHORTEST] = 0;
    compValues[VC_HCP_LONGEST] = 0;
    compValues[VC_HCP_LONG12] = 0;
  }
  else
  {
    const unsigned longest2 = 
      static_cast<unsigned>((*distValues)[VD_LONGEST2]);

    compValues[VC_MCONC] = static_cast<int>(100. * 
      ((*suitValues[BRIDGE_SPADES])[VS_HCP] + 
       (*suitValues[BRIDGE_HEARTS])[VS_HCP]) / compValues[VC_HCP]);

    compValues[VC_TWOCONC] = static_cast<int>(100. * 
      ((*suitValues[longest1])[VS_HCP] + 
       (*suitValues[longest2])[VS_HCP]) / compValues[VC_HCP]);

    compValues[VC_HCP_LONGEST] = (*suitValues[longest1])[VS_HCP];
    compValues[VC_HCP_LONG12] = (*suitValues[longest1])[VS_HCP] +
      (*suitValues[longest2])[VS_HCP];
    
    // Find the shortest non-void suit.  If there are two such,
    // this parameter is not meaningful.
    int shortNonzero = numeric_limits<int>::max();
    int shortHCP = numeric_limits<int>::max();
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    {
      const int sl = (*suitValues[s])[VS_LENGTH];
      if (sl > 0 && sl < shortNonzero)
      {
        shortNonzero = sl;
        shortHCP = (*suitValues[s])[VS_HCP];
      }
    }
    assert(shortHCP < 40);
    compValues[VC_HCP_SHORTEST] = shortHCP;
  }

  for (unsigned p = VS_TOP1; p <= VS_TOP5; p++)
  {
    compValues[p] = 0;
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    {
      if (s != longest1)
        compValues[p] += (*suitValues[s])[p];
    }
  }
}


void Valuation::setCompBalanced()
{
  const int prod = (*distValues)[VD_PROD];
  const int pen = DistInfo[VD_PROD].distance;

  // TODO: Probably better to look at the card distance (minimum
  // number of low cards changed from one suit to another), not
  // the "pen" product distance per "product point".
  // So 2=3=7=1 would be 1 card away (move a diamond to a club).
  // Singletons are OK in some cases, but the card difference
  // may also count.

  if (prod >= 90)
  {
    // Balanced, hence also semi-balanced.
    // UNSBAL distance could be considered larger, but I don't know
    // how to quantify this.
    compValues[VC_BAL] = 0;
    compValues[VC_SBAL] = 0;
    compValues[VC_UNBAL] = (prod - 90) * pen;
    compValues[VC_UNSBAL] = compValues[VC_UNSBAL];
    return;
  }

  // Unbalanced, could be semi-balanced or not.
  compValues[VC_BAL] = (90 - prod) * pen;
  compValues[VC_UNBAL] = 0;

  if (prod == 60 || prod == 64)
  {
    // 5-4-3-1, 4-4-4-1.
    if (compValues[VC_EFF_L4] == 4)
    {
      // Stiff A/K.
      if ((*distValues)[VD_MPROD] == 20)
      {
        // (5-4)=(3-1): SBAL except for one card distance.
        compValues[VC_SBAL] = SuitInfo[VS_LENGTH].distance;
        compValues[VC_UNSBAL] = 0;
      }
      else
      {
        // Distance to a Q.
        compValues[VC_SBAL] = 0;
        compValues[VC_UNSBAL] = CompInfo[VC_HCP].distance *
          ((*suitValues[ static_cast<unsigned>((*distValues)[VD_SHORTEST]) ])
            [VS_HCP]-2);
      }
    }
    else
    {
      // SBAL except for distance to a K.
      compValues[VC_SBAL] = CompInfo[VC_HCP].distance *
        (3-(*suitValues[ static_cast<unsigned>((*distValues)[VD_SHORTEST]) ])
          [VS_HCP]);
      compValues[VC_UNSBAL] = 0;

      // (5-4)=(3-1): Add one card distance.
      if ((*distValues)[VD_MPROD] == 20)
        compValues[VC_SBAL] += SuitInfo[VS_LENGTH].distance;
    }
  }
  else if ((prod == 72 || prod == 80) &&
    (*distValues)[VD_MMAX] >= 3 && (*distValues)[VD_MMAX] <= 5 &&
    (*distValues)[VD_MPROD] < 18)
  {
    // (5-2)=(4-2), (4-2)=(5-2), (3-2)=(6-2) are SBAL.
    compValues[VC_SBAL] = 0;
    compValues[VC_UNSBAL] = SuitInfo[VS_LENGTH].distance;
  }
  else
  {
    compValues[VC_SBAL] = (60 - prod) * pen;
    if (compValues[VC_SBAL] > 100)
      compValues[VC_SBAL] = 100;
    compValues[VC_UNSBAL] = 0;
  }
}


void Valuation::evaluate(
  const unsigned cards[BRIDGE_SUITS],
  const bool fullFlag)
{
  if (setFlag)
    THROW("Deal already set");
  setFlag = true;

  Valuation::lookup(cards);

  for (unsigned p = VS_HCP; p <= VS_TOP5; p++)
  {
    // Abuse of eval numbering!
    compValues[p] = 0;
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      compValues[p] += (*suitValues[s])[p];
  }

  compValues[VC_ZAR] += (*distValues)[VD_ZAR];

  Valuation::setCompBalanced();

  if (fullFlag)
  {
    detailFlag = true;
    Valuation::calcDetails();
  }
}


int Valuation::distance(const Term& term) const
{
  // Returns a positive value if condition is not met (the "distance"
  // to meeting the condition), a negative value if it is (the "distance"
  // to meeting it).

  if (! setFlag)
    THROW("Deal not set");

  const TermCategory cat = term.category();
  const unsigned paramNo = term.paramNo();
  int param, penalty;
  
  switch (cat)
  {
    case TERM_SUIT:
      param = (*suitValues[term.suit()])[paramNo];
      penalty = SuitInfo[paramNo].distance;
      break;

    case TERM_DIST:
      param = (*distValues)[paramNo];
      penalty = DistInfo[paramNo].distance;
      break;

    case TERM_COMP:
      if (paramNo >= VC_EFF_MDIFF && ! detailFlag)
        THROW("Details not set");
      param = compValues[paramNo];
      penalty = 1;
      break;

    default:
      THROW("Bad category");
  }

  const TermComparator comp = term.comparator();
  int limit1, limit2;

  switch (comp)
  {
    case COMPARATOR_LE:
      return penalty * (param - term.limit());

    case COMPARATOR_LT:
      return penalty * (param + 1 - term.limit());

    case COMPARATOR_EQ:
      if (param > term.limit())
        return penalty * (param - term.limit());
      else
        return penalty * (term.limit() - param);

    case COMPARATOR_GT:
      return penalty * (term.limit() - param + 1);

    case COMPARATOR_GE:
      return penalty * (term.limit() - param);

    case COMPARATOR_IN:
      limit1 = term.limit1();
      limit2 = term.limit2();
      if (param <= limit1)
        return penalty * (limit1 - param);
      else if (param >= limit2)
        return penalty * (param - limit2);
      else
      {
        const int d1 = param - limit1;
        const int d2 = limit2 - param;
        if (d1 >= d2)
          return - penalty * d2;
        else
          return - penalty * d1;
      }

    case COMPARATOR_NE:
      if (param > term.limit())
        return penalty * (term.limit() - param);
      else
        return penalty * (param - term.limit());

    default:
      THROW("Bad comparator");
  }
}


unsigned Valuation::getCompositeParam(const CompositeParams cparam) const
{
  assert(cparam < VC_SIZE);
  return static_cast<unsigned>(compValues[cparam]);
}


string Valuation::strName(const CompositeParams cparam) const
{
  assert(cparam < VC_SIZE);
  return CompInfo[cparam].text;
}


string Valuation::strHeader(const string& text) const
{
  stringstream ss;
  ss << text << ":\n\n";
  ss << setw(20) << left << "" << right;
  for (unsigned s = 0; s < BRIDGE_SUITS; s++)
    ss << setw(6) << DENOM_NAMES_SHORT_PBN[s];
  return ss.str() + "\n";
}


string Valuation::strEntry(
  const int value,
  const int scale) const
{
  stringstream ss;
  if (scale == 1)
    ss << setw(6) << right << value;
  else
    ss << setw(6) << right << fixed << setprecision(2) << 
      value / static_cast<double>(scale);
  return ss.str();
}


string Valuation::str() const
{
  if (! setFlag)
    return "";

  stringstream ss;
  ss << "\n" << Valuation::strHeader("Single suit parameters");

  for (unsigned p = 0; p < VS_SIZE; p++)
  {
    ss << setw(20) << left << SuitInfo[p].text;
    for (unsigned s = 0; s < BRIDGE_SUITS; s++)
      ss << Valuation::strEntry((*suitValues[s])[p], SuitInfo[p].scale);
    ss << "\n";
  }
  ss << "\n";

  ss << "Distribution parameters:\n\n";
  for (unsigned p = 0; p < VD_SIZE; p++)
  {
    ss << setw(20) << left << DistInfo[p].text;
    ss << Valuation::strEntry((*distValues)[p], 1) << "\n";
  }
  ss << "\n";

  ss << "Overall parameters\n\n";
  const unsigned upper = (detailFlag ? VC_SIZE : VC_EFF_MDIFF);
  for (unsigned p = 0; p < upper; p++)
  {
    ss << setw(20) << left << CompInfo[p].text;
    ss << Valuation::strEntry(compValues[p], CompInfo[p].scale) << "\n";
  }
  ss << "\n";

  return ss.str();
}


int Valuation::handDist() const
{
  // This is a 12-bit entry with 3 groups of 4 bits.
  // Each group is the number of cards held in that suit.
  // Clubs are neglected, as they follow from the three others.

  if (! setFlag)
    return 0;

  return ((*suitValues[0])[VS_LENGTH] << 8) |
      ((*suitValues[1])[VS_LENGTH] << 4) |
       (*suitValues[2])[VS_LENGTH];
}


void Valuation::getLengths(vector<unsigned>& lengths) const
{
  lengths = { 
    static_cast<unsigned>((*suitValues[0])[VS_LENGTH]),
    static_cast<unsigned>((*suitValues[1])[VS_LENGTH]),
    static_cast<unsigned>((*suitValues[2])[VS_LENGTH]),
    static_cast<unsigned>((*suitValues[3])[VS_LENGTH]) };
}


bool Valuation::distMatch(const DistMatcher& distMatcher) const
{
  return distMatcher.match(
    (*suitValues[0])[VS_LENGTH],
    (*suitValues[1])[VS_LENGTH],
    (*suitValues[2])[VS_LENGTH],
    (*suitValues[3])[VS_LENGTH],
    (*distValues)[VD_L1],
    (*distValues)[VD_L2],
    (*distValues)[VD_L3],
    (*distValues)[VD_L4]);
}

