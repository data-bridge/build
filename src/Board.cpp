/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <regex>
#include <iterator>
#include <map>
#include <assert.h>

#include "bconst.h"
#include "Board.h"
#include "Valuation.h"
#include "portab.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


Board::Board():
  players(0), 
  auction(0), 
  contract(0), 
  play(0)
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
  players.clear();
  auction.clear();
  contract.clear();
  play.clear();

  givenScore = 0.0f;
  LINset = false;
}


void Board::newInstance()
{
  numActive = len++;
  players.resize(len);
  auction.resize(len);
  contract.resize(len);
  play.resize(len);

  // Default, may change.
  if (numActive == 0)
    Board::SetRoom("Open", 0, BRIDGE_FORMAT_PBN);
  else if (numActive == 1)
    Board::SetRoom("Closed", 1, BRIDGE_FORMAT_PBN);

  if (numActive == 0)
    return;

  // Reuse data from first instance.
  auction[numActive].copyDealerVul(auction[0]);
  contract[numActive].SetVul(auction[0].getVul());

  if (deal.isSet())
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);
    play[numActive].SetHoldingDDS(cards);
  }
}


void Board::setInstance(const unsigned no)
{
  if (len == 0 || no > len-1)
    THROW("Invalid instance selected");

  numActive = no;
}


unsigned Board::getInstance() const
{
  return numActive;
}


unsigned Board::count() const
{
  return len;
}


void Board::setLINheader(const LINData& lin)
{
  if (! LINset)
    LINdata = lin;
  LINset = true;

  if (LINdata.mp[0] != "")
    Board::setScoreIMP(LINdata.mp[0], BRIDGE_FORMAT_LIN);
  else if (LINdata.mp[1] != "")
    Board::setScoreIMP("-" + LINdata.mp[1], BRIDGE_FORMAT_LIN);
  else
    Board::setScoreIMP("0.0", BRIDGE_FORMAT_LIN);
}


void Board::setDealer(
  const string& text,
  const Format format)
{
  auction[0].setDealer(text, format);
}


void Board::setVul(
  const string& text,
  const Format format)
{
  auction[0].setVul(text, format);

  contract[numActive].SetVul(auction[0].getVul());
}


playerType Board::GetDealer() const
{
  return auction[0].getDealer();
}


// Deal

void Board::setDeal(
  const string& text,
  const Format format)
{
    // Assume the cards are unchanged.  Don't check for now.
  if (numActive > 0)
    return;

  deal.set(text, format);

  if (numActive == 0)
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);

    if (! play[0].SetHoldingDDS(cards))
      THROW("Cannot set holding"); // TODO: Probably play already throws?
  }

  if (format == BRIDGE_FORMAT_LIN)
    auction[numActive].setDealer(text.substr(0, 1), format);
}


bool Board::GetDealDDS(
  unsigned cards[][BRIDGE_SUITS]) const
{
  deal.getDDS(cards);
  return true;
}


// Auction

bool Board::AddCall(
  const string& call,
  const string& alert)
{
  auction[numActive].addCall(call, alert);
  return true;
}


bool Board::AddAlert(
  const unsigned alertNo,
  const string& alert)
{
  auction[numActive].addAlert(alertNo, alert);
  return true;
}


void Board::AddPasses()
{
  auction[numActive].addPasses();
}


bool Board::UndoLastCall()
{
  auction[numActive].undoLastCall();
  return true;
}


bool Board::PassOut()
{
  return contract[numActive].SetPassedOut();
}


void Board::setAuction(
  const string& text,
  const Format format)
{
  auction[numActive].addAuction(text, format);

  if (auction[numActive].hasDealerVul())
    contract[numActive].SetVul(auction[numActive].getVul());

  // Doesn't bother us unduly.
  if (! auction[numActive].getContract(contract[numActive]))
    return;

  play[numActive].SetContract(contract[numActive]);
}


bool Board::AuctionIsOver() const
{
  return auction[numActive].isOver();
}


bool Board::AuctionIsEmpty() const
{
  return auction[numActive].isEmpty();
}


bool Board::IsPassedOut() const
{
  return auction[numActive].isPassedOut();
}


// Contract

bool Board::ContractIsSet() const
{
  return contract[numActive].ContractIsSet();
}


bool Board::SetContract(
  const Vul vul,
  const Player declarer,
  const unsigned level,
  const Denom denom,
  const multiplierType mult)
{
  return contract[numActive].SetContract(vul, declarer, level, denom, mult);
}


void Board::setContract(
  const Vul vul,
  const string& cstring)
{
  contract[numActive].SetContract(vul, cstring);
}


void Board::setContract(
  const string& text,
  const Format format)
{
  contract[numActive].SetContract(text, format);

  play[numActive].SetContract(contract[numActive]);
}


void Board::setDeclarer(
  const string& text,
  const Format format)
{
  contract[numActive].SetDeclarer(text, format);
}


bool Board::SetTricks(
  const unsigned tricks)
{
  return contract[numActive].SetTricks(tricks);
}


void Board::setScore(
  const string& text,
  const Format format)
{
  contract[numActive].setScore(text, format);
}


void Board::setScoreIMP(
  const string& text,
  const Format format)
{
  // We regenerate this ourselves, so mostly ignore for now.
  if (format == BRIDGE_FORMAT_LIN)
  {
    if (! str2float(text, givenScore))
      THROW("Bad IMP score");
  }
}


void Board::setScoreMP(
  const string& text,
  const Format format)
{
  // We ignore this for now.
  UNUSED(format);
  if (! str2float(text, givenScore))
    THROW("Bad matchpoint score");
}


// Play

playStatus Board::AddPlay(
  const string& str)
{
  return play[numActive].AddPlay(str);
}


void Board::setPlays(
  const string& text,
  const Format format)
{
  if (! play[numActive].SetPlays(text, format))
    THROW("Cannot set play"); // TODO: Already throws?

  if (play[numActive].PlayIsOver())
    contract[numActive].SetTricks( play[numActive].GetTricks() );
}


bool Board::SetPlays(
  const vector<string>& list,
  const Format format)
{
  return play[numActive].SetPlays(list, format);
}


bool Board::UndoLastPlay()
{
  return play[numActive].UndoPlay();
}


bool Board::PlayIsOver() const
{
  return play[numActive].PlayIsOver();
}


claimStatus Board::Claim(
  const unsigned tricks)
{
  return play[numActive].Claim(tricks);
}


bool Board::ClaimIsMade() const
{
  return play[numActive].ClaimIsMade();
}


// Result

void Board::setResult(
  const string& text,
  const Format format)
{
  if (! contract[numActive].SetResult(text, format))
    THROW("Cannot set result"); // TODO: Already throws?

  if (contract[numActive].IsPassedOut())
    return;

  if (play[numActive].Claim(contract[numActive].GetTricks()) !=
      PLAY_CLAIM_NO_ERROR)
    THROW("Bad claim"); // TODO: Already throws?
}


bool Board::ResultIsSet() const
{
  return contract[numActive].ResultIsSet();
}


// Tableau

void Board::setTableau(
  const string& text,
  const Format format)
{
  tableau.set(text, format);
}


bool Board::SetTableauEntry(
  const Player player,
  const Denom denom,
  const unsigned tricks)
{
  return tableau.set(player, denom, tricks);
}


unsigned Board::GetTableauEntry(
  const Player player,
  const Denom denom) const
{
  return tableau.get(player, denom);
}


bool Board::GetPar(
  Player dealer,
  Vul v,
  string& text) const
{
  return tableau.getPar(dealer, v, text);
}


bool Board::GetPar(
  Player dealer,
  Vul v,
  list<Contract>& text) const
{
  return tableau.getPar(dealer, v, text);
}


// Players

bool Board::SetPlayers(
  const string& text,
  const Format format)
{
  players[numActive].set(text, format);
  return true;
}


bool Board::SetPlayer(
  const string& text,
  const Player player)
{
  players[numActive].setPlayer(text, player);
  return true;
}


void Board::CopyPlayers(
  const Board& b2,
  const unsigned inst)
{
  players[inst] = b2.players[inst];
}


bool Board::SetRoom(
  const string& s,
  const unsigned inst,
  const Format f)
{
  players[inst].setRoom(s, f);
  return true;
}


bool Board::GetValuation(
  Valuation& valuation) const
{
  // TODO
  UNUSED(valuation);
  assert(false);
  return true;
}


void Board::CalculateScore()
{
  contract[numActive].CalculateScore();
}


bool Board::CheckBoard() const
{
  // TODO
  return true;
}


roomType Board::GetRoom() const
{
  return players[numActive].room();
}


bool Board::operator == (const Board& b2) const
{
  if (len != b2.len)
    DIFF("Different number of instances");

  if (deal != b2.deal)
    return false;
  if (tableau != b2.tableau)
    return false;

  for (unsigned b = 0; b < len; b++)
  {
    if (players[b] != b2.players[b])
      return false;
    if (auction[b] != b2.auction[b])
      return false;
    if (contract[b] != b2.contract[b])
      return false;
    if (play[b] != b2.play[b])
      return false;
  }

  return true;
}


bool Board::operator != (const Board& b2) const
{
  return ! (* this == b2);
}


string Board::strDealer(const Format format) const
{
  return auction[numActive].strDealer(format);
}


string Board::strVul(const Format format) const
{
  return auction[numActive].strVul(format);
}


string Board::strDeal(const Format format) const
{
  return deal.str(auction[0].getDealer(), format);
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


string Board::strAuction(const Format format) const
{
  if (format == BRIDGE_FORMAT_TXT)
  {
    unsigned lengths[BRIDGE_PLAYERS];
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      Player pp = static_cast<Player>((p+3) % 4);
      lengths[p] = players[numActive].strPlayer(pp, format).length();
      lengths[p] = Max(12u, static_cast<int>(lengths[p]+1));
    }
    return auction[numActive].str(format, lengths);
  }
  else
    return auction[numActive].str(format);
}


string Board::strContract(const Format format) const
{
  return contract[numActive].AsString(format);
}


string Board::strDeclarer(const Format format) const
{
  return contract[numActive].DeclarerAsString(format);
}


string Board::strTricks(const Format format) const
{
  return contract[numActive].TricksAsString(format);
}


string Board::ScoreAsString(
  const Format format,
  const bool scoringIsIMPs) const
{
  if (numActive != 1 || 
     ! scoringIsIMPs ||
     ! contract[0].ResultIsSet())
    return contract[numActive].ScoreAsString(format);
  else
    return contract[numActive].ScoreAsString(format, 
      contract[0].GetScore());
}


string Board::GivenScoreAsString(
  const Format format) const
{
  UNUSED(format);
  stringstream s;
  if (givenScore == 0.0f)
    s << "--,--,";
  else if (givenScore > 0.0f)
    s << setprecision(1) << fixed << givenScore << ",,";
  else
    s << "," << setprecision(1) << fixed << -givenScore << ",";
  return s.str();
}


string Board::ScoreIMPAsString(
  const Format format,
  const bool showFlag) const
{
  if (format != BRIDGE_FORMAT_REC)
    return "";

  if (! showFlag)
    return "Points:       ";

  return contract[numActive].ScoreIMPAsString(format, 
    contract[0].GetScore());
}


int Board::ScoreIMPAsInt() const
{
  if (numActive != 1 || ! contract[0].ResultIsSet())
    return 0;
  else
    return contract[numActive].ScoreIMPAsInt(contract[0].GetScore());
}


string Board::LeadAsString(const Format format) const
{
  return play[numActive].LeadAsString(format);
}


string Board::PlayAsString(const Format format) const
{
  return play[numActive].AsString(format);
}


string Board::ClaimAsString(const Format format) const
{
  return play[numActive].ClaimAsString(format);
}


string Board::strPlayer(
  const Player player,
  const Format format) const
{
  return players[numActive].strPlayer(player, format);
}


string Board::PlayersAsString(
  const Format format) const
{
  // TODO: Not a reliable indicator of open/closed.
  return players[numActive].str(format, numActive == 1);
}


string Board::PlayersAsDeltaString(
  Board * refBoard,
  const Format format) const
{
  if (refBoard == nullptr)
    return players[numActive].str(BRIDGE_FORMAT_LIN_RP);
  else
    return players[numActive].strDelta(refBoard->players[numActive], 
      format);
}


string Board::ResultAsString(
  const Format format,
  const bool scoringIsIMPs) const
{
  if (numActive != 1 || 
     ! scoringIsIMPs ||
     ! contract[0].ResultIsSet())
    return contract[numActive].ResultAsString(format);
  else
    return contract[numActive].ResultAsString(format, 
      contract[0].GetScore());
}


string Board::ResultAsString(
  const Format format,
  const string& team) const
{
  if (numActive != 1 || 
     ! contract[0].ResultIsSet())
    return contract[numActive].ResultAsString(format);
  else
    return 
      contract[numActive].ResultAsString(format, 
        contract[0].GetScore(), team);
}


string Board::strRoom(
  const unsigned no,
  const Format format) const
{
  return players[numActive].strRoom(no, format);
}

