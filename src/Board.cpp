/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "bconst.h"
#include "Board.h"
#include "Debug.h"
#include "portab.h"

#include <map>
#include <assert.h>

extern Debug debug;


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
}


unsigned Board::NewInstance()
{
  numActive = len++;
  players.resize(len);
  auction.resize(len);
  contract.resize(len);
  play.resize(len);

  if (numActive > 0)
  {
    auction[numActive].CopyDealerVulFrom(auction[0]);

    if (deal.IsSet())
    {
      unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
      if (deal.GetDDS(cards))
        play[numActive].SetHoldingDDS(cards);
    }
  }

  return numActive;
}


bool Board::SetInstance(const unsigned no)
{
  if (len == 0 || no > len-1)
  {
    LOG("Invalid instance selected");
    return false;
  }
  else
  {
    numActive = no;
    return true;
  }
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
  return auction[0].SetDealerVul(d, v, f);
}


bool Board::SetDealer(
  const string& d,
  const formatType f)
{
  return auction[0].SetDealer(d, f);
}


bool Board::SetVul(
  const string& v,
  const formatType f)
{
  return auction[0].SetVul(v, f);
}


bool Board::CheckDealerVul(
  const string& d,
  const string& v,
  const formatType f) const
{
  return auction[0].CheckDealerVul(d, v, f);
}


// Deal

bool Board::SetDeal(
  const string& s,
  const formatType f)
{
  if (! deal.Set(s, f))
    return false;

  if (numActive == 0)
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    if (! deal.GetDDS(cards))
      return false;

    return play[0].SetHoldingDDS(cards);
  }
  return true;
}


bool Board::SetDeal(
  const string cards[][BRIDGE_SUITS],
  const formatType f)
{
  return deal.Set(cards, f);
}


bool Board::GetDealDDS(
  unsigned cards[][BRIDGE_SUITS]) const
{
  return deal.GetDDS(cards);
}


// Auction

bool Board::AddCall(
  const string& call,
  const string& alert)
{
  return auction[numActive].AddCall(call, alert);
}


bool Board::AddAlert(
  const unsigned alertNo,
  const string& alert)
{
  return auction[numActive].AddAlert(alertNo, alert);
}


void Board::AddPasses()
{
  auction[numActive].AddPasses();
}


bool Board::UndoLastCall()
{
  return auction[numActive].UndoLastCall();
}


bool Board::PassOut()
{
  return contract[numActive].SetPassedOut();
}


bool Board::SetAuction(
  const string& s,
  const formatType f)
{
  if (! auction[numActive].AddAuction(s, f))
    return false;

  if (auction[numActive].DVIsSet())
  {
    if (! contract[numActive].SetVul(auction[numActive].GetVul()))
      return false;
  }

  return true;
}


bool Board::AuctionIsOver() const
{
  return auction[numActive].IsOver();
}


bool Board::IsPassedOut() const
{
  return auction[numActive].IsPassedOut();
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
  return play[numActive].SetPlays(str, f);
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
  return contract[numActive].SetResult(text, f);
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
  return tableau.Set(text, f);
}

bool Board::SetTableauEntry(
  const playerType p,
  const denomType d,
  const unsigned t)
{
  return tableau.SetEntry(p, d, t);
}


unsigned Board::GetTableauEntry(
  const playerType p,
  const denomType d) const
{
  tableau.GetEntry(p, d);
}


bool Board::GetPar(
  playerType dealer,
  vulType v,
  string& text) const
{
  return tableau.GetPar(dealer, v, text);
}


bool Board::GetPar(
  playerType dealer,
  vulType v,
  list<Contract>& text) const
{
  return tableau.GetPar(dealer, v, text);
}


// Players

bool Board::SetPlayers(
  const string& text,
  const formatType f)
{
  return players[numActive].SetPlayers(text, f);
}


bool Board::SetPlayer(
  const string& text,
  const playerType player)
{
  return players[numActive].SetPlayer(text, player);
}


bool Board::PlayersAreSet(const unsigned inst) const
{
  return players[inst].PlayersAreSet();
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
  return players[inst].SetRoom(s, f);
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


bool Board::operator == (const Board& b2) const
{
  if (len != b2.len)
  {
    LOG("Different number of instances");
    return false;
  }

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
  return auction[numActive].DealerAsString(f);
}


string Board::VulAsString(
  const formatType f) const
{
  return auction[numActive].VulAsString(f);
}


string Board::DealAsString(
  const playerType start,
  const formatType f) const
{
  return deal.AsString(start, f);
}


string Board::TableauAsString(
  const formatType f) const
{
  return tableau.AsString(f);
}


string Board::AuctionAsString(
  const formatType f,
  const string& names) const
{
  return auction[numActive].AsString(f, names);
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
  const formatType f) const
{
  return contract[numActive].ScoreAsString(f);
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
  return players[numActive].PlayerAsString(p, f);
}


string Board::PlayersAsString(
  const formatType f) const
{
  return players[numActive].AsString(f);
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


string Board::RoomAsString(
  const unsigned no,
  const formatType f) const
{
  return players[numActive].RoomAsString(no, f);
}

