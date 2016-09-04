/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEAM_H
#define BRIDGE_TEAM_H

#include "bconst.h"
#include <string>

using namespace std;


class Team
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

    teamType team;


  public:

    Team();

    ~Team();

    void Reset();

    bool Set(
      const string list[],
      const unsigned teamNo,
      const formatType f);
      
    bool operator == (const Team& t2) const;

    bool operator != (const Team& t2) const;

    string AsString(
      const Team& team2,
      const formatType f) const;
};

#endif

