/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_INSTANCE_H
#define BRIDGE_INSTANCE_H

#include <string>

#include "Players.h"
#include "Auction.h"
#include "Contract.h"
#include "Play.h"

using namespace std;


class Instance
{
  private:

    Players players;
    Auction auction;
    Contract contract;
    Play play;

    LINInstData LINdata;
    bool LINset;


  public:

    Instance();

    ~Instance();

    void reset();

    void copyDealerVul(const Instance& inst2);

    void setPlayerDeal(
      const unsigned cards[BRIDGE_PLAYERS][BRIDGE_SUITS]);

    void setLINheader(const LINInstData& lin);

    void setDealer(
      const string& text,
      const Format format);

    void setVul(
      const string& text,
      const Format format);

    Player getDealer() const;

    Vul getVul() const;

    // Deal

    void setDeal(
      const string& text,
      const Format format);
      
    // Auction

    void addCall(
      const string& call,
      const string& alert = "");

    void addAlert(
      const unsigned alertNo,
      const string& alert);

    void addPasses();

    void undoLastCall();

    void passOut();

    void setAuction(
      const string& text,
      const Format format);

    bool auctionIsEmpty() const;

    bool hasDealerVul() const;

    bool auctionIsOver() const;

    bool isPassedOut() const;

    unsigned lengthAuction() const;

    // Contract

    void setContract(
      const Vul vul,
      const string& cstring);

    void setContract(
      const string& text,
      const Format format);

    void setDeclarer(
      const string& text,
      const Format format);

    bool contractIsSet() const;

    void setScore(
      const string& text,
      const Format format);

    void calculateScore();

    // Play

    void setPlays(
      const string& text,
      const Format format);

    void undoLastPlay();

    bool dealIsSet() const;

    bool playIsOver() const;

    bool hasClaim() const;

    unsigned getTricks() const;

    void getStateDDS(RunningDD& runningDD) const;

    // Result

    void setResult(
      const string& text,
      const Format format);
      
    bool hasResult() const;

    // Players

    void setPlayers(
      const string& text,
      const Format format);

    void setPlayers(
      const string& text,
      const Format format,
      const bool hardFlag);

    void setPlayer(
      const string& text,
      const Player player);

    void setWest(
      const string& text,
      const Format format);

    void setNorth(
      const string& text,
      const Format format);

    void setEast(
      const string& text,
      const Format format);

    void setSouth(
      const string& text,
      const Format format);

    void copyPlayers(const Instance& inst2);

    unsigned missingPlayers() const;
    bool overlappingPlayers(const Instance& inst2) const;

    void setRoom(
      const string& s,
      const Format format);

    Room room() const;

    bool operator == (const Instance& inst2) const;
    bool operator != (const Instance& inst2) const;

    string strDealer(const Format format) const;
    string strVul(const Format format) const;
    string strAuction(const Format format) const;
    string strContract(const Format format) const;
    string strHeaderContract() const;
    string strDeclarer(const Format format) const;
    string strTricks(const Format format) const;

    string strScore(const Format format) const;
    string strScore(
      const Format format,
      const Instance& inst2) const;

    string strScoreIMP(
      const Format format,
      const Instance& inst2) const;

    int IMPScore(const Instance& inst2) const;
    int IMPScore(const int refScore) const;

    string strLead(const Format format) const;
    string strPlay(const Format format) const;
    string strClaim(const Format format) const;
    
    string strPlayer(
      const Player player,
      const Format format) const;
    
    string strPlayers(const Format format) const;

    string strPlayersDelta(
      const Instance& inst2,
      const Format format) const;

    string strPlayersFromLINHeader() const;

    string strResultEntry() const;

    string strResult(const Format format) const;

    string strResult(
      const Instance& inst2,
      const Format format) const;

    string strResult(
      const Instance& inst2,
      const string& team,
      const Format format) const;
    
    string strRoom(
      const unsigned no,
      const Format format) const;
};

#endif

