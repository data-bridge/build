/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BOARD_H
#define BRIDGE_BOARD_H

#include <string>
#include <vector>

#include "Deal.h"
#include "Tableau.h"
#include "Players.h"
#include "Auction.h"
#include "Contract.h"
#include "Play.h"

using namespace std;

class Valuation;


struct LINData
{
  string contract[2];
  string players[2][4];
  string mp[2];
  string no;
};


class Board
{
  private:

    Deal deal;
    Tableau tableau;
    vector<Players> players;
    vector<Auction> auction;
    vector<Contract> contract;
    vector<Play> play;
    float givenScore;

    unsigned len;
    unsigned numActive;
    LINData LINdata;
    bool LINset;


  public:

    Board();

    ~Board();

    void reset();

    void newInstance();

    void setInstance(const unsigned no);

    unsigned getInstance() const;

    unsigned count() const;

    void setLINheader(const LINData& lin);

    void setDealer(
      const string& text,
      const Format format);

    void setVul(
      const string& text,
      const Format format);

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

    bool auctionIsOver() const;

    bool isPassedOut() const;

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

    void setScoreIMP(
      const string& text,
      const Format format);

    void setScoreMP(
      const string& text,
      const Format format);

    // Play

    void setPlays(
      const string& text,
      const Format format);

    bool undoLastPlay();

    bool playIsOver() const;

    bool claimIsMade() const;

    // Result

    void setResult(
      const string& text,
      const Format format);
      
    bool resultIsSet() const;

    // Tableau
    //
    void setTableau(
      const string& text,
      const Format format);

    bool SetTableauEntry(
      const Player player,
      const Denom denom,
      const unsigned tricks);

    unsigned GetTableauEntry(
      const Player player,
      const Denom denom) const;

    bool GetPar(
      Player dealer,
      Vul v,
      string& text) const;

    bool GetPar(
      Player dealer,
      Vul v,
      list<Contract>& text) const;

    // Players

    void setPlayers(
      const string& text,
      const Format format);

    void setPlayer(
      const string& text,
      const Player player);

    void copyPlayers(const Board& board2);

    void setRoom(
      const string& s,
      const Format format);

    bool getValuation(Valuation& valuation) const;

    void calculateScore();

    bool checkBoard() const;

    Room room() const;

    bool operator == (const Board& b2) const;

    bool operator != (const Board& b2) const;

    string strDealer(const Format format) const;
    string strVul(const Format format) const;
    string strDeal(const Format format) const;
    string strDeal(
      const Player player,
      const Format format) const;
    string strTableau(const Format format) const;
    string strAuction(const Format format) const;
    string strContract(const Format format) const;
    string strDeclarer(const Format format) const;
    string strTricks(const Format format) const;

    string ScoreAsString(
      const Format format,
      const bool scoringIsIMPs) const;

    string ScoreIMPAsString(
      const Format format,
      const bool showFlag) const;

    string GivenScoreAsString(const Format format) const;

    int ScoreIMPAsInt() const;

    string LeadAsString(const Format format) const;

    string PlayAsString(const Format format) const;

    string ClaimAsString(const Format format) const;
    
    string strPlayer(
      const Player player,
      const Format format) const;
    
    string PlayersAsString(const Format format) const;
    
    string PlayersAsDeltaString(
      Board * refBoard,
      const Format format) const;
    
    string ResultAsString(const Format format) const;
    
    string ResultAsString(
      const Format format,
      const bool scoringIsIMPs) const;

    string ResultAsString(
      const Format format,
      const string& team) const;

    string strRoom(
      const unsigned no,
      const Format format) const;
};

#endif

