/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BOARD_H
#define BRIDGE_BOARD_H

#include <string>
#include <vector>

#include "Deal.h"
#include "Tableau.h"
#include "Instance.h"

using namespace std;

class Valuation;


class Board
{
  private:

    Deal deal;
    Tableau tableau;
    vector<Instance> instances;
    vector<bool> skip;
    float givenScore;
    bool givenSet;

    unsigned len;
    unsigned numActive;
    bool basicsFlag;
    LINData LINdata;
    bool LINset;
    bool LINScoreSet;

    string strPlayersFromLINHeader(const unsigned instNo) const;

    string strIMPEntry(const int imps) const;
    

  public:

    Board();

    ~Board();

    void reset();

    Instance * acquireInstance(const unsigned instNo = 0);

    void setInstance(const unsigned no);

    const Instance& getInstance(const unsigned instNo) const;

    bool skipped() const;
    bool skipped(const unsigned no) const;
    bool skippedAll() const;

    void markUsed(const unsigned instNo);

    unsigned count() const;
    unsigned countAll() const;

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
      
    Player holdsCard(const string& text) const;

    // Contract

    void setScoreIMP(
      const string& text,
      const Format format);

    void setScoreMP(
      const string& text,
      const Format format);

    void calculateScore();

    // Tableau
    //
    void setTableau(
      const string& text,
      const Format format);

    bool setTableauEntry(
      const Player player,
      const Denom denom,
      const unsigned tricks);

    unsigned getTableauEntry(
      const Player player,
      const Denom denom) const;

    bool getPar(
      Player dealer,
      Vul vul,
      string& text) const;

    bool getPar(
      Player dealer,
      Vul vul,
      list<Contract>& text) const;

    // Players

    void setPlayers(
      const string& text,
      const Format format,
      const bool hardFlag = true);

    void setPlayer(
      const string& text,
      const Player player);

    void copyPlayers(const Board& board2);

    unsigned missingPlayers() const;
    bool overlappingPlayers() const;

    void setRoom(
      const string& s,
      const Format format);

    bool getValuation(Valuation& valuation) const;

    Room room() const;
    Room roomFirst() const;

    bool operator == (const Board& b2) const;

    bool operator != (const Board& b2) const;

    string strDeal(const Format format) const;
    string strDeal(
      const Player player,
      const Format format) const;
    string strDealRemain(const Format format) const;
    string strTableau(const Format format) const;
    string strContract(
      const unsigned instNo,
      const Format format) const;

    string strScore(
      const Format format,
      const bool scoringIsIMPs,
      const bool swapFlag = false) const;

    string strGivenScore(const Format format) const;

    string strScoreIMP(
      const Format format,
      const bool showFlag,
      const bool swapFlag = false) const;

    int IMPScore(const bool swapFlag = false) const;

    string strPlayersDelta(
      Board * refBoard,
      const unsigned instNo,
      const Format format) const;

    string strPlayer(
      const Player player,
      const Format format) const;
    
    string strPlayers(
      const unsigned instNo,
      const Format format) const;

    string strPlayers(const Format format) const;

    string strPlayersBoard(
      const Format format,
      const bool isIMPs = true,
      Board * refBoard = nullptr) const;
    
    string strContracts(
      const string& contractFromHeader,
      const Format format) const;

    string strResult(
      const Format format,
      const bool scoringIsIMPs,
      const bool swapFlag = false) const;

    string strResult(
      const Format format,
      const string& team,
      const bool swapFlag = false) const;

    string strIMPSheetLine(
      const string& bno,
      unsigned& score1,
      unsigned& score2) const;
};

#endif

