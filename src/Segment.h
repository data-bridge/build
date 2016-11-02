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

    Board * activeBoard;
    unsigned activeNo;

    vector<LINData> LINdata;
    unsigned LINcount;


    void SetTitleLIN(const string& text);

    string TitleAsLINCommon(const bool swapFlag = false) const;
    string TitleAsLIN() const;
    string TitleAsLIN_RP() const;
    string TitleAsLIN_VG() const;
    string TitleAsLIN_TRN() const;
    string TitleAsLIN_EXT() const;


  public:

    Segment();

    ~Segment();

    void Reset();

    Board * GetBoard(const unsigned no);

    Board * AcquireBoard(const unsigned no);

    unsigned GetExtBoardNo(const unsigned no) const;

    unsigned GetActiveExtBoardNo() const;

    void TransferHeader(const unsigned no);
    void TransferHeader(
      const unsigned intNo,
      const unsigned instNo);

    unsigned GetLength() const;

    unsigned count() const;

    void setTitle(
      const string& t,
      const formatType f);

    void setDate(
      const string& d,
      const formatType f);

    void setLocation(
      const string& l,
      const formatType f);

    void setEvent(
      const string& e,
      const formatType f);

    void setSession(
      const string& s,
      const formatType f);

    void setScoring(
      const string& s,
      const formatType f);

    void setTeams(
      const string& s,
      const formatType f);

    void setFirstTeam(
      const string& s,
      const formatType f);

    void setSecondTeam(
      const string& s,
      const formatType f);

    void setPlayers(
      const string& s,
      const formatType f);

    void setWest(
      const string& s,
      const formatType f);

    void setNorth(
      const string& s,
      const formatType f);

    void setEast(
      const string& s,
      const formatType f);

    void setSouth(
      const string& s,
      const formatType f);

    void setResultsList(
      const string& s,
      const formatType f);

    void setPlayersList(
      const string& s,
      const formatType f);

    void SetFromHeader(
      const string& room);

    void setPlayersHeader(
      const string& s,
      const formatType f);

    void setScoresList(
      const string& s,
      const formatType f);

    void setBoardsList(
      const string& s,
      const formatType f);

    void CopyPlayers();

    void setRoom(
      const string& text,
      const Format format);

    void setNumber(
      const string& text,
      const Format format);

    bool ScoringIsIMPs() const;

    bool CarryExists() const;

    bool operator == (const Segment& s2) const;

    bool operator != (const Segment& s2) const;

    string TitleAsString(
      const formatType f) const;

    string DateAsString(
      const formatType f) const;

    string LocationAsString(
      const formatType f) const;

    string EventAsString(
      const formatType f) const;

    string SessionAsString(
      const formatType f) const;

    string ScoringAsString(
      const formatType f) const;

     string TeamsAsString(
      const formatType f) const;

     string TeamsAsString(
      const int score1,
      const int score2,
      const formatType f) const;

     string FirstTeamAsString(
      const formatType f) const;

     string SecondTeamAsString(
      const formatType f) const;

     string NumberAsString(
      const formatType f,
      const unsigned intNo) const;

     string NumberAsBoardString(
      const formatType f,
      const unsigned intNo) const;

     string ContractsAsLIN(
      const formatType f);

     string ContractsAsString(
      const formatType f);

     string PlayersAsString(
      const formatType f);

     string ScoresAsString(
      const formatType f) const;

     string BoardsAsString(
      const formatType f) const;
};

#endif

