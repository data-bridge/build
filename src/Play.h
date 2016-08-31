/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PLAY_H
#define BRIDGE_PLAY_H

#include "bconst.h"
#include "Contract.h"
#include "Deal.h"
#include <iostream>
#include <string>

using namespace std;


enum playStatus
{
  PLAY_NO_ERROR = 0,
  PLAY_OVER = 1,
  PLAY_CARD_NOT_HELD = 2,
  PLAY_REVOKE = 3,
  PLAY_CARD_INVALID = 4,
  PLAY_HOLDING_NOT_SET = 5,
  PLAY_INVALID_PBN = 6,
  PLAY_INVALID_RBN = 7
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
      playerType leader;
      denomType suit;
      bool wonByDeclarer;
    };

    bool setDDFlag;
    playerType declarer;
    denomType denom;

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

    string AsLIN() const;
    string AsPBN() const;
    string AsRBN() const;
    string AsTXT() const;

    void SetTables();

    unsigned TrickWinnerRelative() const;

    string ClaimAsLIN() const;


  public:

    Play();

    ~Play();

    void Reset();

    bool SetContract(
      const Contract& contract);
    
    bool SetDeclAndDenom(
      const playerType decl,
      const denomType denom);
    
    bool SetHoldingDDS(
      const unsigned h[][BRIDGE_SUITS]);

    playStatus AddPlay(
      const string& str);
    
    playStatus AddTrickPBN(
      const string& str);
    
    playStatus AddAllRBN(
      const string& str);
    
    bool UndoPlay();

    bool PlayIsOver();

    claimStatus Claim(
      const unsigned tricks);
    
    bool ClaimIsMade() const;

    bool operator == (const Play& p2);

    bool operator != (const Play& p2);

    string AsString(
      const formatType f) const;

    string ClaimAsString(
      const formatType f) const;

};

#endif

