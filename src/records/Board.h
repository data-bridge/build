/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BOARD_H
#define BRIDGE_BOARD_H

#include <vector>
#include <string>

#include "Deal.h"
#include "Tableau.h"
#include "Instance.h"
#include "GivenScore.h"

#include "../analysis/Valuation.h"

enum Format: unsigned;

using namespace std;


class Board
{
  private:

    Deal deal;
    Tableau tableau;
    vector<Valuation> valuation;
    vector<Instance> instances;
    vector<bool> skip;
    GivenScore givenScore;

    unsigned len;

    string strIMPEntry(const int imps) const;
    

  public:

    Board();

    void reset();

    Instance * acquireInstance(const unsigned instNo = 0);

    const Instance& getInstance(const unsigned instNo) const;

    bool skipped(const unsigned no) const;
    bool skippedAll() const;

    void markUsed(const unsigned instNo);

    unsigned count() const;
    unsigned countAll() const;

    void setLINheader(LINData const * lin);

    void setDeal(
      const string& text,
      const Format format);
      
    void setDealer(
      const string& text,
      const Format format);

    void setVul(
      const string& text,
      const Format format);

    Player holdsCard(const string& text) const;

    void setScoreIMP(
      const string& text,
      const Format format);

    void setScoreMP(
      const string& text,
      const Format format);

    void calculateScore();

    void setTableau(
      const string& text,
      const Format format);

    void setTableauDDS(const int res[5][4]);

    bool setTableauEntry(
      const Player player,
      const Denom denom,
      const unsigned tricks);

    unsigned getTableauEntry(
      const Player player,
      const Denom denom) const;

    Player getDealer() const;

    const vector<Valuation>& getValuations() const;

    bool getPar(
      Player dealer,
      Vul vul,
      string& text) const;

    bool getPar(
      Player dealer,
      Vul vul,
      list<Contract>& text) const;

    void copyPlayers(const Board& board2);

    bool overlappingPlayers() const;

    void performValuation(const bool fullFlag = false);

    bool operator == (const Board& b2) const;
    bool operator != (const Board& b2) const;
    bool operator <= (const Board& b2) const;

    string strDeal(const Format format) const;
    string strDeal(
      const Player player,
      const Format format) const;

    string strTableau(const Format format) const;

    string strContract(
      const unsigned instNo,
      const Format format) const;

    string strGivenScore(const Format format) const;

    string strPlayersBoard(
      const Format format,
      const bool isIMPs = true,
      Board const * refBoard = nullptr) const;
    
    string strScore(
      const unsigned instNo,
      const Format format) const;

    string strScoreIMP(
      const unsigned instNo,
      const Format format) const;

    string strResult(
      const unsigned instNo,
      const Format format) const;

    string strResult(
      const unsigned instNo,
      const string& team,
      const Format format) const;

    int IMPScore(const unsigned instNo) const;

    string strIMPSheetLine(
      const string& bno,
      unsigned& score1,
      unsigned& score2) const;
    
    string strValuation() const;

    int hash8() const;
    int hash12() const;
};

#endif

