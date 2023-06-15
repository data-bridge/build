/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TABLEAU_H
#define BRIDGE_TABLEAU_H


#include <string>
#include <list>

#include "../bconst.h"

using namespace std;


class Contract;


class Tableau
{
  private:

    struct ListTableau
    {
      int score;
      unsigned dno;
      unsigned no;
      unsigned tricks;
      unsigned down;
    };

    struct DataTableau
    {
      int primacy;
      unsigned highestMakingNo;
      unsigned dearestMakingNo;
      int dearestScore;
      unsigned vulNo;
    };

    unsigned setNum;
    unsigned table[BRIDGE_DENOMS][BRIDGE_PLAYERS];


    string strPBN() const;
    string strRBN() const;
    string strTXT() const;

    bool setRBNPlayer(
      const string text,
      const Player player);
    
    bool RBNStringToList(
      const string text,
      unsigned listRBN[],
      const bool invertFlag = false);

    bool setRBNPlayer(
      const unsigned listRBN[],
      const Player player);

    bool setRBN(const string& text);
    bool setPBN(const string& text);
    
    unsigned toRBNPlayer(const Player player) const;

    void surveyScores(
      const Player dealer,
      const unsigned * vulBySide,
      DataTableau& data,
      unsigned& numCand,
      ListTableau list[][BRIDGE_DENOMS]) const;

    void bestSacrifice(
      const unsigned side,
      const unsigned no,
      const unsigned dno,
      const Player dealer,
      const ListTableau slist[][BRIDGE_DENOMS],
      unsigned sacrTable[][BRIDGE_DENOMS],
      unsigned& bestDown) const;

    void reduceContract(
      unsigned& no,
      const int sacGap,
      unsigned& plus) const;
    
    bool addSacrifices(
      const Vul vul,
      const unsigned side,
      const Player dealer,
      const int bestDown,
      const unsigned noDecl,
      const unsigned dno,
      const ListTableau slist[][BRIDGE_DENOMS],
      const unsigned sacr[][BRIDGE_DENOMS],
      list<Contract>& clist) const;

    bool addContract(
      const Vul vul,
      const unsigned side,
      const unsigned no,
      const Denom dno,
      const int delta,
      list<Contract>& clist) const;

    bool addSpecialSac(
      const Vul vul,
      const unsigned no,
      const Denom denom,
      const Player sacker,
      const int delta,
      list<Contract>& clist) const;


  public:

    Tableau();

    void reset();

    bool isComplete() const;

    bool set(
      const string& text,
      const Format format);
    
    bool set(
      const Player player,
      const Denom denom,
      const unsigned t);

    void setDDS(const int resDDS[5][4]);

    unsigned get(
      const Player player,
      const Denom denom) const;
    
    bool operator == (const Tableau& tableau2) const;

    bool operator != (const Tableau& tableau2) const;

    string str(const Format format = BRIDGE_FORMAT_LIN) const;
    
    bool getPar(
      const Player dealer,
      const Vul vul,
      string& text) const;

    bool getPar(
      const Player dealer,
      const Vul vul,
      list<Contract>& clist) const;
};

#endif

