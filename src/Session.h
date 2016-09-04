/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SESSION_H
#define BRIDGE_SESSION_H

#include "bconst.h"
#include <string>

using namespace std;


// Private to Session, but needs to be here...

enum stageType
{
  BRIDGE_SESSION_FINAL = 0,
  BRIDGE_SESSION_PLAYOFF = 1,
  BRIDGE_SESSION_SEMIFINAL = 2,
  BRIDGE_SESSION_QUARTERFINAL = 3,
  BRIDGE_SESSION_ROUND_OF = 4,
  BRIDGE_SESSION_INITIAL = 5,
  BRIDGE_SESSION_UNDEFINED = 6
};


class Session
{
  private:

    string general;
    stageType stage;
    unsigned roundOf;
    unsigned sessionNo;

    stageType CharToType(const char c) const;

    stageType StringToType(
      const string& t,
      unsigned& rOf) const;


  public:

    Session();

    ~Session();

    void Reset();

    bool Set(
      const string& t,
      const formatType f);

    bool IsRBNPart(const string& t) const;

    bool operator == (const Session& s2) const;

    bool operator != (const Session& s2) const;

    string AsString(const formatType f) const;
};

#endif

