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
      const Format format);

    void setDate(
      const string& d,
      const Format format);

    void setLocation(
      const string& l,
      const Format format);

    void setEvent(
      const string& e,
      const Format format);

    void setSession(
      const string& s,
      const Format format);

    void setScoring(
      const string& s,
      const Format format);

    void setTeams(
      const string& s,
      const Format format);

    void setFirstTeam(
      const string& s,
      const Format format);

    void setSecondTeam(
      const string& s,
      const Format format);

    void setPlayers(
      const string& s,
      const Format format);

    void setWest(
      const string& s,
      const Format format);

    void setNorth(
      const string& s,
      const Format format);

    void setEast(
      const string& s,
      const Format format);

    void setSouth(
      const string& s,
      const Format format);

    void setResultsList(
      const string& s,
      const Format format);

    void setPlayersList(
      const string& s,
      const Format format);

    void SetFromHeader(
      const string& room);

    void setPlayersHeader(
      const string& s,
      const Format format);

    void setScoresList(
      const string& s,
      const Format format);

    void setBoardsList(
      const string& s,
      const Format format);

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
      const Format format) const;

    string DateAsString(
      const Format format) const;

    string LocationAsString(
      const Format format) const;

    string EventAsString(
      const Format format) const;

    string SessionAsString(
      const Format format) const;

    string ScoringAsString(
      const Format format) const;

     string TeamsAsString(
      const Format format) const;

     string TeamsAsString(
      const int score1,
      const int score2,
      const Format format) const;

     string FirstTeamAsString(
      const Format format) const;

     string SecondTeamAsString(
      const Format format) const;

     string NumberAsString(
      const Format format,
      const unsigned intNo) const;

     string NumberAsBoardString(
      const Format format,
      const unsigned intNo) const;

     string ContractsAsLIN(
      const Format format);

     string ContractsAsString(
      const Format format);

     string PlayersAsString(
      const Format format);

     string ScoresAsString(
      const Format format) const;

     string BoardsAsString(
      const Format format) const;
};

#endif

