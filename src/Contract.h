/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CONTRACT_H
#define BRIDGE_CONTRACT_H

#include <string>

#include "bconst.h"

using namespace std;


struct ContractInternal
{
  Player declarer;
  unsigned level;
  Denom denom;
  Multiplier mult;
};


class Contract
{
  private:

    bool setContractFlag;
    bool setVulFlag;
    bool setResultFlag;

    Vul vul;
    ContractInternal contract;
    int tricksRelative;
    int score;

    void setTables();

    void setContractByString(const string& text);

    void setContractTXT(const string& text);

    void textToTricks(const string& text);

    int diffToIMPs(const int d) const;

    void setResultTXT(const string& text);
    void setResultEML(const string& text);

    string strLIN(const Format format) const;
    string strPBN() const;
    string strRBNCore() const;
    string strRBN() const;
    string strRBX() const;
    string strTXT() const;
    string strEML() const;
    string strPar() const;

    string strScorePBN() const;
    string strScorePBN(const int refScore) const;
    string strScoreEML() const;
    string strScoreEML(const int refScore) const;
    string strScoreTXT() const;
    string strScoreREC() const;

    string strResultPBN() const;
    string strResultRBNCore() const;
    string strResultRBN() const;
    string strResultRBX() const;
    string strResultRBNCore(const int refScore) const;
    string strResultRBN(const int refScore) const;
    string strResultRBX(const int refScore) const;
    string strResultEML() const;
    string strResultTXT() const;
    string strResultTXT(
      const int refScore,
      const string& team) const;
    string strResultREC() const;


  public:

    Contract();

    ~Contract();

    void reset();

    bool isSet() const;

    bool hasResult() const;

    void passOut();

    void setContract(
      const Vul vul,
      const Player declarer,
      const unsigned level,
      const Denom denom,
      const Multiplier mult);

    void setContract(
      const Vul vul,
      const string& cstring);

    void setContract(
      const string& text,
      const Format format);

    void setDeclarer(const string& text);

    void setVul(const Vul vul);

    bool isPassedOut() const;	

    void setTricks(const unsigned tricks);

    unsigned getTricks() const;

    void setResult(
      const string& text,
      const Format format);

    void setScore(
      const string& text,
      const Format format);

    void calculateScore();

    int getScore() const;

    Player getDeclarer() const;

    Vul getVul() const;

    Denom getDenom() const;

    bool operator == (const Contract& c2) const;

    bool operator != (const Contract& c2) const;

    string str(const Format format) const;
    string strDeclarer(const Format format) const;
    string strDenom(const Format format) const;
    string strVul(const Format format) const;
    string strTricks(const Format format) const;

    string strScore(
      const Format format) const;

    string strScore(
      const Format format,
      const int refScore) const;

    string strScoreIMP(
      const Format format,
      const int refScore) const;

    string strResult(
      const Format format) const;

    string strResult(
      const Format format,
      const int refScore) const;

    string strResult(
      const Format format,
      const int refScore,
      const string& team) const;

    string strDiffTag(const Contract& c2) const;

    int IMPScore(const int refSCore) const;
};

#endif

