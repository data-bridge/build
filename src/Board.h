/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BOARD_H
#define BRIDGE_BOARD_H

#include "Deal.h"
#include "Tableau.h"
#include "Valuation.h"
#include "Players.h"
#include "Auction.h"
#include "Contract.h"
#include "Play.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

using namespace std;


class Board
{
  private:

    Deal deal;
    Tableau tableau;
    vector<Players> players;
    vector<Auction> auction;
    vector<Contract> contract;
    vector<Play> play;

    unsigned len;
    unsigned numActive;


  public:

    Board();

    ~Board();

    void Reset();

    unsigned NewInstance();

    bool SetInstance(
      const unsigned no);

    unsigned GetLength() const;

    unsigned GetInstance() const;

    bool SetDealerVul(
      const string& d,
      const string& v,
      const formatType f);

    bool SetDealer(
      const string& d,
      const formatType f);

    bool SetVul(
      const string& v,
      const formatType f);

    bool CheckDealerVul(
      const string& d,
      const string& v,
      const formatType f) const;

    // Deal

    bool SetDeal(
      const string& s,
      const formatType f);
      
    bool SetDeal(
      const string cardsArg[][BRIDGE_SUITS],
      const formatType f);

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

    bool SetAuction(
      const string& s,
      const formatType f);

    bool SetAuction(
      const vector<string>& s,
      const formatType f);

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

    bool SetContract(
      const vulType vul,
      const string& cstring);

    bool SetContract(
      const string& text,
      const formatType f);

    bool SetDeclarer(
      const string& text,
      const formatType f);

    bool SetTricks(
      const unsigned tricks);

    bool SetScore(
      const string& text,
      const formatType f);

    bool SetScoreIMP(
      const string& text,
      const formatType f);

    bool SetScoreMP(
      const string& text,
      const formatType f);

    // Play

    playStatus AddPlay(
      const string& str);

    bool SetPlays(
      const string& str,
      const formatType f);

    bool SetPlays(
      const vector<string>& str,
      const formatType f);

    bool UndoLastPlay();

    bool PlayIsOver() const;

    claimStatus Claim(
      const unsigned tricks);

    bool ClaimIsMade() const;

    // Result

    bool SetResult(
      const string& text,
      const formatType f);
      
    bool ResultIsSet() const;

    // Tableau
    //
    bool SetTableau(
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

    bool PlayersAreSet(const unsigned instance) const;

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

    bool operator == (const Board& b2) const;

    bool operator != (const Board& b2) const;

    string DealerAsString(
      const formatType f) const;

    string VulAsString(
      const formatType f) const;

    string DealAsString(
      const playerType start,
      const formatType f) const;

    string TableauAsString(
      const formatType f) const;

    string AuctionAsString(
      const formatType f,
      const string& names = "") const;

    string ContractAsString(
      const formatType f) const;

    string DeclarerAsString(
      const formatType f) const;

    string TricksAsString(
      const formatType f) const;

    string ScoreAsString(
      const formatType f,
      const bool scoringIsIMPs) const;

    string LeadAsString(
      const formatType f) const;

    string PlayAsString(
      const formatType f) const;

    string ClaimAsString(
      const formatType f) const;
    
    string PlayerAsString(
      const playerType p,
      const formatType f) const;
    
    string PlayersAsString(
      const formatType f) const;
    
    string WestAsString(
      const formatType f) const;
    
    string NorthAsString(
      const formatType f) const;
    
    string EastAsString(
      const formatType f) const;
    
    string SouthAsString(
      const formatType f) const;
    
    string ResultAsString(
      const formatType f) const;
    
    string ResultAsString(
      const formatType f,
      const bool scoringIsIMPs) const;

    string RoomAsString(
      const unsigned no,
      const formatType f) const;
};

#endif

