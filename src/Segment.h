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

    vector<LINdataType> LINdata;
    unsigned LINcount;


    bool SetTitleLIN(const string& t);

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
      const string& s,
      const formatType f);

    bool SetFirstTeam(
      const string& s,
      const formatType f);

    bool SetSecondTeam(
      const string& s,
      const formatType f);

    bool SetPlayers(
      const string& s,
      const formatType f);

    bool SetWest(
      const string& s,
      const formatType f);

    bool SetNorth(
      const string& s,
      const formatType f);

    bool SetEast(
      const string& s,
      const formatType f);

    bool SetSouth(
      const string& s,
      const formatType f);

    bool SetResultsList(
      const string& s,
      const formatType f);

    bool SetPlayersList(
      const string& s,
      const formatType f);

    void SetFromHeader(
      const string& room);

    bool SetPlayersHeader(
      const string& s,
      const formatType f);

    bool SetScoresList(
      const string& s,
      const formatType f);

    bool SetBoardsList(
      const string& s,
      const formatType f);

    void CopyPlayers();

    bool SetRoom(
      const string& s,
      const formatType f);

    bool SetNumber(
      const string& s,
      const formatType f);

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

