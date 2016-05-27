/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_PLAYERS_H
#define BRIDGE_PLAYERS_H

#include "bconst.h"
#include <string>

using namespace std;


class Players
{
  private:

    bool setFlag;
    int value;


  public:

    Players();

    ~Players();

    void Reset();

    bool IsSet() const;

    bool Set(const int v2);

    string Get(playerType p) const;

    bool operator == (const Players& p2);

    bool operator != (const Players& p2);
};

#endif

