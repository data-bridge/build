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

    string strTitleLINCore(const bool swapFlag = false) const;
    string strTitleLIN() const;
    string strTitleLIN_RP() const;
    string strContractsLIN(const Format format);


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

    string strTitle(const Format format) const;
    string strDate(const Format format) const;
    string strLocation(const Format format) const;
    string strEvent(const Format format) const;
    string strSession(const Format format) const;
    string strScoring(const Format format) const;
    string strTeams(const Format format) const;

    string strTeams(
      const int score1,
      const int score2,
      const Format format) const;

    string strFirstTeam(const Format format) const;
    string strSecondTeam(const Format format) const;

    string strNumber(
      const unsigned intNo,
      const Format format) const;

    string strNumberBoard(
      const unsigned intNo,
      const Format format) const;

    string strContracts(const Format format);
    string strPlayers(const Format format);
    string strScores(const Format format) const;
    string strBoards(const Format format) const;
};

#endif

