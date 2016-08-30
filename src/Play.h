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


class Play
{
  private:

    bool setFlag;
    int value;


  public:

    Play();

    ~Play();

    void Reset();

    bool SetContract(
      const Contract& contract);
    
    bool SetDeclAndDenom(
      const playerType decl,
      const denomType denom);
    
    bool AddPlay(
      const string& str);
    
    bool AddPlay(
      const string& str,
      const Deal& deal);
    
    bool AddPlays(
      const string& str);
    
    bool AddPlays(
      const string& str,
      const Deal& deal);
    
    bool AddPlaysAbsolute(
      const string& str);
    
    bool AddPlaysAbsolute(
      const string& str,
      const Deal& deal);

    bool UndoPlay();

    bool PlayIsOver();

    bool Claim(
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

