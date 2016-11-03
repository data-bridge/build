/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SEGMENT_H
#define BRIDGE_SEGMENT_H

#include <string>

#include "Date.h"
#include "Location.h"
#include "Session.h"
#include "Scoring.h"
#include "Teams.h"
#include "Board.h"
#include "bconst.h"

using namespace std;


struct BoardPair
{
  unsigned no; // Internal Segment number
  unsigned extNo; // Externally seen number
  Board board;
};


class Segment
{
  private:

    string title;
    Date date;
    Location location;
    string event;
    Session session;
    Scoring scoring;
    Teams teams;

    unsigned len;
    vector<BoardPair> boards;
    unsigned bmin;
    unsigned bmax;
    unsigned bInmin;
    unsigned bInmax;

    Board * activeBoard;
    unsigned activeNo;

    vector<LINData> LINdata;
    unsigned LINcount;


    void setTitleLIN(const string& text);

    string TitleAsLINCommon(const bool swapFlag = false) const;
    string TitleAsLIN() const;
    string TitleAsLIN_RP() const;
    string TitleAsLIN_VG() const;
    string TitleAsLIN_TRN() const;
    string TitleAsLIN_EXT() const;


  public:

    Segment();

    ~Segment();

    vector<BoardPair>::iterator begin() { return boards.begin(); }

    vector<BoardPair>::iterator end() { return boards.end(); }

    void reset();

    Board * getBoard(const unsigned intNo);

    Board * acquireBoard(const unsigned intNo);

    void setBoard(const unsigned intNo);

    unsigned getExtBoardNo(const unsigned intNo) const;

    unsigned getActiveExtBoardNo() const;

    void loadFromHeader(
      const unsigned intNo,
      const unsigned instNo);

    unsigned size() const;

    unsigned count() const;

    void setTitle(
      const string& text,
      const Format format);

    void setDate(
      const string& text,
      const Format format);

    void setLocation(
      const string& text,
      const Format format);

    void setEvent(
      const string& text,
      const Format format);

    void setSession(
      const string& text,
      const Format format);

    void setScoring(
      const string& text,
      const Format format);

    void setTeams(
      const string& text,
      const Format format);

    void setFirstTeam(
      const string& text,
      const Format format);

    void setSecondTeam(
      const string& text,
      const Format format);

    void setPlayers(
      const string& text,
      const Format format);

    void setWest(
      const string& text,
      const Format format);

    void setNorth(
      const string& text,
      const Format format);

    void setEast(
      const string& text,
      const Format format);

    void setSouth(
      const string& text,
      const Format format);

    void setResultsList(
      const string& text,
      const Format format);

    void setPlayersList(
      const string& text,
      const Format format);

    void loadSpecificsFromHeader(
      const string& room);

    void setPlayersHeader(
      const string& text,
      const Format format);

    void setScoresList(
      const string& text,
      const Format format);

    void setBoardsList(
      const string& text,
      const Format format);

    void copyPlayers();

    void setRoom(
      const string& text,
      const Format format);

    void setNumber(
      const string& text,
      const Format format);

    bool scoringIsIMPs() const;

    bool hasCarry() const;

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

