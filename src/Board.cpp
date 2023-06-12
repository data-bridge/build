/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#pragma warning(pop)

#include "bconst.h"
#include "Board.h"
#include "Valuation.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


Board::Board()
{
  Board::reset();
}


Board::~Board()
{
}


void Board::reset()
{
  len = 0;

  deal.reset();
  tableau.reset();
  valuation.clear();
  instances.clear();
  skip.clear();
  givenScore.reset();
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

  // If we ever have len > 2, we would find the lowest non-skipped
  // instance and copy that to all skipped instances.  Here we take
  // the shortcut.

  if (len == 2 && deal.isSet())
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);

    for (unsigned i = 0; i < len; i++)
      instances[i].setDeal(cards);

    if (skip[0] && ! skip[1])
      instances[0].copyDealerVul(instances[1]);
    else if (skip[1] && ! skip[0])
      instances[1].copyDealerVul(instances[0]);
  }

  return &instances[instNo];
}


const Instance& Board::getInstance(const unsigned instNo) const
{
  if (len == 0 || instNo >= len)
    THROW("Invalid instance selected");

  return instances[instNo];
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


void Board::setLINheader(LINData const * lin)
{
  if (lin->data[0].mp != "")
    Board::setScoreIMP(lin->data[0].mp, BRIDGE_FORMAT_LIN);
  else if (lin->data[1].mp != "")
    Board::setScoreIMP("-" + lin->data[1].mp, BRIDGE_FORMAT_LIN);
  
  for (unsigned i = 0; i < len; i++)
  {
    string st = "";
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      st += lin->data[i].players[(p+2) % 4];
      if (i < 3)
        st += ",";
    }

    if (st != ",,,")
      instances[i].setPlayers(st, BRIDGE_FORMAT_LIN, false);
    
    instances[i].setLINheader(&lin->data[i]);
  }
}


void Board::setDeal(
  const string& text,
  const Format format)
{
  // Assume the cards are unchanged.  Don't check for now.
  if (! deal.isSet())
    deal.set(text, format);

  unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
  string dlr;
  bool loadedFlag = false;

  for (unsigned i = 0; i < len; i++)
  {
    if (! instances[i].dealIsSet())
    {
      if (! loadedFlag)
      {
        deal.getDDS(cards);
        dlr = text.substr(0, 1);
        loadedFlag = true;
      }

      instances[i].setDeal(cards);

      // Won't get the dealer (which is part of the LIN deal) if
      // we don't copy it here.

      if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
        instances[i].setDealer(dlr, format);
    }
  }
}


void Board::setDealer(
  const string& text,
  const Format format)
{
  // A bit overkill, but cheap.
  for (unsigned i = 0; i < len; i++)
    instances[i].setDealer(text, format);
}


void Board::setVul(
  const string& text,
  const Format format)
{
  // A bit overkill, but cheap.
  for (unsigned i = 0; i < len; i++)
    instances[i].setVul(text, format);
}


Player Board::holdsCard(const string& text) const
{
  return deal.holdsCard(text);
}


void Board::setScoreIMP(
  const string& text,
  const Format format)
{
  givenScore.setIMP(text, format);
}


void Board::setScoreMP(
  const string& text,
  const Format format)
{
  givenScore.setIMP(text, format);
}


void Board::calculateScore()
{
  for (unsigned i = 0; i < len; i++)
    instances[i].calculateScore();
}


void Board::setTableau(
  const string& text,
  const Format format)
{
  tableau.set(text, format);
}


void Board::setTableauDDS(const int res[5][4])
{
  tableau.setDDS(res);
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


void Board::performValuation([[maybe_unused]] const bool fullFlag)
{
  unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
  deal.getDDS(cards);

  if (valuation.size() == 0)
    valuation.resize(BRIDGE_PLAYERS);

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    valuation[p].evaluate(cards[p], fullFlag);
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


bool Board::operator <= (const Board& board2) const
{
  if (len > board2.len)
    DIFF("Too many instances");

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
    if (skip[b])
      continue;

    if (board2.skip[b])
      THROW("Individual skips differ");

    if (! (instances[b] <= board2.instances[b]))
      return false;
  }

  return true;
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


string Board::strTableau(const Format format) const
{
  return tableau.str(format);
}


string Board::strContract(
  const unsigned instNo,
  const Format format) const
{
  if (instNo >= len)
    return "";
  else
    return instances[instNo].strContract(format);
}


string Board::strGivenScore(const Format format) const
{
  return givenScore.str(format);
}


string Board::strPlayersBoard(
  const Format format,
  const bool isIMPs,
  Board const * refBoard) const
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
      THROW("Invalid format: " + to_string(format));
  }
}


string Board::strScore(
  const unsigned instNo,
  const Format format) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].strScore(format, instances[instOther]);
}


string Board::strScoreIMP(
  const unsigned instNo,
  const Format format) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].strScoreIMP(format, instances[instOther]);
}


string Board::strResult(
  const unsigned instNo,
  const Format format) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].strResult(instances[instOther], format);
}


string Board::strResult(
  const unsigned instNo,
  const string& team,
  const Format format) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].strResult(instances[instOther], team, format);
}


int Board::IMPScore(const unsigned instNo) const
{
  const unsigned instOther = (instNo == 0 ? 1u : 0u);
  return instances[instNo].IMPScore(instances[instOther]);
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


string Board::strIMPSheetLine(
  const string& bno,
  unsigned& imps1,
  unsigned& imps2) const
{
  stringstream ss;
  ss << setw(4) << right << bno << "  " <<
    instances[0].strResultEntry() <<
    instances[1].strResultEntry();

  const int imps = (len < 2 ? 0 : Board::IMPScore(0));
  ss << Board::strIMPEntry(imps);

  if (imps >= 0)
    imps1 += static_cast<unsigned>(imps);
  else
    imps2 += static_cast<unsigned>(-imps);

  return ss.str();
}


string Board::strValuation() const
{
  string st;
  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    st += valuation[p].str();

  return st;
}


int Board::hash8() const
{
  const int h = valuation[0].handDist() ^
    ((valuation[1].handDist() * 5)) ^
    ((valuation[2].handDist() * 25)) ^
    ((valuation[3].handDist() * 125));

  return (h ^ (h >> 5)) & 0xff;
}


int Board::hash12() const
{
  const int h = valuation[0].handDist() ^
    ((valuation[1].handDist() * 5)) ^
    ((valuation[2].handDist() * 25)) ^
    ((valuation[3].handDist() * 125));

  return (h ^ (h >> 5)) & 0xfff;
}

