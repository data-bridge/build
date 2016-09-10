/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CONTRACT_H
#define BRIDGE_CONTRACT_H

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <list>

using namespace std;

#include "bconst.h"
#include "Contract.h"


struct contractType
{
  playerType declarer;
  unsigned level;
  denomType denom;
  multiplierType mult;
};


class Contract
{
  private:

    bool setContractFlag;
    bool setResultFlag;

    vulType vul;
    contractType contract;
    int tricksRelative;
    int score;

    void SetTables();

    void CalculateScore();

    string DeclarerAsPBN() const;

    string VulAsRBN() const;

    string TricksAsPBN() const;

    string ScoreAsPBN() const;
    string ScoreAsTXT() const;

    string AsLIN() const;
    string AsPBN() const;
    string AsRBN() const;
    string AsTXT() const;
    string AsPar() const;


  public:

    Contract();

    ~Contract();

    void Reset();

    bool ContractIsSet() const;

    bool ResultIsSet() const;

    bool SetPassedOut();

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

    bool SetResult(
      const string& text,
      const formatType f);

    bool IsPassedOut() const;	

    bool operator == (const Contract& c2) const;

    bool operator != (const Contract& c2) const;

    playerType GetDeclarer() const;

    denomType GetDenom() const;

    string AsString(
      const formatType f = BRIDGE_FORMAT_LIN) const;

    string DeclarerAsString(const formatType f) const;

    string VulAsString(const formatType f) const;

    string TricksAsString(const formatType f) const;

    string ScoreAsString(const formatType f) const;
};

#endif

