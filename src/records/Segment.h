/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SEGMENT_H
#define BRIDGE_SEGMENT_H

#include <string>
#include <list>

#include "Date.h"
#include "Location.h"
#include "Session.h"
#include "Teams.h"
#include "Scoring.h"
#include "HeaderLIN.h"
#include "Board.h"

enum Format: unsigned;

using namespace std;


struct BoardPair
{
  unsigned intNo; // Internal Segment number
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
    HeaderLIN headerLIN;

    bool flagCOCO; // Team order is swapped in the input

    unsigned len;
    list<BoardPair> boards;
    unsigned bmin;
    unsigned bmax;

    unsigned getIntBoardNo(const unsigned extNo) const;
    unsigned getExtBoardNo(const unsigned intNo) const;

    unsigned getLINActiveNo(const unsigned intNo) const;

    void equalHeader(const Segment& segment2) const;

    string strTitleLINCore() const;
    string strTitleLIN() const;
    string strTitleLIN_RP() const;
    string strTitleLIN_VG() const;

    void setTitleLIN(
      const string& text,
      const Format format);

    string strContractsCore(const Format format) const;

    string strPlayersLIN() const;


  public:

    Segment();

    list<BoardPair>::const_iterator begin() const { return boards.begin(); }
    list<BoardPair>::const_iterator end() const { return boards.end(); }

    list<BoardPair>::iterator mbegin() { return boards.begin(); }
    list<BoardPair>::iterator mend() { return boards.end(); }

    void reset();

    Board const * getBoard(const unsigned extNo) const;

    Board * acquireBoard(const unsigned extNo);

    void setBoard(const unsigned extNo);

    void setCOCO(const Format format = BRIDGE_FORMAT_SIZE);
    bool getCOCO() const;

    unsigned size() const;
    unsigned count() const;
    unsigned countBoards() const;

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

    void setResultsList(
      const string& text,
      const Format format);

    void setPlayersList(
      const string& text,
      const Format format);

    void setPlayersHeader(
      const string& text,
      const Format format);

    void setScoresList(
      const string& text,
      const Format format);

    void setBoardsList(
      const string& text,
      const Format format);

    void setRoom(
      const string& text,
      const Format format);

    void setNumber(
      const string& text,
      const Format format);

    unsigned firstBoardNumber() const;
    unsigned lastRealBoardNumber() const;

    bool scoringIsIMPs() const;

    bool hasCarry() const;

    void getCarry(
      unsigned& score1,
      unsigned& score2) const;

    bool operator == (const Segment& s2) const;
    bool operator != (const Segment& s2) const;
    bool operator <= (const Segment& s2) const;

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
      const Format format,
      const bool swapFlag = false) const;
    string strFirstTeam(
      const Format format,
      const bool swapFlag = false) const;
    string strSecondTeam(
      const Format format,
      const bool swapFlag = false) const;

    string strNumber(
      const unsigned extNo,
      const Format format) const;

    string strContracts(const Format format) const;
    string strPlayers(const Format format) const;
    string strScores(const Format format) const;
    string strBoards(const Format format) const;

    string strIMPSheetHeader() const;
    string strIMPSheetFooter(
      const unsigned score1,
      const unsigned score2) const;
};

#endif

