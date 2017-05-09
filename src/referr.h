/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFERR_H
#define BRIDGE_REFERR_H

#include <string>
#include <vector>

using namespace std;


enum RefControl
{
  ERR_REF_STANDARD = 0,
  ERR_REF_SKIP = 1,
  ERR_REF_NOVAL = 2,
  ERR_REF_OUT_COCO = 3,
  ERR_REF_OUT_OOCC = 4
};


enum FixType
{
  BRIDGE_REF_INSERT = 0,
  BRIDGE_REF_REPLACE = 1,
  BRIDGE_REF_DELETE = 2,
  BRIDGE_REF_INSERT_LIN = 3,
  BRIDGE_REF_REPLACE_LIN = 4,
  BRIDGE_REF_DELETE_LIN = 5
};

struct RefFixLIN
{
  unsigned tagNo; // Starting from 1
  unsigned fieldNo; // Starting from 1
  bool reverseFlag;
  string tag;
  string was;
  string is;
  unsigned extent;
};

struct RefFix
{
  unsigned lno; // First line is 1
  FixType type;
  string value;
  RefFixLIN fixLIN;
  unsigned count;
  bool partialFlag;
};

enum RefErrorsType
{
  ERR_LIN_VG_FIRST,
  ERR_LIN_VG_LAST,
  ERR_LIN_VG_REPLACE,
  ERR_LIN_VG_SYNTAX,

  ERR_LIN_VHEADER_INSERT,
  ERR_LIN_VHEADER_SYNTAX,

  ERR_LIN_RESULTS_REPLACE, 
  ERR_LIN_RESULTS_INSERT, 
  ERR_LIN_RESULTS_DELETE, 

  ERR_LIN_RS_REPLACE, 
  ERR_LIN_RS_INSERT, 
  ERR_LIN_RS_DELETE, 
  ERR_LIN_RS_DECL_PARD,
  ERR_LIN_RS_DECL_OPP,
  ERR_LIN_RS_DENOM,
  ERR_LIN_RS_LEVEL,
  ERR_LIN_RS_MULT,
  ERR_LIN_RS_TRICKS,
  ERR_LIN_RS_EMPTY,
  ERR_LIN_RS_INCOMPLETE,
  ERR_LIN_RS_SYNTAX,

  ERR_LIN_PLAYERS_UNKNOWN,
  ERR_LIN_PLAYERS_WRONG,
  ERR_LIN_PLAYERS_REPLACE,
  ERR_LIN_PLAYERS_DELETE,

  ERR_LIN_PN_REPLACE,
  ERR_LIN_PN_INSERT,
  ERR_LIN_PN_DELETE,

  ERR_LIN_QX_REPLACE,
  ERR_LIN_QX_ORDER_COCO,
  ERR_LIN_QX_ORDER_OOCC,
  ERR_LIN_QX_UNORDERED,

  ERR_LIN_MD_REPLACE,
  ERR_LIN_MD_MISSING,
  ERR_LIN_MD_SYNTAX,

  ERR_LIN_NT_SYNTAX,

  ERR_LIN_SV_REPLACE,
  ERR_LIN_SV_INSERT,
  ERR_LIN_SV_DELETE,
  ERR_LIN_SV_SYNTAX,

  ERR_LIN_MB_TRAILING,
  ERR_LIN_MB_REPLACE,
  ERR_LIN_MB_INSERT,
  ERR_LIN_MB_DELETE,
  ERR_LIN_MB_SYNTAX,

  ERR_LIN_AN_REPLACE,
  ERR_LIN_AN_DELETE,

  ERR_LIN_PC_REPLACE,
  ERR_LIN_PC_INSERT,
  ERR_LIN_PC_DELETE,
  ERR_LIN_PC_SYNTAX,

  ERR_LIN_MC_REPLACE,
  ERR_LIN_MC_INSERT,
  ERR_LIN_MC_DELETE,
  ERR_LIN_MC_SYNTAX,

  ERR_LIN_TRICK_INSERT,
  ERR_LIN_TRICK_DELETE,

  ERR_LIN_HAND_OUT_OF_RANGE,
  ERR_LIN_HAND_DUPLICATED,
  ERR_LIN_HAND_AUCTION_NONE,
  ERR_LIN_HAND_AUCTION_WRONG,
  ERR_LIN_HAND_AUCTION_LIVE,
  ERR_LIN_HAND_AUCTION_ABBR,
  ERR_LIN_HAND_CARDS_MISSING,
  ERR_LIN_HAND_CARDS_WRONG,
  ERR_LIN_HAND_PLAY_MISSING,
  ERR_LIN_HAND_PLAY_WRONG,
  ERR_LIN_HAND_DIRECTOR,

  ERR_LIN_SYNTAX,
  ERR_LIN_OMIT,

  ERR_PBN_SITE_REPLACE,
  ERR_PBN_ROOM_REPLACE,
  ERR_PBN_ROOM_INSERT,
  ERR_PBN_PLAYER_REPLACE,
  ERR_PBN_BOARD_REPLACE,
  ERR_PBN_DECLARER_REPLACE,
  ERR_PBN_RESULT_REPLACE,
  ERR_PBN_SCORE_REPLACE,
  ERR_PBN_SCORE_IMP_REPLACE,
  ERR_PBN_CALL_REPLACE,
  ERR_PBN_ALERT_REPLACE,
  ERR_PBN_ALERT_INSERT,
  ERR_PBN_ALERT_DELETE,
  ERR_PBN_NOTE_REPLACE,
  ERR_PBN_NOTE_INSERT,
  ERR_PBN_NOTE_DELETE,
  ERR_PBN_PLAY_REPLACE,
  ERR_PBN_HAND_AUCTION_LIVE,
  ERR_PBN_SYNTAX,
  ERR_PBN_OMIT,

  ERR_RBN_L_DELETE,
  ERR_RBN_N_REPLACE,
  ERR_RBN_P_REPLACE,
  ERR_RBN_R_REPLACE,
  ERR_RBN_HAND_AUCTION_LIVE,

  ERR_SIZE
};

struct RefErrorBundle
{
  RefErrorsType val;
  string name;
  string text;
};

const vector<RefErrorBundle> RefErrors =
{
  {ERR_LIN_VG_FIRST, "ERR_LIN_VG_FIRST", "LIN vg first number"},
  {ERR_LIN_VG_LAST, "ERR_LIN_VG_LAST", "LIN vg last number"},
  {ERR_LIN_VG_REPLACE, "ERR_LIN_VG_REPLACE", "LIN vg line replace"},
  {ERR_LIN_VG_SYNTAX, "ERR_LIN_VG_SYNTAX", "LIN vg syntax"},

  {ERR_LIN_VHEADER_INSERT, "ERR_LIN_HEADER_INSERT", "LIN vg line insert"},
  {ERR_LIN_VHEADER_SYNTAX, "ERR_LIN_VHEADER_SYNTAX", "LIN vg line syntax"},

  {ERR_LIN_RESULTS_REPLACE, "ERR_LIN_RESULTS_REPLACE", "LIN contracts wrong"},
  {ERR_LIN_RESULTS_INSERT, "ERR_LIN_RESULTS_INSERT", "LIN contracts missing"},
  {ERR_LIN_RESULTS_DELETE, "ERR_LIN_RESULTS_DELETE", 
    "LIN contract line delete"},

  {ERR_LIN_RS_REPLACE, "ERR_LIN_RS_REPLACE", "LIN rs replace contract"},
  {ERR_LIN_RS_INSERT, "ERR_LIN_RS_INSERT", "LIN rs INSERT contract"},
  {ERR_LIN_RS_DELETE, "ERR_LIN_RS_DELETE", "LIN rs delete contract"},
  {ERR_LIN_RS_DECL_PARD, "ERR_LIN_RS_DECL_PARD", "LIN rs partner is declarer"},
  {ERR_LIN_RS_DECL_OPP, "ERR_LIN_RS_DECL_OPP", "LIN rs opponent is declarer"},
  {ERR_LIN_RS_DENOM, "ERR_LIN_RS_DENOM", "LIN rs denomination wrong"},
  {ERR_LIN_RS_LEVEL, "ERR_LIN_RS_LEVEL", "LIN rs level wrong"},
  {ERR_LIN_RS_MULT, "ERR_LIN_RS_MULT", "LIN rs doubling wrong"},
  {ERR_LIN_RS_TRICKS, "ERR_LIN_RS_TRICKS", "LIN rs tricks wrong"},
  {ERR_LIN_RS_EMPTY, "ERR_LIN_RS_EMPTY", "LIN rs field empty"},
  {ERR_LIN_RS_INCOMPLETE, "ERR_LIN_RS_INCOMPLETE", "LIN rs field incomplete"},
  {ERR_LIN_RS_SYNTAX, "ERR_LIN_RS_SYNTAX", "LIN rs field syntax"},


  {ERR_LIN_PLAYERS_UNKNOWN, "ERR_LIN_PLAYERS_UNKNOWN", "LIN not all players"},
  {ERR_LIN_PLAYERS_WRONG, "ERR_LIN_PLAYERS_WRONG", "LIN player name commas"},
  {ERR_LIN_PLAYERS_REPLACE, "ERR_LIN_PLAYERS_REPLACE", "LIN replace players"},
  {ERR_LIN_PLAYERS_DELETE, "ERR_LIN_PLAYERS_DELETE", "LIN delete player line"},

  {ERR_LIN_PN_REPLACE, "ERR_LIN_PN_REPLACE", "LIN player replace"},
  {ERR_LIN_PN_INSERT, "ERR_LIN_PN_INSERT", "LIN player insert"},
  {ERR_LIN_PN_DELETE, "ERR_LIN_PN_DELETE", "LIN player delete"},

  {ERR_LIN_QX_REPLACE, "ERR_LIN_QXREPLACE", "LIN qx replace"},
  {ERR_LIN_QX_ORDER_COCO, "ERR_LIN_QX_ORDER_COCO", "LIN qx order COCO"},
  {ERR_LIN_QX_ORDER_OOCC, "ERR_LIN_QX_ORDER_OOCC", "LIN qx order OOCC"},
  {ERR_LIN_QX_UNORDERED, "ERR_LIN_QX_UNORDERED", "LIN qx unordered"},

  {ERR_LIN_MD_REPLACE, "ERR_LIN_MD_REPLACE", "LIN md deal replace"},
  {ERR_LIN_MD_MISSING, "ERR_LIN_MD_MISSING", "LIN md deal missing"},
  {ERR_LIN_MD_SYNTAX, "ERR_LIN_MD_SYNTAX", "LIN md syntax"},

  {ERR_LIN_NT_SYNTAX, "ERR_LIN_NT_SYNTAX", "LIN nt syntax"},

  {ERR_LIN_SV_REPLACE, "ERR_LIN_SV_REPLACE", "LIN sv wrong"},
  {ERR_LIN_SV_INSERT, "ERR_LIN_SV_INSERT", "LIN sv missing"},
  {ERR_LIN_SV_DELETE, "ERR_LIN_SV_DELETE", "LIN sv spare"},
  {ERR_LIN_SV_SYNTAX, "ERR_LIN_SV_SYNTAX", "LIN sv syntax"},

  {ERR_LIN_MB_TRAILING, "ERR_LIN_MB_TRAILING", "LIN mb auction long"},
  {ERR_LIN_MB_REPLACE, "ERR_LIN_MB_REPLACE", "LIN mb bid wrong"},
  {ERR_LIN_MB_INSERT, "ERR_LIN_MB_INSERT", "LIN mb bid missing"},
  {ERR_LIN_MB_DELETE, "ERR_LIN_MB_DELETE", "LIN mb bid spare"},
  {ERR_LIN_MB_SYNTAX, "ERR_LIN_MB_SYNTAX", "LIN mb syntax"},

  {ERR_LIN_AN_REPLACE, "ERR_LIN_AN_REPLACE", "LIN an replace"},
  {ERR_LIN_AN_DELETE, "ERR_LIN_AN_DELETE", "LIN an spare"},

  {ERR_LIN_PC_REPLACE, "ERR_LIN_PC_REPLACE", "LIN pc play wrong"},
  {ERR_LIN_PC_INSERT, "ERR_LIN_PC_INSERT", "LIN pc play missing"},
  {ERR_LIN_PC_DELETE, "ERR_LIN_PC_DELETE", "LIN pc play spare"},
  {ERR_LIN_PC_SYNTAX, "ERR_LIN_PC_SYNTAX", "LIN pc syntax"},

  {ERR_LIN_MC_REPLACE, "ERR_LIN_MC_REPLACE", "LIN mc claim wrong"},
  {ERR_LIN_MC_INSERT, "ERR_LIN_MC_INSERT", "LIN mc claim missing"},
  {ERR_LIN_MC_DELETE, "ERR_LIN_MC_DELETE", "LIN mc claim surplus"},
  {ERR_LIN_MC_SYNTAX, "ERR_LIN_MC_SYNTAX", "LIN mc syntax"},

  {ERR_LIN_TRICK_INSERT, "ERR_LIN_TRICK_INSERT", "LIN insert trick"},
  {ERR_LIN_TRICK_DELETE, "ERR_LIN_TRICK_DELETE", "LIN delete trick"},

  {ERR_LIN_HAND_OUT_OF_RANGE, "ERR_LIN_HAND_OUT_OF_RANGE",
   "LIN hand out of range"},
  {ERR_LIN_HAND_DUPLICATED, "ERR_LIN_HAND_DUPLICATED", "LIN duplicated"},
  {ERR_LIN_HAND_AUCTION_NONE, "ERR_LIN_AUCTION_NONE", "LIN no auction"},
  {ERR_LIN_HAND_AUCTION_WRONG, "ERR_LIN_AUCTION_WRONG", "LIN auction wrong"},
  {ERR_LIN_HAND_AUCTION_LIVE, "ERR_LIN_AUCTION_LIVE", "LIN auction not over"},
  {ERR_LIN_HAND_AUCTION_ABBR, "ERR_LIN_AUCTION_ABBR", 
    "LIN auction abbreviated"},
  {ERR_LIN_HAND_CARDS_MISSING, "ERR_LIN_CARDS_MISSING", "LIN cards missing"},
  {ERR_LIN_HAND_CARDS_WRONG, "ERR_LIN_CARDS_WRONG", "LIN cards wrong"},
  {ERR_LIN_HAND_PLAY_MISSING, "ERR_LIN_PLAY_MISSING", "LIN play missing"},
  {ERR_LIN_HAND_PLAY_WRONG, "ERR_LIN_PLAY_WRONG", "LIN play wrong"},
  {ERR_LIN_HAND_DIRECTOR, "ERR_LIN_HAND_DIRECTOR", "LIN director decision"},


  {ERR_LIN_SYNTAX, "ERR_LIN_SYNTAX", "LIN syntax"},

  {ERR_LIN_OMIT, "ERR_LIN_OMIT", "LIN omit in general"},

  {ERR_PBN_SITE_REPLACE, "ERR_PBN_SITE_REPLACE", "PBN site replace"},
  {ERR_PBN_ROOM_REPLACE, "ERR_PBN_ROOM_REPLACE", "PBN room replace"},
  {ERR_PBN_ROOM_INSERT, "ERR_PBN_ROOM_INSERT", "PBN room insert"},
  {ERR_PBN_PLAYER_REPLACE, "ERR_PBN_PLAYER_REPLACE", "PBN player replace"},
  {ERR_PBN_BOARD_REPLACE, "ERR_PBN_BOARD_REPLACE",  "PBN board replace"},
  {ERR_PBN_DECLARER_REPLACE, "ERR_PBN_DECLARER_REPLACE",  
    "PBN declarer replace"},
  {ERR_PBN_RESULT_REPLACE, "ERR_PBN_RESULT_REPLACE", "PBN result replace"},
  {ERR_PBN_SCORE_REPLACE, "ERR_SCORE_REPLACE", "PBN score replace"},
  {ERR_PBN_SCORE_IMP_REPLACE, "ERR_SCORE_IMP_REPLACE", 
    "PBN IMP score replace"},
  {ERR_PBN_CALL_REPLACE, "ERR_PBN_CALL_REPLACE", "PBN call replace"},
  {ERR_PBN_ALERT_REPLACE, "ERR_PBN_ALERT_REPLACE", "PBN alert replace"},
  {ERR_PBN_ALERT_INSERT, "ERR_PBN_ALERT_INSERT", "PBN alert insert"},
  {ERR_PBN_ALERT_DELETE, "ERR_PBN_ALERT_DELETE", "PBN alert delete"},
  {ERR_PBN_NOTE_REPLACE, "ERR_PBN_NOTE_REPLACE", "PBN note replace"},
  {ERR_PBN_NOTE_INSERT, "ERR_PBN_NOTE_INSERT", "PBN note insert"},
  {ERR_PBN_NOTE_DELETE, "ERR_PBN_NOTE_DELETE", "PBN note delete"},
  {ERR_PBN_PLAY_REPLACE, "ERR_PBN_PLAY_REPLACE", "PBN play replace"},
  {ERR_PBN_HAND_AUCTION_LIVE, "ERR_PBN_AUCTION_LIVE", 
    "PBN auction not over"},
  {ERR_PBN_SYNTAX, "ERR_PBN_SYNTAX", "PBN syntax"},
  {ERR_PBN_OMIT, "ERR_PBN_OMIT", "PBN omit in general"},

  {ERR_RBN_L_DELETE, "ERR_RBN_L_DELETE", "RBN location delete"},
  {ERR_RBN_N_REPLACE, "ERR_RBN_N_REPLACE", "RBN names replace"},
  {ERR_RBN_P_REPLACE, "ERR_RBN_P_REPLACE", "RBN play replace"},
  {ERR_RBN_R_REPLACE, "ERR_RBN_R_REPLACE", "RBN result replace"},
  {ERR_RBN_HAND_AUCTION_LIVE, "ERR_RBN_AUCTION_LIVE", 
    "RBN auction not over"},

  {ERR_SIZE, "ERR_SIZE", "Unclassified error"},
};

struct RefErrorClass
{
  FixType type;
  RefErrorsType code;
  vector<string> list;
  bool pureFlag;
  unsigned numTags;
};


void setRefTable();

void readRefFix(
  const string& fname,
  vector<RefFix>& refFix,
  RefControl& refControl);

string strRefFix(const RefFix& refFix);

bool modifyLINLine(
  const string& line,
  const RefFix& refFix,
  string& lineNew);

bool classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  RefErrorClass& diff);

#endif
