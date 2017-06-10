/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "Instance.h"
#include "bconst.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


Instance::Instance()
{
  Instance::reset();
}


Instance::~Instance()
{
}


void Instance::reset()
{
  players.reset();
  auction.reset();
  contract.reset();
  play.reset();

  LINset = false;
}


void Instance::copyDealerVul(const Instance& inst2)
{
  auction.copyDealerVul(inst2.auction);
  contract.setVul(auction.getVul());
}


void Instance::setPlayerDeal(
  const unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS])
{
  play.setHoldingDDS(cards);
}


void Instance::setLINheader(const LINInstData& lin)
{
  if (LINset)
    return;

  LINdata = lin;
  LINset = true;

  if (LINdata.contract != "")
    Instance::setContract(LINdata.contract, BRIDGE_FORMAT_LIN);

  const string st = Instance::strPlayersFromLINHeader();
  if (st != ",,,")
    Instance::setPlayers(st, BRIDGE_FORMAT_LIN, false);
}


void Instance::setDealer(
  const string& text,
  const Format format)
{
  auction.setDealer(text, format);
}


void Instance::setVul(
  const string& text,
  const Format format)
{
  auction.setVul(text, format);
  contract.setVul(auction.getVul());
}


Player Instance::getDealer() const
{
  return auction.getDealer();
}


Vul Instance::getVul() const
{
  return auction.getVul();
}


// Auction

void Instance::addCall(
  const string& call,
  const string& alert)
{
  auction.addCall(call, alert);
}


void Instance::addAlert(
  const unsigned alertNo,
  const string& alert)
{
  auction.addAlert(alertNo, alert);
}


void Instance::addPasses()
{
  auction.addPasses();
}


void Instance::undoLastCall()
{
  auction.undoLastCall();
}


void Instance::passOut()
{
  contract.passOut();
}


void Instance::setAuction(
  const string& text,
  const Format format)
{
  auction.addAuction(text, format);

  if (auction.hasDealerVul())
    contract.setVul(auction.getVul());

  // Doesn't bother us unduly if there is no contract here.
  if (auction.getContract(contract))
    play.setContract(contract);
}


bool Instance::hasDealerVul() const
{
  return auction.hasDealerVul();
}


bool Instance::auctionIsOver() const
{
  return auction.isOver();
}


bool Instance::auctionIsEmpty() const
{
  return auction.isEmpty();
}


bool Instance::isPassedOut() const
{
  return auction.isPassedOut();
}


unsigned Instance::lengthAuction() const
{
  return auction.length();
}


// Contract

void Instance::setContract(
  const Vul vul,
  const string& cstring)
{
  contract.setContract(vul, cstring);
}


void Instance::setContract(
  const string& text,
  const Format format)
{
  contract.setContract(text, format);
  play.setContract(contract);
}


void Instance::setDeclarer(
  const string& text,
  const Format format)
{
  UNUSED(format);
  contract.setDeclarer(text);
}


bool Instance::contractIsSet() const
{
  return contract.isSet();
}


void Instance::setScore(
  const string& text,
  const Format format)
{
  contract.setScore(text, format);
}


void Instance::calculateScore()
{
  contract.calculateScore();
}


// Play

void Instance::setPlays(
  const string& text,
  const Format format)
{
  play.setPlays(text, format);

  if (play.isOver())
    contract.setTricks(play.getTricks());
}


void Instance::undoLastPlay()
{
  play.undoPlay();
}


bool Instance::dealIsSet() const
{
  return play.dealIsSet();
}


bool Instance::playIsOver() const
{
  return play.isOver();
}


bool Instance::hasClaim() const
{
  return play.hasClaim();
}


unsigned Instance::getTricks() const
{
  return play.getTricks();
}


void Instance::getStateDDS(RunningDD& runningDD) const
{
  play.getStateDDS(runningDD);
}


// Result

void Instance::setResult(
  const string& text,
  const Format format)
{
  contract.setResult(text, format);

  if (! contract.isPassedOut())
    play.makeClaim(contract.getTricks());
}


bool Instance::hasResult() const
{
  return contract.hasResult();
}


// Players

void Instance::setPlayers(
  const string& text,
  const Format format)
{
  players.set(text, format, false);
}


void Instance::setPlayers(
  const string& text,
  const Format format,
  const bool hardFlag)
{
  players.set(text, format, hardFlag);
}


void Instance::setPlayer(
  const string& text,
  const Player player)
{
  players.setPlayer(text, player);
}


void Instance::setWest(
  const string& text,
  const Format format)
{
  UNUSED(format);
  players.setPlayer(text, BRIDGE_WEST);
}


void Instance::setNorth(
  const string& text,
  const Format format)
{
  UNUSED(format);
  players.setPlayer(text, BRIDGE_NORTH);
}


void Instance::setEast(
  const string& text,
  const Format format)
{
  UNUSED(format);
  players.setPlayer(text, BRIDGE_EAST);
}


void Instance::setSouth(
  const string& text,
  const Format format)
{
  UNUSED(format);
  players.setPlayer(text, BRIDGE_SOUTH);
}


void Instance::copyPlayers(const Instance& inst2)
{
  players = inst2.players;
}


unsigned Instance::missingPlayers() const
{
  return players.missing();
}


bool Instance::overlappingPlayers(const Instance& inst2) const
{
  return players.overlap(inst2.players);
}


void Instance::setRoom(
  const string& text,
  const Format format)
{
  players.setRoom(text, format);
}


Room Instance::room() const
{
  // Do we have to ask?
  return players.room();
}


bool Instance::operator == (const Instance& inst2) const
{
  if (players != inst2.players)
    return false;
  if (auction != inst2.auction)
    return false;
  if (contract != inst2.contract)
    return false;
  if (play != inst2.play)
    return false;

  return true;
}


bool Instance::operator != (const Instance& inst2) const
{
  return ! (* this == inst2);
}


string Instance::strDealer(const Format format) const
{
  return auction.strDealer(format);
}


string Instance::strVul(const Format format) const
{
  return contract.strVul(format);
}


string Instance::strAuction(const Format format) const
{
  if (format == BRIDGE_FORMAT_TXT)
  {
    int lengths[BRIDGE_PLAYERS];
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      Player pp = PLAYER_DDS_TO_TXT[p];
      lengths[p] = static_cast<int>
        (players.strPlayer(pp, format).length());
      lengths[p] = Max(12, lengths[p]+1);
    }
    return auction.str(format, lengths);
  }
  else
    return auction.str(format);
}


string Instance::strContract(const Format format) const
{
  return contract.str(format);
}


string Instance::strHeaderContract() const
{
  return LINdata.contract;
}


string Instance::strDeclarer(const Format format) const
{
  return contract.strDeclarer(format);
}


string Instance::strTricks(const Format format) const
{
  return contract.strTricks(format);
}


string Instance::strScore(const Format format) const
{
  return contract.strScore(format);
}


string Instance::strScore(
  const Format format,
  const Instance& inst2) const
{
  return contract.strScore(format, inst2.contract.getScore());
}


string Instance::strScoreIMP(
  const Format format,
  const Instance& inst2) const
{
  return contract.strScoreIMP(format, inst2.contract.getScore());
}


int Instance::IMPScore(const Instance& inst2) const
{
  if (! contract.hasResult() || ! inst2.contract.hasResult())
    return 0;
  else
    return contract.IMPScore(inst2.contract.getScore());
}


string Instance::strLead(const Format format) const
{
  return play.strLead(format);
}


string Instance::strPlay(const Format format) const
{
  return play.str(format);
}


string Instance::strClaim(const Format format) const
{
  return play.strClaim(format);
}


string Instance::strPlayer(
  const Player player,
  const Format format) const
{
  return players.strPlayer(player, format);
}


string Instance::strPlayers(const Format format) const
{
  return players.str(format);
}


string Instance::strPlayersFromLINHeader() const
{
  string st;
  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
    st += LINdata.players[PLAYER_DDS_TO_LIN[i]] + ",";
  st.pop_back(); // Trailing comma
  return st;
}


string Instance::strPlayersDelta(
  const Instance& inst2,
  const Format format) const
{
  return players.strDelta(inst2.players, format);
}


string Instance::strResult(const Format format) const
{
  return contract.strResult(format);
}


string Instance::strResult(
  const Instance& inst2,
  const Format format) const
{
  if (inst2.contract.hasResult())
    return contract.strResult(format, inst2.contract.getScore());
  else
    return contract.strResult(format);
}


string Instance::strResult(
  const Instance& inst2,
  const string& team,
  const Format format) const
{
  if (inst2.contract.hasResult())
    return contract.strResult(format, inst2.contract.getScore(), team);
  else
  return contract.strResult(format);
}


string Instance::strResultEntry() const
{
  stringstream ss;
  ss << setw(8) << left << contract.str(BRIDGE_FORMAT_LIN);

  const int score = contract.getScore();
  if (score == 0)
    ss << "     --     --";
  else if (score > 0)
    ss << setw(7) << right << score << setw(7) << "";
  else
    ss << setw(14) << right << -score;
  return ss.str();
}


string Instance::strRoom(
  const unsigned no,
  const Format format) const
{
  // TODO: Don't need to ask players?
  return players.strRoom(no, format);
}

