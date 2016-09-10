/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SEGMENT_H
#define BRIDGE_SEGMENT_H

#include "Date.h"
#include "Location.h"
#include "Session.h"
#include "Scoring.h"
#include "Teams.h"
#include "Board.h"
#include "bconst.h"
#include <string>

using namespace std;


enum segOutputType
{
  SEGMENT_FORCE = 0,
  SEGMENT_DELTA = 1
};


class Segment
{
  private:

    struct sdata
    {
      string title;
      Date date;
      Location location;
      string event;
      Session session;
      Scoring scoring;
      Teams teams;
    };

    struct boardPairType
    {
      unsigned no; // Internal Segment number
      unsigned extNo; // Externally seen number
      Board board;
      
    };

    sdata seg;

    unsigned len;
    vector<boardPairType> boards;
    unsigned bmin;
    unsigned bmax;
    unsigned bInmin;
    unsigned bInmax;
    bool firstStringFlag;

    bool SetTitleLIN(const string t);

    string TitleAsLIN() const;


  public:

    Segment();

    ~Segment();

    void Reset();

    bool MakeBoard(const unsigned no);

    Board * GetBoard(const unsigned no);

    bool SetTitle(
      const string& t,
      const formatType f);

    bool SetDate(
      const string& d,
      const formatType f);

    bool SetLocation(
      const string& l,
      const formatType f);

    bool SetEvent(
      const string& e,
      const formatType f);

    bool SetSession(
      const string& s,
      const formatType f);

    bool SetScoring(
      const string& s,
      const formatType f);

    bool SetTeams(
      const string list[],
      const formatType f);

    bool SetTeams(
      const string& s,
      const formatType f);

    bool SetPlayers(
      const string& s,
      const formatType f);

    bool SetNumber(
      const unsigned no,
      const string& extStr);

    bool SetNumber(
      const string& s,
      const formatType f);

    bool operator == (const Segment& s2) const;

    bool operator != (const Segment& s2) const;

    string TitleAsString(
      const formatType f,
      const segOutputType s) const;

    string DateAsString(
      const formatType f,
      const segOutputType s) const;

    string LocationAsString(
      const formatType f,
      const segOutputType s) const;

    string EventAsString(
      const formatType f,
      const segOutputType s) const;

    string SessionAsString(
      const formatType f,
      const segOutputType s) const;

    string ScoringAsString(
      const formatType f,
      const segOutputType s) const;

     string TeamsAsString(
      const formatType f,
      const segOutputType s) const;

};

#endif

