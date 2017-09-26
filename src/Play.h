/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PLAY_H
#define BRIDGE_PLAY_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

#include "bconst.h"
#include "ddsIF.h"

using namespace std;

class Contract;


enum playStatus
{
  PLAY_NO_ERROR = 0,
  PLAY_OVER = 1,
  PLAY_CARD_NOT_HELD = 2,
  PLAY_REVOKE = 3,
  PLAY_CARD_INVALID = 4,
  PLAY_HOLDING_NOT_SET = 5,
  PLAY_INVALID_PBN = 6,
  PLAY_INVALID_RBN = 7,
  PLAY_INVALID_FORMAT = 8
};


enum claimStatus
{
  PLAY_CLAIM_NO_ERROR = 0,
  PLAY_CLAIM_TOO_LOW = 1,
  PLAY_CLAIM_TOO_HIGH = 2,
  PLAY_CLAIM_PLAY_OVER = 3,
  PLAY_CLAIM_ALREADY = 4
};


class Play
{
  private:

    struct LeadInfo
    {
      Player leader;
      Denom suit;
      bool wonByDeclarer;
    };

    bool setDDFlag;
    Player declarer;
    Denom denom;

    bool setDealFlag;
    unsigned holding[BRIDGE_PLAYERS][BRIDGE_SUITS];

    unsigned len;
    unsigned lenMax;
    vector<unsigned> sequence;

    unsigned trickToPlay;
    unsigned cardToPlay;
    bool playOverFlag;
    bool claimMadeFlag;
    unsigned tricksDecl;
    unsigned tricksDef;

    LeadInfo leads[BRIDGE_TRICKS];


    void setTables();

    void setDeclAndDenom(
      const Player decl,
      const Denom denom);
    
    unsigned trickWinnerRelative() const;

    void addPlay(const string& text);

    unsigned getRotationalOffset(const vector<string>& cards) const;

    void addTrickPBN(const string& text); // Currently unused
    
    void setPlaysPBN(const vector<string>& list);

    void setPlaysRBN(
      const string& text,
      const Format format);
    
    string strLIN() const;
    string strLIN_VG() const;
    string strLIN_TRN() const;
    string strLIN_RP() const;
    string strPBN() const;
    string strRBNCore() const;
    string strRBN() const;
    string strRBX() const;
    string strEML() const;
    string strTXT() const;
    string strREC() const;
    string strPAR() const;

    string strClaimLIN() const;


  public:

    Play();

    ~Play();

    void reset();

    void setContract(const Contract& contract);
    
    void setHoldingDDS(const unsigned h[][BRIDGE_SUITS]);

    void setPlays(
      const string& text,
      const Format format);
    
    void undoPlay();

    bool isOver() const;

    bool dealIsSet() const;

    void makeClaim(const unsigned tricks);
    
    bool hasClaim() const;

    unsigned getTricks() const;

    void getStateDDS(RunningDD& runningDD) const;

    void getPlayedBy(vector<Player>& playedBy) const;

    bool operator == (const Play& play2) const;
    bool operator != (const Play& play2) const;
    bool operator <= (const Play& play2) const;

    string str(const Format format) const;

    string strLead(const Format format) const;

    string strDeclarer(const Format format) const;

    string strClaim(const Format format) const;
};

#endif

