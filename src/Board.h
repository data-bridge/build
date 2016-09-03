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

    unsigned GetLength() const;

    bool SetInstance(
      unsigned no);

    bool SetDealerVul(
      const string& d,
      const string& v,
      const formatType f);

    bool CheckDealerVul(
      const string& d,
      const string& v,
      const formatType f) const;

    bool SetDeal(
      const string& s,
      const formatType f);
      
    bool SetDeal(
      const string cardsArg[][BRIDGE_SUITS],
      const formatType f);

    bool GetDealDDS(
      unsigned cards[][BRIDGE_SUITS]) const;

    bool SetTableau(
      const string text,
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

    bool AuctionIsOver() const;

    bool IsPassedOut() const;

    bool AddCall(
      const string& call,
      const string& alert = "");

    bool AddAlert(
      const unsigned alertNo,
      const string& alert);

    void AddPasses();

    bool UndoLastCall();

    bool AddAuction(
      const string& s,
      const formatType f);

    bool ContractIsSet() const;

    bool ResultIsSet() const;

    bool PassOut();

    bool SetContract(
      const vulType vul,
      const playerType declarer,
      const unsigned level,
      const denomType denom,
      const multiplierType mult);

    bool SetContract(
      const vulType vul,
      const string& cstring);

    bool SetTricks(
      const unsigned tricks);

    playStatus AddPlay(
      const string& str);

    playStatus AddPlays(
      const string& str,
      const formatType f);

    bool UndoLastPlay();

    bool PlayIsOver() const;

    claimStatus Claim(
      const unsigned tricks);

    bool ClaimIsMade() const;

    bool GetValuation(
      Valuation& valuation) const;

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
      const formatType f) const;

    string PlayAsString(
      const formatType f) const;

    string ClaimAsString(
      const formatType f) const;
};

#endif

