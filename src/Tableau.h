/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TABLEAU_H
#define BRIDGE_TABLEAU_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <list>

using namespace std;

#include "bconst.h"
#include "Contract.h"


class Tableau
{
  private:

    struct listType
    {
      int score;
      unsigned dno;
      unsigned no;
      unsigned tricks;
      unsigned down;
    };

    struct dataType
    {
      int primacy;
      unsigned highestMakingNo;
      unsigned dearestMakingNo;
      int dearestScore;
      unsigned vulNo;
    };

    unsigned setNum;
    unsigned table[BRIDGE_DENOMS][BRIDGE_PLAYERS];


    string ToRBN() const;

    bool SetRBNPlayer(
      const string text,
      const playerType p);
    
    bool RBNStringToList(
      const string text,
      unsigned listRBN[],
      const bool invertFlag = false);

    bool SetRBNPlayer(
      const unsigned listRBN[],
      const playerType p);

    unsigned ToRBNPlayer(const playerType p) const;

    void SurveyScores(
      const playerType dealer,
      const unsigned * vulBySide,
      dataType& data,
      unsigned& numCand,
      listType list[][BRIDGE_DENOMS]) const;

    void BestSacrifice(
      const unsigned side,
      const unsigned no,
      const unsigned dno,
      const playerType dealer,
      const listType slist[][BRIDGE_DENOMS],
      unsigned sacrTable[][BRIDGE_DENOMS],
      unsigned& bestDown) const;

    void ReduceContract(
      unsigned& no,
      const int sacGap,
      unsigned& plus) const;
    
    bool AddSacrifices(
      const vulType v,
      const unsigned side,
      const playerType dealer,
      const int bestDown,
      const unsigned noDecl,
      const unsigned dno,
      const listType slist[][BRIDGE_DENOMS],
      const unsigned sacr[][BRIDGE_DENOMS],
      list<Contract>& clist) const;

    bool AddContract(
      const vulType v,
      const unsigned side,
      const unsigned no,
      const denomType dno,
      const int delta,
      list<Contract>& clist) const;

    bool AddSpecialSac(
      const vulType v,
      const unsigned no,
      const denomType denom,
      const playerType sacker,
      const int delta,
      list<Contract>& clist) const;


  public:

    Tableau();

    ~Tableau();

    void Reset();

    bool TableIsSet() const;

    bool SetRBN(const string text);
    
    bool SetEntry(
      const playerType p,
      const denomType d,
      const unsigned t);

    unsigned GetEntry(
      const playerType p,
      const denomType d) const;
    
    bool operator ==(const Tableau& tab2) const;

    bool operator !=(const Tableau& tab2) const;

    string ToString(
      formatType f = BRIDGE_FORMAT_LIN) const;
    
    bool GetPar(
      playerType dealer,
      vulType v,
      string& text) const;

    bool GetPar(
      playerType dealer,
      vulType v,
      list<Contract>& clist) const;
};

#endif

