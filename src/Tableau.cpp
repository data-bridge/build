/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>

#include "Tableau.h"
#include "Contract.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"



Tableau::Tableau()
{
  Tableau::reset();
}


Tableau::~Tableau()
{
}


void Tableau::reset()
{
  setNum = 0;
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      table[d][p] = BRIDGE_TRICKS+1;
}


bool Tableau::isComplete() const
{
  return (setNum == 20);
}



bool Tableau::RBNStringToList(
  const string text,
  unsigned listRBN[],
  const bool invertFlag)
{
  unsigned t;
  for (unsigned drbn = 0; drbn < BRIDGE_DENOMS; drbn++)
  {
    char c = text.at(drbn);
    if (c >= '0' && c <= '9')
      t = static_cast<unsigned>(c - '0');
    else if (c >= 'A' && c <= 'D')
      t = static_cast<unsigned>(c - 'A' + 10);
    else
      THROW("Invalid number of tricks: '" + STR(c) + "'");

    listRBN[drbn] = (invertFlag ? 13-t : t);
  }
  return true;
}


bool Tableau::setRBNPlayer(
  const unsigned listRBN[],
  const Player p)
{
  for (unsigned drbn = 0; drbn < BRIDGE_DENOMS; drbn++)
  {
    if (! Tableau::set(p, DENOM_RBN_TO_DDS[drbn], listRBN[drbn]))
      return false;
  }
  return true;
}


bool Tableau::setRBN(const string& text)
{
  if (text == "")
    THROW("Empty text");
    
  int c = static_cast<int>(count(text.begin(), text.end(), ':'));
  if (c != 2 && c != 3)
    THROW("Text without 2 or 3 colons: '" + text + "'");

  vector<string> tokens(4);
  tokens.clear();
  tokenize(text, tokens, ":");
  string ns = tokens[2];
  string ew = tokens[3];

  size_t nsl = ns.length();
  size_t ewl = ew.length();

  if (nsl != 6 && nsl != 11) 
    THROW("NS text not of length 6 or 11: '" + ns + "'");

  if (ewl != 2 && ewl != 6 && ewl != 11)
    THROW("EW text not of length 2, 6 or 11: '" + ew + "'");
  if (nsl == 6 && ns.at(5) != '=')
    THROW("NS text without = at position 6: '" + ns + "'");

  if (nsl == 11 && ns.at(5) != '+')
    THROW("NS text without + at position 6: '" + ns + "'");

  if (ewl == 11 && ew.at(5) != '+')
    THROW("EW text without + at position 6: '" + ew + "'");

  unsigned listRBN[5];
  if (! Tableau::RBNStringToList(ns.substr(0, 5), listRBN))
    return false;
  if (! Tableau::setRBNPlayer(listRBN, BRIDGE_NORTH))
    return false;

  string sub = (nsl == 6 ? ns.substr(0, 5) : ns.substr(6, 5));
  if (! Tableau::RBNStringToList(sub, listRBN))
    return false;
  if (! Tableau::setRBNPlayer(listRBN, BRIDGE_SOUTH))
    return false;

  string e, w;
  if (ewl == 2)
  {
    w = ew.substr(0, 1);
    e = ew.substr(1, 1);
    if (w != "!")
      THROW("W text not !");

    if (e != "!" && e != "=")
      THROW("E text not ! nor =");
  }
  else if (ewl == 6)
  {
    if (ew.at(0) == '!')
    {
      w = "!";
      e = ew.substr(1, 5);
    }
    else
    {
      w = ew.substr(0, 5);
      e = ew.substr(5, 1);
      if (e != "!" && e != "=")
        THROW("E text not ! nor =");
    }
  }
  else
  {
    e = ew.substr(0, 5);
    w = ew.substr(6, 5);
  }

  if (w == "!")
  {
    if (! Tableau::RBNStringToList(ns.substr(0, 5), listRBN, true))
      return false;
  }
  else
  {
    if (! Tableau::RBNStringToList(w, listRBN))
      return false;
  }
  if (! Tableau::setRBNPlayer(listRBN, BRIDGE_WEST))
    return false;
  
  if (e == "!")
  {
    if (! Tableau::RBNStringToList(sub, listRBN, true))
      return false;
  }
  else if (e != "=")
  {
    if (! Tableau::RBNStringToList(e, listRBN))
      return false;
  }
  
  return Tableau::setRBNPlayer(listRBN, BRIDGE_EAST);
}


bool Tableau::set(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_RBN:
      return Tableau::setRBN(text);

    default:
      THROW("Tableau format not implemented:" + STR(format));
  }
}


bool Tableau::set(
  const Player player,
  const Denom denom,
  const unsigned tricks)
{
  if (tricks == BRIDGE_TRICKS+1)
    THROW("Too many tricks");

  if (table[denom][player] == BRIDGE_TRICKS+1)
  {
    setNum++;
    table[denom][player] = tricks;
    return true;
  }
  else if (table[denom][player] != tricks)
    THROW("Already set to something different");
  else
    return true;
}


unsigned Tableau::get(
  const Player player,
  const Denom denom) const
{
  return table[denom][player];
}


bool Tableau::operator == (const Tableau& tableau2) const
{
  if (setNum != tableau2.setNum) 
    DIFF("Different numbers");

  for (int p = 0; p < BRIDGE_PLAYERS; p++)
  {
    for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    {
      if (table[d][p] == BRIDGE_TRICKS+1)
      {
        if (tableau2.table[d][p] != BRIDGE_TRICKS+1)
	  DIFF("p " + STR(p) + ", d " + STR(d) + 
              ": First unset, second set");
      }
      else if (tableau2.table[d][p] == BRIDGE_TRICKS+1)
        DIFF("p " + STR(p) + ", d " + STR(d) + 
          ": First set, second unset");
      else if (table[d][p] != tableau2.table[d][p])
        DIFF("p " + STR(p) + ", d " + STR(d) + 
          ": Different values");
    }
  }

  return true;
}


bool Tableau::operator != (const Tableau& tableau2) const
{
  return ! (*this == tableau2);
}


string Tableau::strPBN() const
{
  stringstream ss;
  ss << "[OptimumResultTable \"Declarer Denomination Result\"]\n";
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    {
      ss << PLAYER_NAMES_SHORT[p] << " " <<
        DENOM_NAMES_SHORT_RBN[d] << " " <<
      Tableau::get(static_cast<Player>(p), DENOM_RBN_TO_DDS[d]) << "\n";
    }
  }
  return ss.str();
}


string Tableau::strTXT() const
{
  stringstream ss;
  ss << "opt  S  N    E  W\n";
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
  {
    Denom drbn = DENOM_RBN_TO_DDS[d];
    ss << setw(3) << DENOM_NAMES_SHORT_RBN[d] <<
      setw(3) << Tableau::get(BRIDGE_SOUTH, drbn) <<
      setw(3) << Tableau::get(BRIDGE_NORTH, drbn) <<
      setw(5) << Tableau::get(BRIDGE_EAST, drbn) <<
      setw(3) << Tableau::get(BRIDGE_WEST, drbn) << "\n";
  }
  ss << "\n";
  return ss.str();
}


string Tableau::strRBN() const
{
  unsigned n = Tableau::toRBNPlayer(BRIDGE_NORTH);
  unsigned s = Tableau::toRBNPlayer(BRIDGE_SOUTH);
  unsigned e = Tableau::toRBNPlayer(BRIDGE_EAST);
  unsigned w = Tableau::toRBNPlayer(BRIDGE_WEST);
  unsigned ninv = 0xddddd - n;
  unsigned sinv = 0xddddd - s;

  stringstream text;
  text << hex << uppercase;
  text << "::" << n;
  if (n == s)
    text << "-";
  else
    text << "+" << s;
  text << ":";
  if (ninv == w)
    text << "!";
  else
    text << w;
  if (ninv != w && sinv != e && w != e)
    text << "+";
  if (sinv == e)
    text << "!";
  else if (w == e)
    text << "=\n";
  else
    text << e << "\n";
  text << dec << nouppercase;

  return text.str();
}


unsigned Tableau::toRBNPlayer(const Player player) const
{
  unsigned r = 0;
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
  {
    r <<= 4;
    r |= table[DENOM_RBN_TO_DDS[d]][player];
  }
  return r;
}


string Tableau::str(const Format format) const
{
  if (! Tableau::isComplete())
    return "";

  stringstream ss("");

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return "op|" + Tableau::strRBN() + "|\n";

    case BRIDGE_FORMAT_PBN:
      return Tableau::strPBN();

    case BRIDGE_FORMAT_RBN:
      return "M " + Tableau::strRBN() + "\n";

    case BRIDGE_FORMAT_TXT:
      return Tableau::strTXT();
    
    default:
      THROW("Unknown format: " + STR(format));
  }
}


// --------------------------------------------------------------
// Par functions.
// Adapted from DDS (where I contributed them).
// Data structures could probably be cleaned up some.
// --------------------------------------------------------------

// First index is contract number,
// 0 is pass, 1 is 1C, ..., 35 is 7NT.
// Second index is 0 nonvul, 1 vul.

static const int SCORES[36][2] =
{
  {    0,   0},
  {   70,  70}, {  70,   70}, {  80,   80}, {  80,   80}, {  90,   90},
  {   90,  90}, {  90,   90}, { 110,  110}, { 110,  110}, { 120,  120},
  {  110, 110}, { 110,  110}, { 140,  140}, { 140,  140}, { 400,  600},
  {  130, 130}, { 130,  130}, { 420,  620}, { 420,  620}, { 430,  630},
  {  400, 600}, { 400,  600}, { 450,  650}, { 450,  650}, { 460,  660},
  { 920, 1370}, { 920, 1370}, { 980, 1430}, { 980, 1430}, { 990, 1440},
  {1440, 2140}, {1440, 2140}, {1510, 2210}, {1510, 2210}, {1520, 2220}
};

// First index: 0 nonvul, 1 vul. Second index: tricks down.

static const int DOUBLED_SCORES[2][14] =
{
  {
    0   ,  100,  300,  500,  800, 1100, 1400, 1700,
    2000, 2300, 2600, 2900, 3200, 3500
  },
  {
    0   ,  200,  500,  800, 1100, 1400, 1700, 2000,
    2300, 2600, 2900, 3200, 3500, 3800
  }
};

// Second index is contract number, 0 .. 35.
// First index is vul: none, only defender, only declarer, both.

static const unsigned DOWN_TARGET[36][4] =
{
  {0, 0, 0, 0},
  {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0},
  {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0},
  {1, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}, {1, 0, 1, 0}, {2, 1, 3, 2},
  {1, 0, 1, 0}, {1, 0, 1, 0}, {2, 1, 3, 2}, {2, 1, 3, 2}, {2, 1, 3, 2},
  {2, 1, 3, 2}, {2, 1, 3, 2}, {2, 1, 3, 2}, {2, 1, 3, 2}, {2, 1, 3, 2},
  {4, 3, 5, 4}, {4, 3, 5, 4}, {4, 3, 6, 5}, {4, 3, 6, 5}, {4, 3, 6, 5},
  {6, 5, 8, 7}, {6, 5, 8, 7}, {6, 5, 8, 7}, {6, 5, 8, 7}, {6, 5, 8, 7}
};

static const unsigned FLOOR_CONTRACT[36] =
{
   0,  1,  2,  3,  4,  5,  1,  2,  3,  4,  5,
       1,  2,  3,  4, 15,  1,  2, 18, 19, 15,
      21, 22, 18, 19, 15, 26, 27, 28, 29, 30,
      31, 32, 33, 34, 35
};

// First index is vul: none, both, NS, EW.
// Second index is vul (0, 1) for NS and then EW.

static const unsigned VUL_TO_SIDE_VUL[BRIDGE_VUL_SIZE][2] =
{
  {0, 0}, {1, 1}, {1, 0}, {0, 1}
};

// First vul is declarer (not necessarily NS), second is defender.

static const unsigned VUL_TO_NO[2][2] = 
{ 
  {0, 1}, {2, 3} 
};

// Maps DDS order (S, H, D, C, NT) to par order (C, D, H, S, NT).

static const unsigned DENOM_PAR_TO_DDS[5] = 
{ 
  3, 2, 1, 0, 4 
};


#define BIGNUM 9999


bool Tableau::getPar(
  const Player dealer,
  const Vul vul,
  string& text) const
{
  list<Contract> clist;
  if (! Tableau::getPar(dealer, vul, clist))
    return false;

  stringstream s;
  s.clear();
  bool first = true;
  for (auto &contract: clist)
  {
    if (first)
      first = false;
    else
      s << " ";

    s << contract.str(BRIDGE_FORMAT_PAR);
  }
  s << "\n";

  text = s.str();
  return true;
}


bool Tableau::getPar(
  const Player dealer,
  const Vul vul,
  list<Contract>& clist) const
{
  const unsigned * vulBySide = VUL_TO_SIDE_VUL[vul];
  ListTableau list[2][BRIDGE_DENOMS];
  DataTableau data;

  // First we find the side entitled to a plus score (primacy)
  // and some statistics for each constructively bid (undoubled)
  // contract that might be the par score.

  unsigned numCand;
  Tableau::surveyScores(dealer, vulBySide, data, numCand, list);
  int side = data.primacy;

  if (side == -1)
  {
    Contract contract;
    contract.passOut();
    clist.clear();
    clist.push_back(contract);
    return true;
  }

  // Go through the contracts, starting from the highest one.
  ListTableau * lists = list[side];
  unsigned vulNo = data.vulNo;
  int bestPlus = 0;
  unsigned down = 0;
  bool sacFound = false;

  int type[5], sacGap[5];
  unsigned bestDown = 0;
  unsigned sacr[5][5] = 
    { {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}
    };

  for (unsigned n = 0; n < numCand; n++)
  {
    unsigned no = lists[n].no;
    unsigned dno = lists[n].dno;
    unsigned target = DOWN_TARGET[no][vulNo];

    Tableau::bestSacrifice(static_cast<unsigned>(side), no, dno, 
      dealer, list, sacr, down);

    if (down <= target)
    {
      bestDown = Max(bestDown, down);
      if (sacFound)
      {
        // Declarer will never get a higher sacrifice by bidding
        // less, so we can stop looking for sacrifices. But it
        // can't be a worthwhile contract to bid, either.
        type[n] = -1;
      }
      else
      {
        sacFound = true;
        type[n] = 0;
        lists[n].down = down;
      }
    }
    else
    {
      bestPlus = Max(bestPlus, lists[n].score);
      type[n] = 1;
      sacGap[n] = static_cast<int>(target - down);
    }
  }

  unsigned vulDef = vulBySide[1 - side];
  int sac = DOUBLED_SCORES[vulDef][bestDown];

  if (! sacFound || bestPlus > sac)
  {
    // The primacy side bids.
    for (unsigned n = 0; n < numCand; n++)
    {
      ListTableau *l = &(lists[n]);
      if (type[n] != 1 || l->score != bestPlus) 
        continue;
      unsigned plus;
      reduceContract(l->no, sacGap[n], plus);
      if (! Tableau::addContract(vul, static_cast<unsigned>(side), l->no, 
          static_cast<Denom>(l->dno), static_cast<int>(plus), clist))
        return false;
    }
  }
  else
  {
    // The primacy side collects the penalty.
    for (unsigned n = 0; n < numCand; n++)
    {
      const ListTableau *l = &(lists[n]);
      if (type[n] != 0 || lists[n].down != bestDown) 
        continue;
      if (! Tableau::addSacrifices(vul, static_cast<unsigned>(side), 
          dealer, static_cast<int>(bestDown), l->no, l->dno, 
	  list, sacr, clist))
        return false;
    }
  }
  return true;
}


void Tableau::surveyScores(
  const Player dealer,
  const unsigned * vulBySide,
  DataTableau& data,
  unsigned& numCand,
  ListTableau list[][BRIDGE_DENOMS]) const
{
  /*
    When this is done, data has added the following entries:
    * primacy (0 or 1) is the side entitled to a plus score.
      If the deal should be passed out, it is -1, and nothing
      else is set.
    * highestMakingNo is a contract number (for that side)
    * dearestMakingNo is a contract number (for that side)
    * dearestScore is the best score if there is no sacrifice
    * vulNo is an index for a table, seen from the primacy

    list[side][dno] has added the following entries:
    * score
    * dno is the denomination number
    * no is a contract number
    * tricks is the number of tricks embedded in the contract
    For the primacy side, the list is sorted in descending
    order of the contract number (no).
  */

  DataTableau stats[2];

  for (unsigned side = 0; side <= 1; side++)
  {
    DataTableau * sside = &stats[side];
    sside->highestMakingNo = 0;
    sside->dearestMakingNo = 0;
    sside->dearestScore = 0;

    for (unsigned dno = 0; dno < BRIDGE_DENOMS; dno++)
    {
      ListTableau * slist = &list[side][dno];
      const unsigned * t = table[ DENOM_PAR_TO_DDS[dno] ];
      const unsigned best = Max(t[side], t[side+2]);

      if (best < 7)
      {
        slist->score = 0;
	slist->no = 0;
        continue;
      }

      slist->no = 5 * (best - 7) + dno + 1;
      slist->score = SCORES[slist->no][ vulBySide[side] ];
      slist->dno = dno;
      slist->tricks = best;

      if (slist->score > sside->dearestScore)
      {
        sside->dearestScore = slist->score;
        sside->dearestMakingNo = slist->no;
      }
      else if (slist->score == sside->dearestScore)
      {
        /* The lowest such, e.g. 3NT and 5C. */
        sside->dearestMakingNo = Min(sside->dearestMakingNo, slist->no);
      }

      sside->highestMakingNo = Max(sside->highestMakingNo, slist->no);
    }
  }

  unsigned primacy = 0;
  unsigned s0 = stats[0].highestMakingNo;
  unsigned s1 = stats[1].highestMakingNo;
  if (s0 > s1)
    primacy = 0;
  else if (s0 < s1)
    primacy = 1;
  else if (s0 == 0)
  {
    data.primacy = -1;
    return;
  }
  else
  {
    // Special case, depends who can bid it first.
    const unsigned dno = (s0 - 1) % 5;
    const unsigned tMax = list[0][dno].tricks;
    const unsigned * t = table[ DENOM_PAR_TO_DDS[dno] ];

    for (unsigned pno = dealer; 
        pno <= static_cast<unsigned>(dealer) + 3; pno++)
    {
      if (t[pno % 4] != tMax) 
        continue;
      primacy = pno % 2;
      break;
    }
  }

  const DataTableau * sside = &stats[primacy];

  const unsigned dmNo = sside->dearestMakingNo;
  data.primacy = static_cast<int>(primacy);
  data.highestMakingNo = sside->highestMakingNo;
  data.dearestMakingNo = dmNo;
  data.dearestScore = sside->dearestScore;

  unsigned vulPrimacy = vulBySide[primacy];
  unsigned vulOther = vulBySide[1 - primacy];
  data.vulNo = VUL_TO_NO[vulPrimacy][vulOther];

  // Sort the scores in descending order of contract number,
  // i.e. first by score and second by contract number in case
  // the score is the same. Primitive bubble sort...
  unsigned n = 5;
  do
  {
    unsigned nNew = 0;
    for (unsigned i = 1; i < n; i++)
    {
      if (list[primacy][i-1].no > list[primacy][i].no) 
        continue;

      ListTableau temp = list[primacy][i - 1];
      list[primacy][i - 1] = list[primacy][i];
      list[primacy][i] = temp;

      nNew = i;
    }
    n = nNew;
  }
  while (n > 0);

  numCand = BRIDGE_DENOMS;
  for (n = 0; n < BRIDGE_DENOMS; n++)
  {
    if (list[primacy][n].no < dmNo) 
      numCand--;
  }
}


void Tableau::bestSacrifice(
  const unsigned side,
  const unsigned no,
  const unsigned dno,
  const Player dealer,
  const ListTableau slist[][BRIDGE_DENOMS],
  unsigned sacrTable[][BRIDGE_DENOMS],
  unsigned& bestDown) const
{
  const unsigned other = 1 - side;
  const ListTableau * sacrList = slist[other];
  bestDown = BIGNUM;

  for (unsigned eno = 0; eno < BRIDGE_DENOMS; eno++)
  {
    ListTableau sacr = sacrList[eno];
    unsigned down = BIGNUM;

    if (eno == dno)
    {
      unsigned t_max = (no + 34) / 5;
      const unsigned * t = table[ DENOM_PAR_TO_DDS[dno] ];
      unsigned incrFlag = 0;
      for (unsigned pno = dealer; 
          pno <= static_cast<unsigned>(dealer) + 3; pno++)
      {
        unsigned diff = t_max - t[pno % 4];
        if (pno % 2 == side)
        {
          if (diff == 0) 
	    incrFlag = 1;
        }
        else
        {
          unsigned local = diff + incrFlag;
	  down = Min(down, local);
        }
      }
      if (sacr.no + 5 * down > 35) 
        down = BIGNUM;
    }
    else
    {
      down = (no - sacr.no + 4) / 5;
      if (sacr.no + 5 * down > 35) 
        down = BIGNUM;
    }
    sacrTable[dno][eno] = down;
    bestDown = Min(bestDown, down);
  }
}


void Tableau::reduceContract(
  unsigned& no,
  const int sacGap,
  unsigned& plus) const
{
  // Could be that we found 4C just making, but it would be
  // enough to bid 2C +2. But we don't want to bid so low that
  // we lose a game or slam bonus.

  if (sacGap >= -1)
  {
    // No scope to reduce.
    plus = 0;
    return;
  }

  // This is the lowest contract that we could reduce to.
  unsigned flr = FLOOR_CONTRACT[no];

  // As such, declarer could reduce the contract by down+1 levels
  // (where down is negative) and still the opponent's sacrifice
  // would not turn profitable. But for non-vulnerable partials,
  // this can go wrong: 1M+1 and 2M= both pay +90, but 3m*-2
  // is a bad sacrifice against 2M=, while 2m*-1 would be a good
  // sacrifice against 1M+1. */
  unsigned noSacLevel = no + 5 * static_cast<unsigned>(sacGap + 1);
  unsigned noNew = Max(noSacLevel, flr);
  plus = (no - noNew) / 5;
  no = noNew;
}


bool Tableau::addSacrifices(
  const Vul vul,
  const unsigned side,
  const Player dealer,
  const int bestDown,
  const unsigned noDecl,
  const unsigned dno,
  const ListTableau slist[][BRIDGE_DENOMS],
  const unsigned sacr[][BRIDGE_DENOMS],
  list<Contract>& clist) const
{
  const unsigned other = 1 - side;
  const ListTableau * sacrList = slist[other];

  for (unsigned eno = 0; eno <= 4; eno++)
  {
    unsigned down = sacr[dno][eno];
    if (static_cast<int>(down) != bestDown) 
      continue;

    if (eno != dno)
    {
      unsigned noSac = 
        sacrList[eno].no + 5 * static_cast<unsigned>(bestDown);
      if (! Tableau::addContract(vul, other, noSac, 
          static_cast<Denom>(eno), -static_cast<int>(bestDown), 
	  clist))
	return false;
      continue;
    }

    // Special case: Sacrifice in declarer's best suit.
    const unsigned tMax = (noDecl + 34) / 5;
    const unsigned * t = table[ DENOM_PAR_TO_DDS[dno] ];
    unsigned incrFlag = 0;
    unsigned pHit = 0;
    Player pnoList[2];
    unsigned sacList[2];
    for (unsigned pno = dealer; 
        pno <= static_cast<unsigned>(dealer) + 3; pno++)
    {
      unsigned pnoMod = pno % 4;
      unsigned diff = tMax - t[pnoMod];
      if (pno % 2 == side)
      {
        if (diff == 0) 
	  incrFlag = 1;
      }
      else
      {
        down = diff + incrFlag;
        if (static_cast<int>(down) != bestDown) 
	  continue;
        pnoList[pHit] = static_cast<Player>(pnoMod);
        sacList[pHit] = noDecl + 5 * incrFlag;
        pHit++;
      }
    }

    unsigned ns0 = sacList[0];
    if (pHit == 1)
    {
      if (! Tableau::addSpecialSac(vul, ns0, 
          static_cast<Denom>(eno), pnoList[0], bestDown, clist))
	return false;
      continue;
    }

    unsigned ns1 = sacList[1];
    if (ns0 == ns1)
    {
      // Both players.
      if (! Tableau::addContract(vul, other, ns0, 
          static_cast<Denom>(eno), -bestDown, clist))
        return false;
      continue;
    }

    int p = (ns0 < ns1 ? 0 : 1);
    if (! Tableau::addSpecialSac(vul, sacList[p], 
        static_cast<Denom>(eno), pnoList[p], bestDown, clist))
      return false;
  }
  return true;
}


bool Tableau::addContract(
  const Vul vul,
  const unsigned side,
  const unsigned no,
  const Denom dno,
  const int delta,
  list<Contract>& clist) const
{
  Contract contract;
  Player declarer;
  const unsigned level = (no+4) / 5;
  const Denom denom = static_cast<Denom>(DENOM_PAR_TO_DDS[dno]);
  const Multiplier mult = (delta < 0 ? 
    BRIDGE_MULT_DOUBLED : BRIDGE_MULT_UNDOUBLED);

  const unsigned ta = table[denom][side];
  const unsigned tb = table[denom][side + 2];
  const unsigned tricks = Max(ta, tb);

  if (ta == tb)
    declarer = (side == 0 ? BRIDGE_NORTH_SOUTH : BRIDGE_EAST_WEST);
  else if (ta > tb)
    declarer = (side == 0 ? BRIDGE_NORTH : BRIDGE_EAST);
  else
    declarer = (side == 0 ? BRIDGE_SOUTH : BRIDGE_WEST);

  // Will throw exception on failure
  contract.setContract(vul, declarer, level, denom, mult);

  // Will throw exception on failure
  contract.setTricks(tricks);
  
  clist.push_back(contract);
  return true;
}


bool Tableau::addSpecialSac(
  const Vul vul,
  const unsigned no,
  const Denom denom,
  const Player sacker,
  const int delta,
  list<Contract>& clist) const
{
  Contract contract;
  const unsigned level = (no-1) / 5;
  const Multiplier mult = BRIDGE_MULT_DOUBLED;

  // Will throw exception on failure
  contract.setContract(vul, sacker, level, denom, mult);

  // Will throw exception on failure
  contract.setTricks(
      static_cast<unsigned>(static_cast<int>(level) + delta));
  
  clist.push_back(contract);
  return true;
}

