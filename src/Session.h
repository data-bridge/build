/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SESSION_H
#define BRIDGE_SESSION_H

#include <string>
#include "bconst.h"

using namespace std;


// Private to Session, but needs to be here...

enum StageType
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

    string general1;
    StageType stage;
    unsigned roundOf;
    string extension;

    string general2;
    unsigned sessionNo;
    string sessExt;

    StageType charToType(const char c) const;

    StageType stringToType(
      const string& t,
      unsigned& rOf) const;

    void setPart1(const string& t);
    void setPart2(const string& t);

    string asLIN() const;
    string asLIN_RP() const;
    string asPBN() const;
    string asRBNCore() const;
    string asRBN() const;
    string asRBX() const;
    string asTXT() const;


  public:

    Session();

    ~Session();

    void reset();

    void set(
      const string& t,
      const formatType f);

    bool isRBNPart(const string& t) const;

    bool operator == (const Session& s2) const;

    bool operator != (const Session& s2) const;

    string asString(const formatType f) const;
};

#endif

