/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Tableau.h"
#include "Debug.h"
#include "parse.h"

#include <vector>

extern Debug debug;


Tableau::Tableau()
{
  Tableau::Reset();
}


Tableau::~Tableau()
{
}


void Tableau::Reset()
{
  setNum = 0;
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      table[d][p] = BRIDGE_TRICKS+1;
}


bool Tableau::TableIsSet() const
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
    {
      LOG("Invalid number of tricks: '" + STR(c) + "'");
      return false;
    }

    listRBN[drbn] = (invertFlag ? 13-t : t);
  }
  return true;
}


bool Tableau::SetRBNPlayer(
  const unsigned listRBN[],
  const playerType p)
{
  for (unsigned drbn = 0; drbn < BRIDGE_DENOMS; drbn++)
  {
    if (! Tableau::SetEntry(p, DENOM_RBN_TO_DDS[drbn], listRBN[drbn]))
      return false;
  }
  return true;
}


bool Tableau::SetRBN(const string text)
{
  if (text == "")
  {
    LOG("Empty text");
    return false;
  }
    
  int c = count(text.begin(), text.end(), ':');
  if (c != 2 && c != 3)
  {
    LOG("Text without 2 or 3 colons: '" + text + "'");
    return false;
  }

  vector<string> tokens(4);
  tokens.clear();
  tokenize(text, tokens, ":");
  string ns = tokens[2];
  string ew = tokens[3];

  size_t nsl = ns.length();
  size_t ewl = ew.length();
  if (nsl != 6 && nsl != 11) 
  {
    LOG("NS text not of length 6 or 11: '" + ns + "'");
    return false;
  }
  if (ewl != 2 && ewl != 6 && ewl != 11)
  {
    LOG("EW text not of length 2, 6 or 11: '" + ew + "'");
    return false;
  }
  if (nsl == 6 && ns.at(5) != '=')
  {
    LOG("NS text without = at position 6: '" + ns + "'");
    return false;
  }
  if (nsl == 11 && ns.at(5) != '+')
  {
    LOG("NS text without + at position 6: '" + ns + "'");
    return false;
  }
  if (ewl == 11 && ew.at(5) != '+')
  {
    LOG("EW text without + at position 6: '" + ew + "'");
    return false;
  }

  unsigned listRBN[5];
  if (! Tableau::RBNStringToList(ns.substr(0, 5), listRBN))
    return false;
  if (! Tableau::SetRBNPlayer(listRBN, BRIDGE_NORTH))
    return false;

  string sub = (nsl == 6 ? ns.substr(0, 5) : ns.substr(6, 5));
  if (! Tableau::RBNStringToList(sub, listRBN))
    return false;
  if (! Tableau::SetRBNPlayer(listRBN, BRIDGE_SOUTH))
    return false;

  string e, w;
  if (ewl == 2)
  {
    w = ew.substr(0, 1);
    e = ew.substr(1, 1);
    if (w != "!")
    {
      LOG("W text not !");
      return false;
    }
    if (e != "!" && e != "=")
    {
      LOG("E text not ! nor =");
      return false;
    }
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
      {
        LOG("E text not ! nor =");
        return false;
      }
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
  if (! Tableau::SetRBNPlayer(listRBN, BRIDGE_WEST))
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
  
  return Tableau::SetRBNPlayer(listRBN, BRIDGE_EAST);
}


bool Tableau::SetEntry(
  const playerType p,
  const denomType d,
  const unsigned t)
{
  if (t == BRIDGE_TRICKS+1)
  {
    LOG("Too many tricks");
    return false;
  }
  else if (table[d][p] == BRIDGE_TRICKS+1)
  {
    setNum++;
    table[d][p] = t;
    return true;
  }
  else if (table[d][p] != t)
  {
    LOG("Already set to something different");
    return false;
  }
  else
    return true;
}


unsigned Tableau::GetEntry(
  const playerType p,
  const denomType d) const
{
  return table[d][p];
}


bool Tableau::operator ==(const Tableau& tab2) const
{
  if (setNum != tab2.setNum) return false;

  for (int p = 0; p < BRIDGE_PLAYERS; p++)
  {
    for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
    {
      if (table[d][p] == BRIDGE_TRICKS+1)
      {
        if (tab2.table[d][p] != BRIDGE_TRICKS+1)
	{
	  LOG("p " + STR(p) + ", d " + STR(d) + ": First unset, second set");
	  return false;
        }
      }
      else if (tab2.table[d][p] == BRIDGE_TRICKS+1)
      {
        LOG("p " + STR(p) + ", d " + STR(d) + ": First set, second unset");
        return false;
      }
      else if (table[d][p] != tab2.table[d][p])
      {
        LOG("p " + STR(p) + ", d " + STR(d) + ": Different values");
        return false;
      }
    }
  }

  return true;
}


bool Tableau::operator !=(const Tableau& tab2) const
{
  return ! (*this == tab2);
}


string Tableau::ToString(formatType f) const
{
  if (setNum != 20)
    return "";

  stringstream text("");

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      text << "op|" << Tableau::ToRBN() << "|\n";
      break;

    case BRIDGE_FORMAT_PBN:
      text << "[OptimumResultTable \"Declarer Denomination Result\"]\n";
      for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
      {
        for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
        {
          text << PLAYER_NAMES_SHORT[p] << " " <<
            DENOM_NAMES_SHORT_RBN[d] << " " <<
	    Tableau::GetEntry(static_cast<playerType>(p), 
	      DENOM_RBN_TO_DDS[d]) << "\n";
        }
      }
      break;

    case BRIDGE_FORMAT_RBN:
      text << "M " << Tableau::ToRBN() << "\n";
      break;

    case BRIDGE_FORMAT_TXT:
      text << "opt  S  N    E  W\n";
      for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
      {
        denomType drbn = DENOM_RBN_TO_DDS[d];
        text << setw(3) << DENOM_NAMES_SHORT_RBN[d] <<
	  setw(3) << Tableau::GetEntry(BRIDGE_SOUTH, drbn) <<
	  setw(3) << Tableau::GetEntry(BRIDGE_NORTH, drbn) <<
	  setw(5) << Tableau::GetEntry(BRIDGE_EAST, drbn) <<
	  setw(3) << Tableau::GetEntry(BRIDGE_WEST, drbn) << "\n";
      }
      text << "\n";
      break;
    
    default:
      break;
  }
  return text.str();
}


string Tableau::ToRBN() const
{
  unsigned n = Tableau::ToRBNPlayer(BRIDGE_NORTH);
  unsigned s = Tableau::ToRBNPlayer(BRIDGE_SOUTH);
  unsigned e = Tableau::ToRBNPlayer(BRIDGE_EAST);
  unsigned w = Tableau::ToRBNPlayer(BRIDGE_WEST);
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


unsigned Tableau::ToRBNPlayer(const playerType p) const
{
  unsigned r = 0;
  for (unsigned d = 0; d < BRIDGE_DENOMS; d++)
  {
    r <<= 4;
    r |= table[DENOM_RBN_TO_DDS[d]][p];
  }
  return r;
}


// --------------------------------------------------------------
// Par functions.
// Adapted from DDS (where I contributed it).
// --------------------------------------------------------------

// First index is contract number,
// 0 is pass, 1 is 1C, ..., 35 is 7NT.
// Second index is 0 nonvul, 1 vul.

const int SCORES[36][2] =
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

const int DOUBLED_SCORES[2][14] =
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

const unsigned DOWN_TARGET[36][4] =
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

const unsigned FLOOR_CONTRACT[36] =
{
   0,  1,  2,  3,  4,  5,  1,  2,  3,  4,  5,
       1,  2,  3,  4, 15,  1,  2, 18, 19, 15,
      21, 22, 18, 19, 15, 26, 27, 28, 29, 30,
      31, 32, 33, 34, 35
};

// First index is vul: none, both, NS, EW.
// Second index is vul (0, 1) for NS and then EW.

const unsigned VUL_TO_SIDE_VUL[BRIDGE_VUL_SIZE][2] =
{
  {0, 0}, {1, 1}, {1, 0}, {0, 1}
};

// First vul is declarer (not necessarily NS), second is defender.

const unsigned VUL_TO_NO[2][2] = 
{ 
  {0, 1}, {2, 3} 
};

// Maps DDS order (S, H, D, C, NT) to par order (C, D, H, S, NT).

const unsigned DENOM_PAR_TO_DDS[5] = 
{ 
  3, 2, 1, 0, 4 
};


#define BIGNUM 9999


bool Tableau::GetPar(
  const playerType dealer,
  const vulType v,
  string& parstr) const
{
  list<Contract> clist;
  if (! Tableau::GetPar(dealer, v, clist))
    return false;

  stringstream s;
  s.clear();
  bool first = true;
  for (list<Contract>::iterator it = clist.begin(); 
    it != clist.end(); it++)
  {
    if (first)
      first = false;
    else
      s << " ";

    s << it->AsString(BRIDGE_FORMAT_PAR);
  }
  s << "\n";

  parstr = s.str();
  return true;
}


bool Tableau::GetPar(
  const playerType dealer,
  const vulType v,
  list<Contract>& clist) const
{
  const unsigned * vulBySide = VUL_TO_SIDE_VUL[v];
  listType list[2][BRIDGE_DENOMS];
  dataType data;

  // First we find the side entitled to a plus score (primacy)
  // and some statistics for each constructively bid (undoubled)
  // contract that might be the par score.

  unsigned numCand;
  Tableau::SurveyScores(dealer, vulBySide, data, numCand, list);
  int side = data.primacy;

  if (side == -1)
  {
    Contract contract;
    contract.SetPassedOut();
    clist.clear();
    clist.push_back(contract);
    return true;
  }

  // Go through the contracts, starting from the highest one.
  listType * lists = list[side];
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

    Tableau::BestSacrifice(static_cast<unsigned>(side), no, dno, 
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
      listType *l = &(lists[n]);
      if (type[n] != 1 || l->score != bestPlus) 
        continue;
      unsigned plus;
      ReduceContract(l->no, sacGap[n], plus);
      if (! Tableau::AddContract(v, static_cast<unsigned>(side), l->no, 
          static_cast<denomType>(l->dno), static_cast<int>(plus), clist))
        return false;
    }
  }
  else
  {
    // The primacy side collects the penalty.
    for (unsigned n = 0; n < numCand; n++)
    {
      const listType *l = &(lists[n]);
      if (type[n] != 0 || lists[n].down != bestDown) 
        continue;
      if (! Tableau::AddSacrifices(v, static_cast<unsigned>(side), 
          dealer, static_cast<int>(bestDown), l->no, l->dno, 
	  list, sacr, clist))
        return false;
    }
  }
  return true;
}


void Tableau::SurveyScores(
  const playerType dealer,
  const unsigned * vulBySide,
  dataType& data,
  unsigned& numCand,
  listType list[][BRIDGE_DENOMS]) const
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

  dataType stats[2];

  for (unsigned side = 0; side <= 1; side++)
  {
    dataType * sside = &stats[side];
    sside->highestMakingNo = 0;
    sside->dearestMakingNo = 0;
    sside->dearestScore = 0;

    for (unsigned dno = 0; dno < BRIDGE_DENOMS; dno++)
    {
      listType * slist = &list[side][dno];
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

  const dataType * sside = &stats[primacy];

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

      listType temp = list[primacy][i - 1];
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


void Tableau::BestSacrifice(
  const unsigned side,
  const unsigned no,
  const unsigned dno,
  const playerType dealer,
  const listType slist[][BRIDGE_DENOMS],
  unsigned sacrTable[][BRIDGE_DENOMS],
  unsigned& bestDown) const
{
  const unsigned other = 1 - side;
  const listType * sacrList = slist[other];
  bestDown = BIGNUM;

  for (unsigned eno = 0; eno < BRIDGE_DENOMS; eno++)
  {
    listType sacr = sacrList[eno];
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


void Tableau::ReduceContract(
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


bool Tableau::AddSacrifices(
  const vulType v,
  const unsigned side,
  const playerType dealer,
  const int bestDown,
  const unsigned noDecl,
  const unsigned dno,
  const listType slist[][BRIDGE_DENOMS],
  const unsigned sacr[][BRIDGE_DENOMS],
  list<Contract>& clist) const
{
  const unsigned other = 1 - side;
  const listType * sacrList = slist[other];

  for (unsigned eno = 0; eno <= 4; eno++)
  {
    unsigned down = sacr[dno][eno];
    if (static_cast<int>(down) != bestDown) 
      continue;

    if (eno != dno)
    {
      unsigned noSac = sacrList[eno].no + 5 * bestDown;
      if (! Tableau::AddContract(v, other, noSac, 
          static_cast<denomType>(eno), -static_cast<int>(bestDown), 
	  clist))
	return false;
      continue;
    }

    // Special case: Sacrifice in declarer's best suit.
    const unsigned tMax = (noDecl + 34) / 5;
    const unsigned * t = table[ DENOM_PAR_TO_DDS[dno] ];
    unsigned incrFlag = 0;
    unsigned pHit = 0;
    playerType pnoList[2];
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
        pnoList[pHit] = static_cast<playerType>(pnoMod);
        sacList[pHit] = noDecl + 5 * incrFlag;
        pHit++;
      }
    }

    unsigned ns0 = sacList[0];
    if (pHit == 1)
    {
      if (! Tableau::AddSpecialSac(v, ns0, 
          static_cast<denomType>(eno), pnoList[0], bestDown, clist))
	return false;
      continue;
    }

    unsigned ns1 = sacList[1];
    if (ns0 == ns1)
    {
      // Both players.
      if (! Tableau::AddContract(v, other, ns0, 
          static_cast<denomType>(eno), -bestDown, clist))
        return false;
      continue;
    }

    int p = (ns0 < ns1 ? 0 : 1);
    if (! Tableau::AddSpecialSac(v, sacList[p], 
        static_cast<denomType>(eno), pnoList[p], bestDown, clist))
      return false;
  }
  return true;
}


bool Tableau::AddContract(
  const vulType v,
  const unsigned side,
  const unsigned no,
  const denomType dno,
  const int delta,
  list<Contract>& clist) const
{
  Contract contract;
  playerType declarer;
  const unsigned level = (no+4) / 5;
  const denomType denom = static_cast<denomType>(DENOM_PAR_TO_DDS[dno]);
  const multiplierType mult = (delta < 0 ? 
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

  if (! contract.SetContract(v, declarer, level, denom, mult))
    return false;

  if (! contract.SetTricks(tricks))
    return false;
  
  clist.push_back(contract);
  return true;
}


bool Tableau::AddSpecialSac(
  const vulType v,
  const unsigned no,
  const denomType denom,
  const playerType sacker,
  const int delta,
  list<Contract>& clist) const
{
  Contract contract;
  const unsigned level = (no-1) / 5;
  const multiplierType mult = BRIDGE_MULT_DOUBLED;

  if (! contract.SetContract(v, sacker, level, denom, mult))
    return false;

  if (! contract.SetTricks(level + delta))
    return false;
  
  clist.push_back(contract);
  return true;
}

