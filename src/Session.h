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

enum Stage
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
    Stage stage;
    unsigned roundOf;
    string extension;

    string general2;
    unsigned sessionNo;
    string sessExt;

    Stage charToStage(const char c) const;

    Stage stringToStage(
      const string& text,
      unsigned& rOf) const;

    void setPart1(const string& text);
    void setPart2Numeric(
      const string& possNumber,
      const string& text);
    void setPart2Rest(const string& text);
    void setPart2(const string& text);

    void setSeparated(
      const string& text,
      const string& separator);

    string strLIN() const;
    string strLIN_RP() const;
    string strLIN_VG() const;
    string strPBN() const;
    string strRBNCore() const;
    string strRBN() const;
    string strRBX() const;
    string strTXT() const;


  public:

    Session();

    ~Session();

    void reset();

    void set(
      const string& text,
      const Format format);

    bool isStage(const string& text) const;

    bool isSegmentLike(const string& text) const;

    bool isRoundOfLike(const string& text) const;

    bool operator == (const Session& session2) const;

    bool operator != (const Session& session2) const;

    string str(const Format format) const;
};

#endif

