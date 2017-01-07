/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TEAMS_H
#define BRIDGE_TEAMS_H

#include <string>

#include "Team.h"
#include "bconst.h"

using namespace std;


class Teams
{
  private:

    Team team1;
    Team team2;

    void setPBN(
      const string& text1,
      const string& text2);
    void setRBN(const string& text);
    void setTXT(const string& text);

    string strLIN(const bool swapFlag) const;
    string strRBNCore() const;
    string strRBN() const;
    string strRBX() const;
    string strTXT() const;
    string strTXT(
      const int score1,
      const int score2) const;


  public:

    Teams();

    ~Teams();

    void reset();

    void set(
      const string& text,
      const Format format);
      
    void setFirst(
      const string& text,
      const Format format);
      
    void setSecond(
      const string& text,
      const Format format);
      
    void swap();

    bool hasCarry() const;
      
    bool operator == (const Teams& t2) const;

    bool operator != (const Teams& t2) const;

    string str(
      const Format format,
      const bool swapFlag = false) const;

    string str(
      const int score1,
      const int score2,
      const Format format) const;

    string strFirst(const Format format) const;

    string strSecond(const Format format) const;
};

#endif

