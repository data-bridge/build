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

extern Debug debug;


Board::Board():
  players(1), 
  auction(1), 
  contract(1), 
  play(1)
{
  len = 1;
  numActive = 0;
}


Board::~Board()
{
}


void Board::Reset()
{
  len = 1;
  players.resize(len);
  auction.resize(len);
  contract.resize(len);
  play.resize(len);
  numActive = 0;
}


unsigned Board::NewInstance()
{
  len++;
  auction[numActive+1].CopyDealerVulFrom(auction[numActive]);
  return numActive++;
}


bool Board::SetDealerVul(
  const string& d,
  const string& v,
  const formatType f)
{
  // Only the first one is independent.
  return auction[0].SetDealerVul(d, v, f);
}


bool Board::CheckDealerVul(
  const string& d,
  const string& v,
  const formatType f) const
{
  return auction[0].CheckDealerVul(d, v, f);
}


bool Board::SetDeal(
  const string& s,
  const formatType f)
{
  return deal.Set(s, f);
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


bool Board::SetTableau(
  const string text,
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


bool Board::AuctionIsOver() const
{
  return auction[numActive].IsOver();
}


bool Board::IsPassedOut() const
{
  return auction[numActive].IsPassedOut();
}


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


bool Board::AddAuction(
  const string& s,
  const formatType f)
{
  return auction[numActive].AddAuction(s, f);
}


bool Board::ContractIsSet() const
{
  return contract[numActive].ContractIsSet();
}


bool Board::ResultIsSet() const
{
  return contract[numActive].ResultIsSet();
}


bool Board::PassOut()
{
  return contract[numActive].SetPassedOut();
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


bool Board::SetTricks(
  const unsigned tricks)
{
  return contract[numActive].SetTricks(tricks);
}


playStatus Board::AddPlay(
  const string& str)
{
  return play[numActive].AddPlay(str);
}


playStatus Board::AddPlays(
  const string& str,
  const formatType f)
{
  return play[numActive].AddPlays(str, f);
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


bool Board::GetValuation(
  Valuation& valuation) const
{
  // TODO
  UNUSED(valuation);
  return true;
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

