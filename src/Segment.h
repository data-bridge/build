/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SEGMENT_H
#define BRIDGE_SEGMENT_H

#include "Board.h"
#include "Date.h"
#include "Team.h"
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

    enum scoringType
    {
      BRIDGE_SCORING_IMPS = 0,
      BRIDGE_SCORING_BAM = 1,
      BRIDGE_SCORING_TOTAL = 2,
      BRIDGE_SCORING_CROSS_IMPS = 3,
      BRIDGE_SCORING_MATCHPOINTS = 4,
      BRIDGE_SCORING_INSTANT = 5,
      BRIDGE_SCORING_RUBBER = 6,
      BRIDGE_SCORING_CHICAGO = 7,
      BRIDGE_SCORING_CAVENDISH = 8,
      BRIDGE_SCORING_UNDEFINED = 9
    };

    struct sdata
    {
      string title;
      Date date;
      string location;
      string event;
      string session;
      scoringType scoring;
      Team team1;
      Team team2;
    };

    struct boardPairType
    {
      unsigned extNo;
      Board board;
      
    };

    sdata seg;

    unsigned len;
    vector<boardPairType> boards;
    bool firstStringFlag;

    bool SetTitleLIN(const string t[]);
    bool SetTitlePBN(const string& t);
    bool SetTitleRBN(const string& t);
    bool SetTitleTXT(const string t[]);

    bool SetEventPBN(const string& t);
    bool SetEventRBN(const string& t);
    bool SetEventTXT(const string& t);

    bool SetSessionPBN(const string& t);
    bool SetSessionRBN(const string& t);
    bool SetSessionTXT(const string& t);

    bool SetScoringLIN(const string& t);
    bool SetScoringPBN(const string& t);
    bool SetScoringRBN(const string& t);
    bool SetScoringTXT(const string& t);

    string TitleAsLIN() const;
    string TitleAsPBN() const;
    string TitleAsRBN() const;
    string TitleAsTXT() const;

    string EventAsPBN() const;
    string EventAsRBN() const;
    string EventAsTXT() const;

    string SessionAsPBN() const;
    string SessionAsRBN() const;
    string SessionAsTXT() const;

    string ScoringAsPBN() const;
    string ScoringAsRBN() const;
    string ScoringAsTXT() const;


  public:

    Segment();

    ~Segment();

    void Reset();

    bool MakeBoard(const unsigned no);

    Board * GetBoard(const unsigned no);

    bool SetTitle(
      const string t[],
      const formatType f);

    bool SetDate(
      const string& d,
      const formatType f);

    bool SetLocation(const string& l);

    bool SetEvent(const string& e);

    bool SetSession(
      const string& s,
      const formatType f);

    bool SetScoring(
      const string& s,
      const formatType f);

    bool SetTeams(
      const string list[],
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

