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
    bool setVulFlag;
    bool setResultFlag;

    vulType vul;
    contractType contract;
    int tricksRelative;
    int score;

    void SetTables();

    int ConvertDiffToIMPs(const int d) const;

    string DeclarerAsPBN() const;

    string VulAsRBN() const;

    string TricksAsPBN() const;

    string ScoreAsPBN() const;
    string ScoreAsPBN(const int refScore) const;
    string ScoreAsEML() const;
    string ScoreAsEML(const int refScore) const;
    string ScoreAsTXT() const;

    string ResultAsStringPBN() const;
    string ResultAsStringRBNCore() const;
    string ResultAsStringRBN() const;
    string ResultAsStringRBX() const;
    string ResultAsStringRBNCore(const int refScore) const;
    string ResultAsStringRBN(const int refScore) const;
    string ResultAsStringRBX(const int refScore) const;
    string ResultAsStringEML() const;
    string ResultAsStringTXT() const;
    string ResultAsStringTXT(
      const int refScore,
      const string& team) const;

    string AsLIN() const;
    string AsPBN() const;
    string AsRBNCore() const;
    string AsRBN() const;
    string AsRBX() const;
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

    bool SetContract(const string& text);

    bool SetContract(
      const string& text,
      const formatType f);

    bool SetDeclarer(
      const string& text,
      const formatType f);

    bool SetVul(const vulType vul);

    bool SetTricks(
      const unsigned tricks);

    unsigned GetTricks() const;

    bool SetResult(
      const string& text,
      const formatType f);

    bool SetScore(
      const string& text,
      const formatType f);

    bool IsPassedOut() const;	

    int GetScore() const;

    bool operator == (const Contract& c2) const;

    bool operator != (const Contract& c2) const;

    playerType GetDeclarer() const;

    denomType GetDenom() const;

    void CalculateScore();

    string AsString(
      const formatType f = BRIDGE_FORMAT_LIN) const;

    string DeclarerAsString(const formatType f) const;

    string VulAsString(const formatType f) const;

    string TricksAsString(const formatType f) const;

    string ScoreAsString(const formatType f) const;

    int ScoreIMPAsInt(const int refSCore) const;

    string ScoreAsString(
      const formatType f,
      const int refScore) const;

    string ResultAsString(
      const formatType f) const;

    string ResultAsString(
      const formatType f,
      const int refScore) const;

    string ResultAsString(
      const formatType f,
      const int refScore,
      const string& team) const;
};

#endif

