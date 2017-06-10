/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>

#include "bconst.h"
#include "Board.h"
#include "Valuation.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


Board::Board()
{
  len = 0;
  numActive = 0;
}


Board::~Board()
{
}


void Board::reset()
{
  len = 0;
  numActive = 0;

  deal.reset();
  tableau.reset();
  instances.clear();
  skip.clear();

  basicsFlag = false;
  givenScore = 0.0f;
  givenSet = false;
  LINset = false;
  LINScoreSet = false;
}


Instance * Board::acquireInstance(const unsigned instNo)
{
  if (instNo >= len)
  {
    const unsigned lenOld = len;
    len = instNo+1;

    instances.resize(len);
    skip.resize(len);

    for (unsigned i = lenOld; i < len; i++)
      skip[i] = true;

    instances[0].setRoom("Open", BRIDGE_FORMAT_PBN);
    if (instNo == 1)
      instances[1].setRoom("Closed", BRIDGE_FORMAT_PBN);
  }

  numActive = instNo;

  // If we ever have len > 2, we would find the lowest non-skipped
  // instance and copy that to all skipped instances.  Here we take
  // the shortcut.

  if (len == 2 && deal.isSet())
  {
    if (skip[0] && ! skip[1] && ! basicsFlag)
    {
      basicsFlag = true;

      instances[0].copyDealerVul(instances[1]);

      // TODO: Make cleaner
      if (deal.isSet())
      {
        unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
        deal.getDDS(cards);
        instances[0].setPlayerDeal(cards);
      }
    }
    else if (skip[1] && ! skip[0])
    {
      basicsFlag = true;

      instances[1].copyDealerVul(instances[0]);

      // TODO: Make cleaner
      if (deal.isSet())
      {
        unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
        deal.getDDS(cards);
        instances[1].setPlayerDeal(cards);
      }
    }
  }

  return &instances[numActive];
}


void Board::setInstance(const unsigned no)
{
  if (len == 0 || no > len-1)
    THROW("Invalid instance selected");

  numActive = no;
}


const Instance& Board::getInstance(const unsigned instNo) const
{
  if (len == 0 || instNo > len-1)
    THROW("Invalid instance selected");

  return instances[instNo];
}


bool Board::skipped() const
{
  return skip[numActive];
}


bool Board::skipped(const unsigned no) const
{
  return (no >= len || skip[no]);
}


void Board::markUsed(const unsigned instNo)
{
  skip[instNo] = false;
}


bool Board::skippedAll() const
{
  for (unsigned l = 0; l < len; l++)
  {
    if (! skip[l])
      return false;
  }
  return true;
}


unsigned Board::count() const
{
  unsigned c = 0;
  for (unsigned l = 0; l < len; l++)
  {
    if (! skip[l])
      c++;
  }
  return c;
}


unsigned Board::countAll() const
{
  return len;
}


void Board::setLINheader(const LINData& lin)
{
  if (LINset)
    return;

  LINdata = lin;
  LINset = true;
  LINScoreSet = true;

  if (LINdata.data[0].mp != "")
    Board::setScoreIMP(LINdata.data[0].mp, BRIDGE_FORMAT_LIN);
  else if (LINdata.data[1].mp != "")
    Board::setScoreIMP("-" + LINdata.data[1].mp, BRIDGE_FORMAT_LIN);
  else
    LINScoreSet = false;
  
  for (unsigned i = 0; i < len; i++)
  {
    numActive = i;

    string st = "";
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      st += LINdata.data[i].players[(p+2) % 4];
      if (i < 3)
        st += ",";
    }

    if (st != ",,,")
      instances[i].setPlayers(st, BRIDGE_FORMAT_LIN, false);
    
    instances[i].setLINheader(LINdata.data[i]);
  }
}


void Board::setDealer(
  const string& text,
  const Format format)
{
  instances[numActive].setDealer(text, format);
}


void Board::setVul(
  const string& text,
  const Format format)
{
  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_TXT)
  {
    for (unsigned i = 0; i < len; i++)
      instances[i].setVul(text, format);
  }
  else
    instances[numActive].setVul(text, format);
}


// Deal

void Board::setDeal(
  const string& text,
  const Format format)
{
  // Assume the cards are unchanged.  Don't check for now.
  if (! deal.isSet())
    deal.set(text, format);

  if (! instances[numActive].dealIsSet())
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);

    instances[numActive].setPlayerDeal(cards);
  }

  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    // Fill up, just in case of a skip.
    const string d = text.substr(0, 1);
    for (unsigned i = 0; i < len; i++)
      instances[i].setDealer(d, format);
  }
}


Player Board::holdsCard(const string& text) const
{
  return deal.holdsCard(text);
}


// Contract

void Board::setScoreIMP(
  const string& text,
  const Format format)
{
  // We regenerate this ourselves, so mostly ignore for now.
  if (format == BRIDGE_FORMAT_LIN)
  {
    if (text == "--")
      givenScore = 0.0f;
    else if (! str2float(text, givenScore))
      THROW("Bad IMP score: " + text);

    givenSet = true;
  }
  else if (format == BRIDGE_FORMAT_PBN && numActive == 0)
  {
    if (text == "0")
    {
      givenScore = 0.f;
      givenSet = true;
      return;
    }

    if (text.length() < 4)
      THROW("Short RBN IMP score: " + text);
    int side = 0;
    if (text.substr(0, 3) == "NS ")
      side = 1;
    else if (text.substr(0, 3) == "EW ")
      side = -1;
    else
      THROW("RBN IMP score not NS/EW: " + text);

    // Double Dummy Captain has also used commas.
    string fixed = text.substr(3);
    size_t p = fixed.find(",");
    if (p < string::npos)
      fixed.at(p) = '.';

    if (! str2float(fixed, givenScore))
      THROW("Bad RBN IMP score: " + fixed);

    givenScore *= side;
    givenSet = true;
  }
}


void Board::setScoreMP(
  const string& text,
  const Format format)
{
  if (format == BRIDGE_FORMAT_PBN)
  {
    // We mostly ignore this for now.
  }
  else
  {
    if (! str2float(text, givenScore))
      THROW("Bad matchpoint score");

    givenSet = true;
  }
}


void Board::calculateScore()
{
  for (unsigned i = 0; i < len; i++)
    instances[i].calculateScore();
}


// Tableau

void Board::setTableau(
  const string& text,
  const Format format)
{
  tableau.set(text, format);
}


bool Board::setTableauEntry(
  const Player player,
  const Denom denom,
  const unsigned tricks)
{
  return tableau.set(player, denom, tricks);
}


unsigned Board::getTableauEntry(
  const Player player,
  const Denom denom) const
{
  return tableau.get(player, denom);
}


bool Board::getPar(
  Player dealer,
  Vul vul,
  string& text) const
{
  return tableau.getPar(dealer, vul, text);
}


bool Board::getPar(
  Player dealer,
  Vul vul,
  list<Contract>& text) const
{
  return tableau.getPar(dealer, vul, text);
}


// Players

void Board::copyPlayers(const Board& board2)
{
  for (unsigned i = 0; i < board2.len; i++)
    instances[i].copyPlayers(board2.instances[i]);
}


bool Board::overlappingPlayers() const
{
  if (len < 2)
    return false;
  else
    return instances[0].overlappingPlayers(instances[1]);
}


void Board::setRoom(
  const string& text,
  const Format format)
{
  instances[numActive].setRoom(text, format);
}


bool Board::getValuation(Valuation& valuation) const
{
  // TODO
  UNUSED(valuation);
  THROW("Valuation not implemented yet");
  return true;
}


bool Board::operator == (const Board& board2) const
{
  if (len != board2.len)
    DIFF("Different number of instances");

  bool wholeSkip = Board::skippedAll();
  bool wholeSkip2 = board2.skippedAll();

  if (wholeSkip != wholeSkip2)
    THROW("Whole skips differ");

  if (wholeSkip)
    return true;

  // These object comparisons will actually throw exceptions, not fail.
  if (deal != board2.deal)
    return false;
  if (tableau != board2.tableau)
    return false;

  for (unsigned b = 0; b < len; b++)
  {
    if (skip[b] != board2.skip[b])
      THROW("Individual skips differ");
    if (skip[b])
      continue;

    if (instances[b] != board2.instances[b])
      return false;
  }

  return true;
}


bool Board::operator != (const Board& board2) const
{
  return ! (* this == board2);
}


string Board::strDeal(const Format format) const
{
  return deal.str(instances[0].getDealer(), format);
}


string Board::strDeal(
  const Player start,
  const Format format) const
{
  return deal.str(start, format);
}


string Board::strDealRemain(const Format format) const
{
  RunningDD runningDD;
  instances[numActive].getStateDDS(runningDD);

  Deal dltmp;
  dltmp.set(runningDD.dl.remainCards);

  return dltmp.str(instances[0].getDealer(), format);
}


string Board::strTableau(const Format format) const
{
  return tableau.str(format);
}


string Board::strContract(
  const unsigned instNo,
  const Format format) const
{
  if (skip[instNo] && LINset)
    return instances[instNo].strHeaderContract();
  else if (instNo < len)
    return instances[instNo].strContract(format);
  else if (LINset && instNo < 2)
    return instances[instNo].strHeaderContract();
  else
    return "";
}


string Board::strScore(
  const Format format,
  const bool scoringIsIMPs,
  const bool swapFlag) const
{
  const unsigned baseInst = (swapFlag ? 1u : 0u);
  if (numActive == baseInst || 
     ! scoringIsIMPs ||
     ! instances[0].hasResult() ||
     ! instances[1].hasResult())
  {
    return instances[numActive].strScore(format);
  }
  else
    return instances[numActive].strScore(format, instances[baseInst]);
}


string Board::strGivenScore(const Format format) const
{
  stringstream s;
  
  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    if (givenScore == 0.0f)
    {
      if ((format == BRIDGE_FORMAT_LIN ||
          format == BRIDGE_FORMAT_LIN_TRN) && 
          ! LINScoreSet)
        s << ",,";
      else
        s << "--,--,";
    }
    else if (givenScore > 0.0f)
      s << setprecision(1) << fixed << givenScore << ",,";
    else
      s << "," << setprecision(1) << fixed << -givenScore << ",";
    return s.str();
  }
  else if (format == BRIDGE_FORMAT_PBN && givenSet)
  {
    s << "[ScoreIMP \"NS ";
    if (numActive == 0)
      s << setprecision(2) << fixed << givenScore;
    else
      s << setprecision(2) << fixed << -givenScore;
    s << "\"]\n";

    return s.str();
  }
  else
    return "";
}


string Board::strScoreIMP(
  const Format format,
  const bool showFlag,
  const bool swapFlag) const
{
  if (format != BRIDGE_FORMAT_REC)
    return "";

  const unsigned baseInst = (swapFlag ? 1u : 0u);
  if (! showFlag || 
      ! instances[0].hasResult() ||
      ! instances[1].hasResult())
    return "Points:       ";

  return instances[numActive].strScoreIMP(format, instances[baseInst]);
}


string Board::strPlayersBoard(
  const Format format,
  const bool isIMPs,
  Board * refBoard) const
{
  string st;
  const unsigned no = (isIMPs ? len : 1);

  switch (format)
  {
    case BRIDGE_FORMAT_LIN:
      if (refBoard == nullptr)
      {
        for (unsigned i = 0; i < no; i++)
          st += instances[i].strPlayers(BRIDGE_FORMAT_LIN_RP) + ",";
      }
      else
      {
        for (unsigned i = 0; i < no; i++)
          st += instances[i].strPlayersDelta(refBoard->instances[i], 
            format);
      }
      return st;

    case BRIDGE_FORMAT_LIN_VG:

      for (unsigned i = 0; i < no; i++)
        st += instances[i].strPlayers(format) + ",";
      st.pop_back();

      return "pn|" + st + "|pg||\n";

    case BRIDGE_FORMAT_LIN_RP:

      for (unsigned i = 0; i < len; i++)
        st += instances[i].strPlayers(format) + ",";
      st.pop_back();

      return "pn|" + st + "|pg||\n\n";

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Board::strResult(
  const Format format,
  const bool scoringIsIMPs,
  const bool swapFlag) const
{
  const unsigned baseInst = (swapFlag ? 1u : 0u);
  if (numActive == baseInst || 
     ! scoringIsIMPs ||
     ! instances[baseInst].hasResult())
  {
    return instances[numActive].strResult(format);
  }
  else
    return instances[numActive].strResult(instances[baseInst], format);
}


string Board::strResult(
  const unsigned instNo,
  const string& team,
  const Format format) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].strResult(instances[instOther], team, format);
}


string Board::strIMPEntry(const int imps) const
{
  stringstream ss;
  if (imps == 0)
    ss << "    --    --";
  else if (imps > 0)
    ss << setw(6) << right << imps;
  else
    ss << setw(12) << right << -imps;
  return ss.str() + "\n";
}


int Board::IMPScore(const bool swapFlag) const
{
  const unsigned baseInst = (swapFlag ? 1u : 0u);

  if (numActive == baseInst || 
      ! instances[0].hasResult() ||
      ! instances[1].hasResult())
  {
    return 0;
  }
  else
    return instances[numActive].IMPScore(instances[baseInst]);
}


int Board::IMPScoreNew(const unsigned instNo) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].IMPScore(instances[instOther]);
}


string Board::strIMPSheetLine(
  const string& bno,
  unsigned& imps1,
  unsigned& imps2) const
{
  const string divider = "  |  ";
  stringstream ss;
  ss << setw(4) << right << bno << "  ";
  ss << instances[0].strResultEntry() << divider << 
    instances[1].strResultEntry() << divider;

  int imps;
  if (len < 2 || ! instances[0].hasResult() || ! instances[1].hasResult())
    imps = 0;
  else
    imps = Board::IMPScoreNew(0);
    // imps = instances[0].IMPScore(instances[1]);

  ss << Board::strIMPEntry(imps);

  if (imps >= 0)
    imps1 += static_cast<unsigned>(imps);
  else
    imps2 += static_cast<unsigned>(-imps);

  return ss.str();
}

