/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_VALPROF_H
#define BRIDGE_VALPROF_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#include <iostream>
#include <vector>
#pragma warning(pop)

#include "bconst.h"

using namespace std;


enum ValError
{
  // This group of errors covers different or missing headers.
  // These are to expected in conversions, as not all file formats
  // contain the complete information.
  BRIDGE_VAL_TITLE = 0,
  BRIDGE_VAL_DATE = 1,
  BRIDGE_VAL_LOCATION = 2,
  BRIDGE_VAL_EVENT = 3,
  BRIDGE_VAL_SESSION = 4,
  BRIDGE_VAL_PLAYERS_HEADER = 5,
  BRIDGE_VAL_BOARDS_HEADER = 6,
  BRIDGE_VAL_SCORES_HEADER = 7,
  BRIDGE_VAL_SCORING = 8,
  BRIDGE_VAL_TEAMS = 9,
  BRIDGE_VAL_ROOM = 10,
  BRIDGE_VAL_DEAL = 11,
  BRIDGE_VAL_VUL = 12,
  BRIDGE_VAL_AUCTION = 13,
  BRIDGE_VAL_PLAY = 14,
  BRIDGE_VAL_SCORE = 15,
  BRIDGE_VAL_DD = 16,
  BRIDGE_VAL_NAMES_SHORT = 17,
  BRIDGE_VAL_TXT_DASHES = 18,
  BRIDGE_VAL_VG_MD = 19,
  BRIDGE_VAL_VG_CHAT = 20,

  // These include Pavlicek bugs and some not so harmful LIN issues.
  BRIDGE_VAL_TXT_ALL_PASS = 21,
  BRIDGE_VAL_LIN_AH_EXTRA = 22,
  BRIDGE_VAL_LIN_AN_ERROR = 23,
  BRIDGE_VAL_LIN_AN_EXTRA = 24,
  BRIDGE_VAL_LIN_MC_EXTRA = 25,
  BRIDGE_VAL_LIN_MD = 26,
  BRIDGE_VAL_LIN_PC_ROTATED = 27,
  BRIDGE_VAL_LIN_PN_EXTRA = 28,
  BRIDGE_VAL_LIN_PN_MISSING = 29,
  BRIDGE_VAL_LIN_QX = 30,
  BRIDGE_VAL_LIN_RH_EXTRA = 31,
  BRIDGE_VAL_LIN_ST_EXTRA = 32,
  BRIDGE_VAL_LIN_ST_MISSING = 33,
  BRIDGE_VAL_LIN_SV_MISSING = 34,

  BRIDGE_VAL_LIN_PLAY_NL = 35,
  BRIDGE_VAL_PLAY_SHORT = 36,
  BRIDGE_VAL_REC_MADE_32 = 37,
  BRIDGE_VAL_TXT_RESULT = 38,
  BRIDGE_VAL_RECORD_NUMBER = 39,

  // These are real errors.
  BRIDGE_VAL_ERROR = 40,
  BRIDGE_VAL_OUT_SHORT = 41,
  BRIDGE_VAL_REF_SHORT = 42,
  BRIDGE_VAL_VG_MC = 43,
  BRIDGE_VAL_SIZE = 44
};

struct ValErrorBundle
{
  ValError valError;
  string nameLong;
  string nameShort;
};

const vector<ValErrorBundle> ValErrors =
{
  {BRIDGE_VAL_TITLE, "Title", "T"},
  {BRIDGE_VAL_DATE, "Date", "D"},
  {BRIDGE_VAL_LOCATION, "Location", "L"},
  {BRIDGE_VAL_EVENT, "Event", "E"},
  {BRIDGE_VAL_SESSION, "Session", "S"},
  {BRIDGE_VAL_PLAYERS_HEADER, "Players list", "Plist"},
  {BRIDGE_VAL_BOARDS_HEADER, "Board numbers", "Bnos"},
  {BRIDGE_VAL_SCORES_HEADER, "Scores list", "List"},
  {BRIDGE_VAL_SCORING, "Scoring", "F"},
  {BRIDGE_VAL_TEAMS, "Teams", "K"},
  {BRIDGE_VAL_ROOM, "Room", "Room"},
  {BRIDGE_VAL_DEAL, "Deal", "Deal"},
  {BRIDGE_VAL_VUL, "Vul", "Vul"},
  {BRIDGE_VAL_AUCTION, "Auction", "Auct"},
  {BRIDGE_VAL_PLAY, "Play", "Play"},
  {BRIDGE_VAL_SCORE, "Score", "Scor"},
  {BRIDGE_VAL_DD, "Double-dummy", "DD"},
  {BRIDGE_VAL_NAMES_SHORT, "Names-short", "Nsht"},
  {BRIDGE_VAL_TXT_DASHES, "TXT-dashes", "Dash"},
  {BRIDGE_VAL_VG_MD, "VG-cards", "Hlen"},
  {BRIDGE_VAL_VG_CHAT, "VG-chat", "Chat"},

  {BRIDGE_VAL_TXT_ALL_PASS, "All-pass", "Apass"},
  {BRIDGE_VAL_LIN_AH_EXTRA, "Lin-ah+", "ah+"},
  {BRIDGE_VAL_LIN_AN_ERROR, "Lin-an", "an"},
  {BRIDGE_VAL_LIN_AN_EXTRA, "Lin-an+", "an+"},
  {BRIDGE_VAL_LIN_MC_EXTRA, "Lin-mc+", "mc+"},
  {BRIDGE_VAL_LIN_MD, "Lin-md", "md"},
  {BRIDGE_VAL_LIN_PC_ROTATED, "Lin-pc rot", "pcrot"},
  {BRIDGE_VAL_LIN_PN_EXTRA, "Lin-pn+", "pn+"},
  {BRIDGE_VAL_LIN_PN_MISSING, "Lin-pn-", "pn-"},
  {BRIDGE_VAL_LIN_QX, "Lin-qx", "qx"},
  {BRIDGE_VAL_LIN_RH_EXTRA, "Lin-rh+", "rh+"},
  {BRIDGE_VAL_LIN_ST_EXTRA, "Lin-st+", "st+"},
  {BRIDGE_VAL_LIN_ST_MISSING, "Lin-st-", "st-"},
  {BRIDGE_VAL_LIN_SV_MISSING, "Lin-sv-", "sv-"},

  {BRIDGE_VAL_LIN_PLAY_NL, "Play-newline", "Pline"},
  {BRIDGE_VAL_PLAY_SHORT, "Play-short", "Psht"},
  {BRIDGE_VAL_REC_MADE_32, "Made-32", "R32"},
  {BRIDGE_VAL_TXT_RESULT, "TXT-result", "RTXT"},
  {BRIDGE_VAL_RECORD_NUMBER, "Rec-comment", "Comm"},

  {BRIDGE_VAL_ERROR, "Error", "Error"},
  {BRIDGE_VAL_OUT_SHORT, "Out-short", "Osht"},
  {BRIDGE_VAL_REF_SHORT, "Ref-short", "Rsht"},
  {BRIDGE_VAL_VG_MC, "VG-mc", "mc"},
  {BRIDGE_VAL_SIZE, "", ""},

};


class ValProfile 
{
  private:

    struct ValSide
    {
      string line;
      unsigned lno;
    };

    struct ValExample
    {
      ValSide out;
      ValSide ref;
    };

    vector<ValExample> example;
    vector<unsigned> count;


  public:

    ValProfile();

    ~ValProfile();

    void reset();

    void log(
      const ValError label,
      const LineData& dataOut,
      const LineData& dataRef);

    bool labelIsSet(const unsigned label) const;

    bool hasError(const bool minorFlag = false) const;

    unsigned getCount(const unsigned label) const;

    void operator += (const ValProfile& prof2);

    void addRange(
      const ValProfile& prof,
      const unsigned lower,
      const unsigned upper,
      bool& flag);

    void print(
      ostream& fstr = cout,
      const bool minorFlag = false) const;
};

#endif

