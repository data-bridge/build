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
  instances.clear();
  players.clear();
  auction.clear();
  contract.clear();
  play.clear();
  skip.clear();

  basicsFlag = false;
  givenScore = 0.0f;
  givenSet = false;
  LINset = false;
  LINScoreSet = false;
}


void Board::copyBasics(
  const unsigned noFrom,
  const unsigned noToMin,
  const unsigned noToMax)
{
  for (unsigned i = noToMin; i <= noToMax; i++)
  {
    auction[i].copyDealerVul(auction[noFrom]);
    contract[i].setVul(auction[noFrom].getVul());
  }

  if (deal.isSet())
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);
    for (unsigned i = noToMin; i <= noToMax; i++)
      play[i].setHoldingDDS(cards);
  }
}


void Board::acquireInstance(const unsigned instNo)
{
  if (instNo >= len)
  {
    const unsigned lenOld = len;
    len = instNo+1;

    instances.resize(len);
    players.resize(len);
    auction.resize(len);
    contract.resize(len);
    play.resize(len);
    skip.resize(len);

    for (unsigned i = lenOld; i < len; i++)
      skip[i] = true;

    players[0].setRoom("Open", BRIDGE_FORMAT_PBN);
    if (instNo == 1)
      players[1].setRoom("Closed", BRIDGE_FORMAT_PBN);

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
      Board::copyBasics(1, 0, 0);
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
      Board::copyBasics(0, 1, 1);
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


void Board::markInstanceSkip()
{
  skip[numActive] = true;
}


void Board::unmarkInstanceSkip()
{
  if (! skip[numActive])
    THROW("Instance is not skipped");

  skip[numActive] = false;
}


bool Board::skipped() const
{
  return skip[numActive];
}


bool Board::skipped(const unsigned no) const
{
  return (no >= len || skip[no]);
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
    if (LINdata.data[i].contract != "")
      Board::setContract(LINdata.data[i].contract, BRIDGE_FORMAT_LIN);

    string st = "";
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      st += LINdata.data[i].players[(p+2) % 4];
      if (i < 3)
        st += ",";
    }

    if (st != ",,,")
      Board::setPlayers(st, BRIDGE_FORMAT_LIN, false);
    
    instances[i].setLINheader(LINdata.data[i]);
  }
}


void Board::setDealer(
  const string& text,
  const Format format)
{
  auction[numActive].setDealer(text, format);

  instances[numActive].setDealer(text, format);
}


void Board::setVul(
  const string& text,
  const Format format)
{
  auction[numActive].setVul(text, format);

  const Vul v = auction[numActive].getVul();

  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_TXT)
  {
    // Fill up, just in case of a skip.
    for (unsigned i = 0; i < len; i++)
      contract[i].setVul(v);
  }
  else
    contract[numActive].setVul(v);
  

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

  if (! play[numActive].dealIsSet())
  {
    unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS];
    deal.getDDS(cards);
    play[numActive].setHoldingDDS(cards);

    instances[numActive].setPlayerDeal(cards);
  }

  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    // Fill up, just in case of a skip.
    const string d = text.substr(0, 1);
    for (unsigned i = 0; i < len; i++)
    {
      auction[i].setDealer(d, format);

      instances[i].setDealer(d, format);
    }
  }
}


Player Board::holdsCard(const string& text) const
{
  return deal.holdsCard(text);
}


// Auction

void Board::addCall(
  const string& call,
  const string& alert)
{
  auction[numActive].addCall(call, alert);

  instances[numActive].addCall(call, alert);
}


void Board::addAlert(
  const unsigned alertNo,
  const string& alert)
{
  auction[numActive].addAlert(alertNo, alert);

  instances[numActive].addAlert(alertNo, alert);
}


void Board::addPasses()
{
  auction[numActive].addPasses();

  instances[numActive].addPasses();
}


void Board::undoLastCall()
{
  auction[numActive].undoLastCall();

  instances[numActive].undoLastCall();
}


void Board::passOut()
{
  contract[numActive].passOut();

  instances[numActive].undoLastCall();
}


void Board::setAuction(
  const string& text,
  const Format format)
{
  auction[numActive].addAuction(text, format);

  if (auction[numActive].hasDealerVul())
    contract[numActive].setVul(auction[numActive].getVul());

  // Doesn't bother us unduly if there is no contract here.
  if (auction[numActive].getContract(contract[numActive]))
    play[numActive].setContract(contract[numActive]);
  
  instances[numActive].setAuction(text, format);
}


bool Board::hasDealerVul() const
{
  if (len == 0)
    return false;
  else
  {
    if (instances[0].hasDealerVul() != auction[0].hasDealerVul())
    {
      // TODO
      THROW("Different DV set");
    }

    return auction[0].hasDealerVul();
  }
}


bool Board::auctionIsOver() const
{
  if (instances[numActive].auctionIsOver() != auction[numActive].isOver())
  {
    // TODO
    THROW("Different isOver");
  }

  return auction[numActive].isOver();
}


bool Board::auctionIsEmpty() const
{
  if (instances[numActive].auctionIsEmpty() != auction[numActive].isEmpty())
  {
    // TODO
    THROW("Different isEmpty");
  }

  return auction[numActive].isEmpty();
}


bool Board::isPassedOut() const
{
  if (instances[numActive].isPassedOut() != auction[numActive].isPassedOut())
  {
    // TODO
    THROW("Different isPassedOut");
  }

  return auction[numActive].isPassedOut();
}


unsigned Board::lengthAuction() const
{
  if (instances[numActive].lengthAuction() != auction[numActive].length())
  {
    // TODO
    THROW("Different auction length");
  }

  return auction[numActive].length();
}


// Contract

void Board::setContract(
  const Vul vul,
  const string& cstring)
{
  contract[numActive].setContract(vul, cstring);

  instances[numActive].setContract(vul, cstring);
}


void Board::setContract(
  const string& text,
  const Format format)
{
  contract[numActive].setContract(text, format);

  play[numActive].setContract(contract[numActive]);

  instances[numActive].setContract(text, format);
}


void Board::setDeclarer(
  const string& text,
  const Format format)
{
  UNUSED(format);
  contract[numActive].setDeclarer(text);

  instances[numActive].setDeclarer(text, format);
}


bool Board::contractIsSet() const
{
  if (instances[numActive].contractIsSet() != contract[numActive].isSet())
  {
    // TODO
    THROW("Different contract isSet");
  }

  return contract[numActive].isSet();
}


void Board::setScore(
  const string& text,
  const Format format)
{
  contract[numActive].setScore(text, format);

  instances[numActive].setScore(text, format);
}


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
  contract[numActive].calculateScore();

  instances[numActive].calculateScore();
}


// Play

void Board::setPlays(
  const string& text,
  const Format format)
{
  play[numActive].setPlays(text, format);

  if (play[numActive].isOver())
    contract[numActive].setTricks( play[numActive].getTricks() );
  
  instances[numActive].setPlays(text, format);
}


void Board::undoLastPlay()
{
  play[numActive].undoPlay();

  instances[numActive].undoLastPlay();
}


bool Board::playIsOver() const
{
  if (instances[numActive].playIsOver() != play[numActive].isOver())
  {
    // TODO
    THROW("Different play isOver");
  }

  return play[numActive].isOver();
}


bool Board::hasClaim() const
{
  if (instances[numActive].hasClaim() != play[numActive].hasClaim())
  {
    // TODO
    THROW("Different play hasClaim");
  }

  return play[numActive].hasClaim();
}


void Board::getStateDDS(RunningDD& runningDD) const
{
  play[numActive].getStateDDS(runningDD);

  // TODO: instances[numActive].getStateDDS(runningDD);
}


// Result

void Board::setResult(
  const string& text,
  const Format format)
{
  contract[numActive].setResult(text, format);

  if (! contract[numActive].isPassedOut())
    play[numActive].makeClaim(contract[numActive].getTricks());
  
  instances[numActive].setResult(text, format);
}


bool Board::hasResult() const
{
  if (instances[numActive].hasResult() != contract[numActive].hasResult())
  {
    // TODO
    THROW("Different contract hasResult");
  }

  return contract[numActive].hasResult();
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

void Board::setPlayers(
  const string& text,
  const Format format,
  const bool hardFlag)
{
  players[numActive].set(text, format, hardFlag);

  instances[numActive].setPlayers(text, format, hardFlag);
}


void Board::setPlayer(
  const string& text,
  const Player player)
{
  players[numActive].setPlayer(text, player);

  instances[numActive].setPlayer(text, player);
}


void Board::copyPlayers(const Board& board2)
{
  for (unsigned i = 0; i < board2.len; i++)
  {
    players[i] = board2.players[i];
    instances[i].copyPlayers(board2.instances[i]);
  }
}


unsigned Board::missingPlayers() const
{
  if (instances[numActive].missingPlayers() != players[numActive].missing())
  {
    // TODO
    THROW("Different players missing");
  }

  return players[numActive].missing();
}


bool Board::overlappingPlayers() const
{
  if (len < 2)
    return false;
  else
  {
    if (instances[0].overlappingPlayers(instances[1]) != 
      players[0].overlap(players[1]))
    {
      // TODO
      THROW("Different players overlapping");
    }

    return players[0].overlap(players[1]);
  }
}


void Board::setRoom(
  const string& text,
  const Format format)
{
  players[numActive].setRoom(text, format);

  instances[numActive].setRoom(text, format);
}


bool Board::getValuation(Valuation& valuation) const
{
  // TODO
  UNUSED(valuation);
  THROW("Valuation not implemented yet");
  return true;
}


Room Board::room() const
{
  // TODO: 0 is open, 1 is closed.  Why do we have to ask?

  return players[numActive].room();
}


Room Board::roomFirst() const
{
  // TODO: 0 is open, 1 is closed.  Why do we have to ask?

  return players[0].room();
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

    if (players[b] != board2.players[b])
      return false;
    if (auction[b] != board2.auction[b])
      return false;
    if (contract[b] != board2.contract[b])
      return false;
    if (play[b] != board2.play[b])
      return false;
    
    if (instances[b] != board2.instances[b])
      return false;
  }

  return true;
}


bool Board::operator != (const Board& board2) const
{
  return ! (* this == board2);
}


string Board::strDealer(const Format format) const
{
  if (instances[numActive].strDealer(format) != auction[numActive].strDealer(format))
  {
    // TODO
    THROW("Different auction dealer");
  }

  return auction[numActive].strDealer(format);
}


string Board::strVul(const Format format) const
{
  if (instances[numActive].strVul(format) != contract[numActive].strVul(format))
  {
    // TODO
    THROW("Different contract vul");
  }

  return contract[numActive].strVul(format);
}


string Board::strDeal(const Format format) const
{
  if (instances[0].getDealer() != auction[0].getDealer())
  {
    // TODO
    THROW("Different auction dealer");
  }

  return deal.str(auction[0].getDealer(), format);
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
  play[numActive].getStateDDS(runningDD);

  Deal dltmp;
  dltmp.set(runningDD.dl.remainCards);

  if (instances[0].getDealer() != auction[0].getDealer())
  {
    // TODO
    THROW("Different auction dealer");
  }

  return dltmp.str(auction[0].getDealer(), format);
}


string Board::strTableau(const Format format) const
{
  return tableau.str(format);
}


string Board::strAuction(const Format format) const
{
  const string istr = instances[numActive].strAuction(format);

  if (format == BRIDGE_FORMAT_TXT)
  {
    int lengths[BRIDGE_PLAYERS];
    for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
    {
      Player pp = PLAYER_DDS_TO_TXT[p];
      lengths[p] = static_cast<int>
        (players[numActive].strPlayer(pp, format).length());
      lengths[p] = Max(12, lengths[p]+1);
    }

    if (istr != auction[numActive].str(format, lengths))
    {
      // TODO
      THROW("Different auction1");
    }

    return auction[numActive].str(format, lengths);
  }
  else
  {
    if (istr != auction[numActive].str(format))
    {
      // TODO
      THROW("Different auction2");
    }

    return auction[numActive].str(format);
  }
}


string Board::strContract(const Format format) const
{
  if (instances[numActive].strContract(format) != contract[numActive].str(format))
  {
    // TODO
    THROW("Different contract str");
  }

  return contract[numActive].str(format);
}


string Board::strContract(
  const unsigned instNo,
  const Format format) const
{
  if (skip[instNo] && LINset)
  {
    if (instances[instNo].strHeaderContract() != LINdata.data[instNo].contract)
    {
      // TODO
      THROW("Different contract str1");
    }
    return LINdata.data[instNo].contract;
  }
  else if (instNo < len)
  {
    if (instances[instNo].strContract(format) != contract[instNo].str(format))
    {
      // TODO
      THROW("Different contract str1");
    }
    return contract[instNo].str(format);
  }
  else if (LINset && instNo < 2)
  {
    if (instances[instNo].strHeaderContract() != LINdata.data[instNo].contract)
    {
      // TODO
      THROW("Different contract str1");
    }
    return LINdata.data[instNo].contract;
  }
  else
  {
    return "";
  }
}


string Board::strDeclarer(const Format format) const
{
  if (instances[numActive].strDeclarer(format) != 
    contract[numActive].strDeclarer(format))
  {
    // TODO
    THROW("Different declarer ");
  }

  return contract[numActive].strDeclarer(format);
}


string Board::strDenom(const Format format) const
{
  // TODO: Delete, unused
  return contract[numActive].strDenom(format);
}


string Board::strTricks(const Format format) const
{
  if (instances[numActive].strTricks(format) != 
    contract[numActive].strTricks(format))
  {
    // TODO
    THROW("Different tricks");
  }
  return contract[numActive].strTricks(format);
}


string Board::strScore(
  const Format format,
  const bool scoringIsIMPs,
  const bool swapFlag) const
{
  const unsigned baseInst = (swapFlag ? 1u : 0u);
  if (numActive == baseInst || 
     ! scoringIsIMPs ||
     ! contract[0].hasResult() ||
     ! contract[1].hasResult())
  {
    if (instances[numActive].strScore(format) != 
      contract[numActive].strScore(format))
    {
      // TODO
      cout << "format " << FORMAT_NAMES[format] << endl;
      cout << "LIN no " << LINdata.no << endl;
      THROW("Different score1: '" +
        instances[numActive].strScore(format) + "' vs '" +
        contract[numActive].strScore(format) + "'");
    }
    return contract[numActive].strScore(format);
  }
  else
  {
    if (instances[numActive].strScore(format, contract[baseInst].getScore()) != 
      contract[numActive].strScore(format, contract[baseInst].getScore()))
    {
      // TODO
      THROW("Different score1");
    }
    return contract[numActive].strScore(format, contract[baseInst].getScore());
  }
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
      ! contract[0].hasResult() ||
      ! contract[1].hasResult())
    return "Points:       ";

  if (instances[numActive].strScoreIMP(format, contract[baseInst].getScore()) != 
    contract[numActive].strScoreIMP(format, contract[baseInst].getScore()))
  {
    // TODO
    THROW("Different tricks");
  }
  return contract[numActive].strScoreIMP(format, 
    contract[baseInst].getScore());
}


int Board::IMPScore(const bool swapFlag) const
{
  const unsigned baseInst = (swapFlag ? 1u : 0u);

  if (numActive == baseInst || 
      ! contract[0].hasResult() ||
      ! contract[1].hasResult())
    return 0;
  else
  {
    if (instances[numActive].IMPScore(contract[baseInst].getScore()) != 
      contract[numActive].IMPScore(contract[baseInst].getScore()))
    {
      // TODO
      THROW("Different IMPscore");
    }
    return contract[numActive].IMPScore(contract[baseInst].getScore());
  }
}


string Board::strLead(const Format format) const
{
  if (instances[numActive].strLead(format) != play[numActive].strLead(format))
  {
    // TODO
    THROW("Different contract str1");
  }

  return play[numActive].strLead(format);
}


string Board::strPlay(const Format format) const
{
  if (instances[numActive].strPlay(format) != play[numActive].str(format))
  {
    // TODO
    THROW("Different contract str1");
  }

  return play[numActive].str(format);
}


string Board::strClaim(const Format format) const
{
  if (instances[numActive].strClaim(format) != play[numActive].strClaim(format))
  {
    // TODO
    THROW("Different claim");
  }

  return play[numActive].strClaim(format);
}


string Board::strPlayer(
  const Player player,
  const Format format) const
{
  if (instances[numActive].strPlayer(player, format) != 
      players[numActive].strPlayer(player, format))
  {
    // TODO
    THROW("Different Player");
  }

  return players[numActive].strPlayer(player, format);
}


string Board::strPlayers(
  const unsigned instNo,
  const Format format) const
{
  if (instances[instNo].strPlayers(format) != players[instNo].str(format))
  {
    // TODO
    THROW("Different players");
  }
  return players[instNo].str(format);
}


string Board::strPlayersFromLINHeader(const unsigned instNo) const
{
  string st;
  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
    st += LINdata.data[instNo].players[PLAYER_DDS_TO_LIN[i]] + ",";
  st.pop_back(); // Trailing comma

  if (instances[instNo].strPlayersFromLINHeader() != st)
  {
    THROW("Different players from header");
  }
  return st;
}


string Board::strPlayers(
  const Format format,
  const bool isIMPs,
  Board * refBoard) const
{
  string st1, st2;
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      if (isIMPs)
      {
        if (len == 1 && LINset)
        {
          st1 += Board::strPlayersDelta(refBoard, 0, format);
          st1 += ",,,,";
        }
        else
        {
          for (unsigned i = 0; i < len; i++)
            st1 += Board::strPlayersDelta(refBoard, i, format);
        }
        return st1;
      }
      else
      {
        st1 += Board::strPlayersDelta(refBoard, 0, format);
        return st1;
      }

    case BRIDGE_FORMAT_LIN_VG:
      if (! skip[0])
        st1 = Board::strPlayers(0, format);
      else if (LINset)
        st1 = Board::strPlayersFromLINHeader(0);
      else
        st1 = "South,West,North,East";

      if (len == 2)
        st2 = Board::strPlayers(1, format);
      else if (LINset)
        st2 = Board::strPlayersFromLINHeader(1);
      else
        st2 = "South,West,North,East";
      return "pn|" + st1 + "," + st2 + "|pg||\n";

    case BRIDGE_FORMAT_LIN_RP:
      if (len != 2)
        return "";

      if (Board::roomFirst() == BRIDGE_ROOM_CLOSED)
      {
        // TODO: Should soon not happen?
        st1 = Board::strPlayers(1, format);
        st2 = Board::strPlayers(0, format);
      }
      else
      {
        st1 = Board::strPlayers(0, format);
        st2 = Board::strPlayers(1, format);
      }
      return "pn|" + st1 + "," + st2 + "|pg||\n\n";

    case BRIDGE_FORMAT_LIN_TRN:
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_REC:
      return players[numActive].str(format);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Board::strPlayersDelta(
  Board * refBoard,
  const unsigned instNo,
  const Format format) const
{
  if (refBoard == nullptr)
    return players[instNo].str(BRIDGE_FORMAT_LIN_RP) + ",";
  else
    return players[instNo].strDelta(refBoard->players[instNo], format);
}


string Board::strContracts(
  const string& contractFromHeader,
  const Format format) const
{
  string st = "";

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      if (len == 2 && Board::roomFirst() == BRIDGE_ROOM_CLOSED)
      {
        // TODO: Soon shouldn't happen anymore?
        st += Board::strContract(1, format) + ",";
        st += Board::strContract(0, format) + ",";
      }
      else
      {
        for (unsigned i = 0; i < len; i++)
          st += Board::strContract(i, format) + ",";
      }

      if (len == 1)
        st += contractFromHeader + ",";
      return st;

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
     ! contract[baseInst].hasResult())
  {
    if (instances[numActive].strResult(format) !=
        contract[numActive].strResult(format))
    {
      THROW("strResult1");
    }
    return contract[numActive].strResult(format);
  }
  else
  {
    if (instances[numActive].strResult(format, contract[baseInst].getScore()) !=
        contract[numActive].strResult(format, contract[baseInst].getScore()))
    {
      THROW("strResult2");
    }
    return contract[numActive].strResult(format, contract[baseInst].getScore());
  }
}


string Board::strResult(
  const Format format,
  const string& team,
  const bool swapFlag) const
{
  const unsigned baseInst = (swapFlag ? 1u : 0u);
  if (numActive == baseInst || 
      ! contract[baseInst].hasResult())
  {
    if (instances[numActive].strResult(format) !=
        contract[numActive].strResult(format))
    {
      THROW("strResult1");
    }
    return contract[numActive].strResult(format);
  }
  else
  {
    if (instances[numActive].strResult(format, team, contract[baseInst].getScore()) !=
        contract[numActive].strResult(format, contract[baseInst].getScore(), team))
    {
      THROW("strResult1");
    }
    return contract[numActive].strResult(format, 
        contract[baseInst].getScore(), team);
  }
}


string Board::strRoom(
  const unsigned no,
  const Format format) const
{
  // TODO: Don't need to ask players
  return players[numActive].strRoom(no, format);
}


string Board::strResultEntry(const unsigned instNo) const
{
  if (instNo >= len)
    return string(22, ' ');

  stringstream ss;
  ss << setw(8) << left << contract[instNo].str(BRIDGE_FORMAT_LIN);

  const int score = contract[instNo].getScore();
  if (score == 0)
    ss << "     --     --";
  else if (score > 0)
    ss << setw(7) << right << score << setw(7) << "";
  else
    ss << setw(14) << right << -score;
  return ss.str();
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
  const string divider = "  |  ";
  stringstream ss;
  ss << setw(4) << right << bno << "  ";
  ss << Board::strResultEntry(0u) << divider << 
    Board::strResultEntry(1u) << divider;

  int imps;
  if (len < 2 || ! contract[0].hasResult() || ! contract[1].hasResult())
    imps = 0;
  else
    imps = contract[0].IMPScore(contract[1].getScore());

  ss << Board::strIMPEntry(imps);

  if (imps >= 0)
    imps1 += static_cast<unsigned>(imps);
  else
    imps2 += static_cast<unsigned>(-imps);

  return ss.str();
}

