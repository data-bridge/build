/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEAMS_H
#define BRIDGE_TEAMS_H

#include "bconst.h"
#include <string>

using namespace std;


class Teams
{
  private:

    enum carryType
    {
      BRIDGE_CARRY_NONE = 0,
      BRIDGE_CARRY_INT = 1,
      BRIDGE_CARRY_FLOAT = 2
    };

    struct teamType
    {
      string name;
      carryType carry;
      int carryi;
      float carryf;
    };

    teamType team1;
    teamType team2;

    bool ExtractCarry(
      const string& t,
      teamType& tt) const;

    bool SetSinglePBN(
      const string& t,
      teamType& tt) const;

    bool SetSingleTXT(
      const string& t,
      teamType& tt) const;

    bool SetLIN(const string& t);
    bool SetPBN(
      const string& t1,
      const string& t2);
    bool SetRBN(const string& t);
    bool SetTXT(const string& t);

    bool TeamIsEqual(
      const teamType& ta,
      const teamType& tb) const;

    string SingleAsLIN(const teamType& tt) const;
    string SingleAsPBN(const teamType& tt) const;
    string SingleAsTXT(const teamType& tt) const;

    string CarryAsString(
      const teamType& tt,
      const bool forceFlag = false) const;

    string AsLIN(const bool swapFlag) const;
    string AsPBN() const;
    string AsRBNCore(const bool swapFlag) const;
    string AsRBN(const bool swapFlag) const;
    string AsRBX(const bool swapFlag) const;
    string AsTXT(const bool swapFlag) const;
    string AsTXT(
      const int score1,
      const int score2,
      const bool swapFlag) const;


  public:

    Teams();

    ~Teams();

    void Reset();

    bool Set(
      const string& s,
      const formatType f);
      
    bool SetFirst(
      const string& s,
      const formatType f);
      
    bool SetSecond(
      const string& s,
      const formatType f);

    bool CarryExists() const;
      
    bool operator == (const Teams& t2) const;

    bool operator != (const Teams& t2) const;

    string AsString(
      const formatType f,
      const bool swapFlag = false) const;

    string AsString(
      const formatType f,
      const int score1,
      const int score2,
      const bool swapFlag = false) const;

    string FirstAsString(
      const formatType f,
      const bool swapFlag = false) const;

    string SecondAsString(
      const formatType f,
      const bool swapFlag = false) const;

};

#endif

