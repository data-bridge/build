/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "bconst.h"
#include "Board.h"
#include "Valuation.h"
#include "portab.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"

#include <regex>
#include <iterator>
#include <map>
#include <assert.h>


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


void Board::Reset()
{
  len = 0;
  players.clear();
  auction.clear();
  contract.clear();
  play.clear();
  numActive = 0;
  givenScore = 0.0f;
  LINset = false;
}


unsigned Board::NewInstance()
{
  numActive = len++;
  players.resize(len);
  auction.resize(len);
  contract.resize(len);
  play.resize(len);

  // Default
  if (numActive == 0)
    Board::SetRoom("Open", 0, BRIDGE_FORMAT_PBN);
  else if (numActive == 1)
    Board::SetRoom("Closed", 1, BRIDGE_FORMAT_PBN);

  if (numActive > 0)
  {
    auction[numActive].copyDealerVul(auction[0]);
    contract[numActive].SetVul(auction[0].getVul());

    if (deal.isSet())
    {
      unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
      deal.getDDS(cards);
      play[numActive].SetHoldingDDS(cards);
    }
  }

  return numActive;
}


bool Board::SetInstance(const unsigned no)
{
  if (len == 0 || no > len-1)
    THROW("Invalid instance selected");
  else
  {
    numActive = no;
    return true;
  }
}


void Board::SetLINheader(const LINdataType& lin)
{
  if (! LINset)
    LINdata = lin;
  LINset = true;

  if (LINdata.mp[0] != "")
    Board::SetScoreIMP(LINdata.mp[0], BRIDGE_FORMAT_LIN);
  else if (LINdata.mp[1] != "")
    Board::SetScoreIMP("-" + LINdata.mp[1], BRIDGE_FORMAT_LIN);
  else
    Board::SetScoreIMP("0.0", BRIDGE_FORMAT_LIN);
}


unsigned Board::GetLength() const
{
  return len;
}


unsigned Board::GetInstance() const
{
  return numActive;
}


bool Board::SetDealerVul(
  const string& d,
  const string& v,
  const formatType f)
{
  // Only the first one is independent.
  auction[0].setDealerVul(d, v, f);
  return true;
}


bool Board::SetDealer(
  const string& d,
  const formatType f)
{
  auction[0].setDealer(d, f);
  return true;
}


bool Board::SetVul(
  const string& v,
  const formatType f)
{
  auction[0].setVul(v, f);
  if (! contract[numActive].SetVul(auction[0].getVul()))
    return false;
  return true;
}


playerType Board::GetDealer() const
{
  return auction[0].getDealer();
}


// Deal

bool Board::SetDeal(
  const string& s,
  const formatType f)
{
  if (numActive > 0)
  {
    // Assume the cards are unchanged.  Don't check for now.
    return true;
  }

  deal.set(s, f);

  if (numActive == 0)
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);

    if (! play[0].SetHoldingDDS(cards))
      return false;
  }

  if (f == BRIDGE_FORMAT_LIN)
  {
    string d = s.substr(0, 1);
    auction[numActive].setDealer(d, f);
  }

  return true;
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


bool Board::SetAuction(
  const string& s,
  const formatType f)
{
  auction[numActive].addAuction(s, f);

  if (auction[numActive].hasDealerVul())
  {
    if (! contract[numActive].SetVul(auction[numActive].getVul()))
      return false;
  }

  // Doesn't bother us unduly.
  if (! auction[numActive].getContract(contract[numActive]))
    return true;

  return play[numActive].SetContract(contract[numActive]);
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
  const vulType vul,
  const playerType declarer,
  const unsigned level,
  const denomType denom,
  const multiplierType mult)
{
  return contract[numActive].SetContract(vul, declarer, level, denom, mult);
}


bool Board::SetContract(
  const vulType vul,
  const string& cstring)
{
  return contract[numActive].SetContract(vul, cstring);
}


bool Board::SetContract(
  const string& text,
  const formatType f)
{
  if (! contract[numActive].SetContract(text, f))
    return false;

  return play[numActive].SetContract(contract[numActive]);
}


bool Board::SetDeclarer(
  const string& text,
  const formatType f)
{
  return contract[numActive].SetDeclarer(text, f);
}


bool Board::SetTricks(
  const unsigned tricks)
{
  return contract[numActive].SetTricks(tricks);
}


bool Board::SetScore(
  const string& text,
  const formatType f)
{
  return contract[numActive].SetScore(text, f);
}


bool Board::SetScoreIMP(
  const string& text,
  const formatType f)
{
  // We regenerate this ourselves, so mostly ignore for now.
  if (f == BRIDGE_FORMAT_LIN)
    return str2float(text, givenScore);
  else
    return true;
}


bool Board::SetScoreMP(
  const string& text,
  const formatType f)
{
  // We ignore this for now.
  UNUSED(f);
  return str2float(text, givenScore);
}


// Play

playStatus Board::AddPlay(
  const string& str)
{
  return play[numActive].AddPlay(str);
}


bool Board::SetPlays(
  const string& str,
  const formatType f)
{
  if (! play[numActive].SetPlays(str, f))
    return false;

  if (play[numActive].PlayIsOver())
    return contract[numActive].SetTricks( play[numActive].GetTricks() );

  return true;
}


bool Board::SetPlays(
  const vector<string>& list,
  const formatType f)
{
  return play[numActive].SetPlays(list, f);
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

bool Board::SetResult(
  const string& text,
  const formatType f)
{
  if (! contract[numActive].SetResult(text, f))
    return false;

  if (contract[numActive].IsPassedOut())
    return true;
  else if (play[numActive].Claim(contract[numActive].GetTricks()) ==
      PLAY_CLAIM_NO_ERROR)
    return true;
  else
    return false;
}


bool Board::ResultIsSet() const
{
  return contract[numActive].ResultIsSet();
}


// Tableau

bool Board::SetTableau(
  const string& text,
  const formatType f)
{
  return tableau.set(text, f);
}

bool Board::SetTableauEntry(
  const playerType p,
  const denomType d,
  const unsigned t)
{
  return tableau.set(p, d, t);
}


unsigned Board::GetTableauEntry(
  const playerType p,
  const denomType d) const
{
  return tableau.get(p, d);
}


bool Board::GetPar(
  playerType dealer,
  vulType v,
  string& text) const
{
  return tableau.getPar(dealer, v, text);
}


bool Board::GetPar(
  playerType dealer,
  vulType v,
  list<Contract>& text) const
{
  return tableau.getPar(dealer, v, text);
}


// Players

bool Board::SetPlayers(
  const string& text,
  const formatType f)
{
  players[numActive].set(text, f);
  return true;
}


bool Board::SetPlayer(
  const string& text,
  const playerType player)
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
  const formatType f)
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


string Board::DealerAsString(
  const formatType f) const
{
  return auction[numActive].strDealer(f);
}


string Board::VulAsString(
  const formatType f) const
{
  return auction[numActive].strVul(f);
}


string Board::DealAsString(
  const playerType start,
  const formatType f) const
{
  return deal.str(start, f);
}


string Board::TableauAsString(
  const formatType f) const
{
  return tableau.str(f);
}


string Board::AuctionAsString(
  const formatType f,
  const string& names) const
{
  UNUSED(names);
  if (f == BRIDGE_FORMAT_TXT)
  {
    unsigned lengths[BRIDGE_PLAYERS];
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      playerType pp = static_cast<playerType>((p+3) % 4);
      lengths[p] = players[numActive].strPlayer(pp, f).length();
    }
    return auction[numActive].str(f, lengths);
  }
  else
    return auction[numActive].str(f);
}


string Board::ContractAsString(
  const formatType f) const
{
  return contract[numActive].AsString(f);
}


string Board::DeclarerAsString(
  const formatType f) const
{
  return contract[numActive].DeclarerAsString(f);
}


string Board::TricksAsString(
  const formatType f) const
{
  return contract[numActive].TricksAsString(f);
}


string Board::ScoreAsString(
  const formatType f,
  const bool scoringIsIMPs) const
{
  if (numActive != 1 || 
     ! scoringIsIMPs ||
     ! contract[0].ResultIsSet())
    return contract[numActive].ScoreAsString(f);
  else
    return contract[numActive].ScoreAsString(f, contract[0].GetScore());
}


string Board::GivenScoreAsString(
  const formatType f) const
{
  UNUSED(f);
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
  const formatType f,
  const bool showFlag) const
{
  if (f != BRIDGE_FORMAT_REC)
    return "";

  if (! showFlag)
    return "Points:       ";

  return contract[numActive].ScoreIMPAsString(f, contract[0].GetScore());
}


int Board::ScoreIMPAsInt() const
{
  if (numActive != 1 || ! contract[0].ResultIsSet())
    return 0;
  else
    return contract[numActive].ScoreIMPAsInt(contract[0].GetScore());
}


string Board::LeadAsString(
  const formatType f) const
{
  return play[numActive].LeadAsString(f);
}


string Board::PlayAsString(
  const formatType f) const
{
  return play[numActive].AsString(f);
}


string Board::ClaimAsString(
  const formatType f) const
{
  return play[numActive].ClaimAsString(f);
}


string Board::PlayerAsString(
  const playerType p,
  const formatType f) const
{
  return players[numActive].strPlayer(p, f);
}


string Board::PlayersAsString(
  const formatType f) const
{
  // TODO: Not a reliable indicator of open/closed.
  return players[numActive].str(f, numActive == 1);
}


string Board::PlayersAsDeltaString(
  Board * refBoard,
  const formatType f) const
{
  if (refBoard == nullptr)
    return players[numActive].str(BRIDGE_FORMAT_LIN_RP);
  else
    return players[numActive].strDelta(refBoard->players[numActive], f);
}


string Board::WestAsString(
  const formatType f) const
{
  return players[numActive].strPlayer(BRIDGE_WEST, f);
}


string Board::NorthAsString(
  const formatType f) const
{
  return players[numActive].strPlayer(BRIDGE_NORTH, f);
}


string Board::EastAsString(
  const formatType f) const
{
  return players[numActive].strPlayer(BRIDGE_EAST, f);
}


string Board::SouthAsString(
  const formatType f) const
{
  return players[numActive].strPlayer(BRIDGE_SOUTH, f);
}


string Board::ResultAsString(
  const formatType f,
  const bool scoringIsIMPs) const
{
  if (numActive != 1 || 
     ! scoringIsIMPs ||
     ! contract[0].ResultIsSet())
    return contract[numActive].ResultAsString(f);
  else
    return contract[numActive].ResultAsString(f, contract[0].GetScore());
}


string Board::ResultAsString(
  const formatType f,
  const string& team) const
{
  if (numActive != 1 || 
     ! contract[0].ResultIsSet())
    return contract[numActive].ResultAsString(f);
  else
    return 
      contract[numActive].ResultAsString(f, contract[0].GetScore(), team);
}


string Board::RoomAsString(
  const unsigned no,
  const formatType f) const
{
  return players[numActive].strRoom(no, f);
}

