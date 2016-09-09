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

    string CarryAsString(const teamType& tt) const;

    string AsLIN() const;
    string AsPBN() const;
    string AsRBN() const;
    string AsTXT() const;


  public:

    Teams();

    ~Teams();

    void Reset();

    bool Set(
      const string list[],
      const formatType f);
      
    bool operator == (const Teams& t2) const;

    bool operator != (const Teams& t2) const;

    string AsString(const formatType f) const;
};

#endif

