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

    playerType GetDealer() const;

    // Deal

    void setDeal(
      const string& text,
      const Format format);
      
    bool GetDealDDS(
      unsigned cards[][BRIDGE_SUITS]) const;

    // Auction

    bool AddCall(
      const string& call,
      const string& alert = "");

    bool AddAlert(
      const unsigned alertNo,
      const string& alert);

    void AddPasses();

    bool UndoLastCall();

    bool PassOut();

    void setAuction(
      const string& text,
      const Format format);

    bool SetAuction(
      const vector<string>& s,
      const formatType f);

    bool AuctionIsEmpty() const;

    bool AuctionIsOver() const;

    bool IsPassedOut() const;

    // Contract

    bool ContractIsSet() const;

    bool SetContract(
      const vulType vul,
      const playerType declarer,
      const unsigned level,
      const denomType denom,
      const multiplierType mult);

    void setContract(
      const Vul vul,
      const string& cstring);

    void setContract(
      const string& text,
      const formatType f);

    void setDeclarer(
      const string& text,
      const formatType f);

    bool SetTricks(
      const unsigned tricks);

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

    playStatus AddPlay(
      const string& str);

    void setPlays(
      const string& text,
      const Format format);

    bool SetPlays(
      const vector<string>& str,
      const formatType f);

    bool UndoLastPlay();

    bool PlayIsOver() const;

    claimStatus Claim(
      const unsigned tricks);

    bool ClaimIsMade() const;

    // Result

    void setResult(
      const string& text,
      const Format format);
      
    bool ResultIsSet() const;

    // Tableau
    //
    void setTableau(
      const string& text,
      const formatType f);

    bool SetTableauEntry(
      const playerType p,
      const denomType d,
      const unsigned t);

    unsigned GetTableauEntry(
      const playerType p,
      const denomType d) const;

    bool GetPar(
      playerType dealer,
      vulType v,
      string& text) const;

    bool GetPar(
      playerType dealer,
      vulType v,
      list<Contract>& text) const;

    // Players

    bool SetPlayers(
      const string& text,
      const formatType f);

    bool SetPlayer(
      const string& text,
      const playerType player);

    void CopyPlayers(
      const Board& b2,
      const unsigned instance);

    bool SetRoom(
      const string& s,
      const unsigned instance,
      const formatType f);

    bool GetValuation(
      Valuation& valuation) const;

    void CalculateScore();

    bool CheckBoard() const;

    roomType GetRoom() const;

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

