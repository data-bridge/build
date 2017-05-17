/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFCODES_H
#define BRIDGE_REFCODES_H

#include <string>
#include <vector>

using namespace std;


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
  ERR_RBN_N_INSERT,
  ERR_RBN_P_REPLACE,
  ERR_RBN_R_REPLACE,
  ERR_RBN_HAND_AUCTION_LIVE,

  ERR_RBX_L_DELETE,
  ERR_RBX_N_REPLACE,
  ERR_RBX_N_INSERT,
  ERR_RBX_P_REPLACE,
  ERR_RBX_R_REPLACE,
  ERR_RBX_HAND_AUCTION_LIVE,

  ERR_EML_PLAY_REPLACE,
  ERR_EML_RESULT_REPLACE,
  ERR_EML_SCORE_REPLACE,
  ERR_EML_SCORE_IMP_REPLACE,
  ERR_EML_HAND_AUCTION_LIVE,

  ERR_SIZE
};

struct RefErrorBundle
{
  RefErrorsType val;
  string name;
};

const vector<RefErrorBundle> RefErrors =
{
  {ERR_LIN_VG_FIRST, "ERR_LIN_VG_FIRST"},
  {ERR_LIN_VG_LAST, "ERR_LIN_VG_LAST"},
  {ERR_LIN_VG_REPLACE, "ERR_LIN_VG_REPLACE"},
  {ERR_LIN_VG_SYNTAX, "ERR_LIN_VG_SYNTAX"},

  {ERR_LIN_VHEADER_INSERT, "ERR_LIN_VHEADER_INSERT"},
  {ERR_LIN_VHEADER_SYNTAX, "ERR_LIN_VHEADER_SYNTAX"},

  {ERR_LIN_RESULTS_REPLACE, "ERR_LIN_RESULTS_REPLACE"},
  {ERR_LIN_RESULTS_INSERT, "ERR_LIN_RESULTS_INSERT"},
  {ERR_LIN_RESULTS_DELETE, "ERR_LIN_RESULTS_DELETE"},

  {ERR_LIN_RS_REPLACE, "ERR_LIN_RS_REPLACE"},
  {ERR_LIN_RS_INSERT, "ERR_LIN_RS_INSERT"},
  {ERR_LIN_RS_DELETE, "ERR_LIN_RS_DELETE"},
  {ERR_LIN_RS_DECL_PARD, "ERR_LIN_RS_DECL_PARD"},
  {ERR_LIN_RS_DECL_OPP, "ERR_LIN_RS_DECL_OPP"},
  {ERR_LIN_RS_DENOM, "ERR_LIN_RS_DENOM"},
  {ERR_LIN_RS_LEVEL, "ERR_LIN_RS_LEVEL"},
  {ERR_LIN_RS_MULT, "ERR_LIN_RS_MULT"},
  {ERR_LIN_RS_TRICKS, "ERR_LIN_RS_TRICKS"},
  {ERR_LIN_RS_EMPTY, "ERR_LIN_RS_EMPTY"},
  {ERR_LIN_RS_INCOMPLETE, "ERR_LIN_RS_INCOMPLETE"},
  {ERR_LIN_RS_SYNTAX, "ERR_LIN_RS_SYNTAX"},

  {ERR_LIN_PLAYERS_UNKNOWN, "ERR_LIN_PLAYERS_UNKNOWN"},
  {ERR_LIN_PLAYERS_WRONG, "ERR_LIN_PLAYERS_WRONG"},
  {ERR_LIN_PLAYERS_REPLACE, "ERR_LIN_PLAYERS_REPLACE"},
  {ERR_LIN_PLAYERS_DELETE, "ERR_LIN_PLAYERS_DELETE"},

  {ERR_LIN_PN_REPLACE, "ERR_LIN_PN_REPLACE"},
  {ERR_LIN_PN_INSERT, "ERR_LIN_PN_INSERT"},
  {ERR_LIN_PN_DELETE, "ERR_LIN_PN_DELETE"},

  {ERR_LIN_QX_REPLACE, "ERR_LIN_QX_REPLACE"},
  {ERR_LIN_QX_ORDER_COCO, "ERR_LIN_QX_ORDER_COCO"},
  {ERR_LIN_QX_ORDER_OOCC, "ERR_LIN_QX_ORDER_OOCC"},
  {ERR_LIN_QX_UNORDERED, "ERR_LIN_QX_UNORDERED"},

  {ERR_LIN_MD_REPLACE, "ERR_LIN_MD_REPLACE"},
  {ERR_LIN_MD_MISSING, "ERR_LIN_MD_MISSING"},
  {ERR_LIN_MD_SYNTAX, "ERR_LIN_MD_SYNTAX"},

  {ERR_LIN_NT_SYNTAX, "ERR_LIN_NT_SYNTAX"},

  {ERR_LIN_SV_REPLACE, "ERR_LIN_SV_REPLACE"},
  {ERR_LIN_SV_INSERT, "ERR_LIN_SV_INSERT"},
  {ERR_LIN_SV_DELETE, "ERR_LIN_SV_DELETE"},
  {ERR_LIN_SV_SYNTAX, "ERR_LIN_SV_SYNTAX"},

  {ERR_LIN_MB_TRAILING, "ERR_LIN_MB_TRAILING"},
  {ERR_LIN_MB_REPLACE, "ERR_LIN_MB_REPLACE"},
  {ERR_LIN_MB_INSERT, "ERR_LIN_MB_INSERT"},
  {ERR_LIN_MB_DELETE, "ERR_LIN_MB_DELETE"},
  {ERR_LIN_MB_SYNTAX, "ERR_LIN_MB_SYNTAX"},

  {ERR_LIN_AN_REPLACE, "ERR_LIN_AN_REPLACE"},
  {ERR_LIN_AN_DELETE, "ERR_LIN_AN_DELETE"},

  {ERR_LIN_PC_REPLACE, "ERR_LIN_PC_REPLACE"},
  {ERR_LIN_PC_INSERT, "ERR_LIN_PC_INSERT"},
  {ERR_LIN_PC_DELETE, "ERR_LIN_PC_DELETE"},
  {ERR_LIN_PC_SYNTAX, "ERR_LIN_PC_SYNTAX"},

  {ERR_LIN_MC_REPLACE, "ERR_LIN_MC_REPLACE"},
  {ERR_LIN_MC_INSERT, "ERR_LIN_MC_INSERT"},
  {ERR_LIN_MC_DELETE, "ERR_LIN_MC_DELETE"},
  {ERR_LIN_MC_SYNTAX, "ERR_LIN_MC_SYNTAX"},

  {ERR_LIN_TRICK_INSERT, "ERR_LIN_TRICK_INSERT"},
  {ERR_LIN_TRICK_DELETE, "ERR_LIN_TRICK_DELETE"},

  {ERR_LIN_HAND_OUT_OF_RANGE, "ERR_LIN_HAND_OUT_OF_RANGE"},
  {ERR_LIN_HAND_DUPLICATED, "ERR_LIN_HAND_DUPLICATED"},
  {ERR_LIN_HAND_AUCTION_NONE, "ERR_LIN_HAND_AUCTION_NONE"},
  {ERR_LIN_HAND_AUCTION_WRONG, "ERR_LIN_HAND_AUCTION_WRONG"},
  {ERR_LIN_HAND_AUCTION_LIVE, "ERR_LIN_HAND_AUCTION_LIVE"},
  {ERR_LIN_HAND_AUCTION_ABBR, "ERR_LIN_HAND_AUCTION_ABBR"},
  {ERR_LIN_HAND_CARDS_MISSING, "ERR_LIN_HAND_CARDS_MISSING"},
  {ERR_LIN_HAND_CARDS_WRONG, "ERR_LIN_HAND_CARDS_WRONG"},
  {ERR_LIN_HAND_PLAY_MISSING, "ERR_LIN_HAND_PLAY_MISSING"},
  {ERR_LIN_HAND_PLAY_WRONG, "ERR_LIN_HAND_PLAY_WRONG"},
  {ERR_LIN_HAND_DIRECTOR, "ERR_LIN_HAND_DIRECTOR"},

  {ERR_LIN_SYNTAX, "ERR_LIN_SYNTAX"},
  {ERR_LIN_OMIT, "ERR_LIN_OMIT"},

  {ERR_PBN_SITE_REPLACE, "ERR_PBN_SITE_REPLACE"},
  {ERR_PBN_ROOM_REPLACE, "ERR_PBN_ROOM_REPLACE"},
  {ERR_PBN_ROOM_INSERT, "ERR_PBN_ROOM_INSERT"},
  {ERR_PBN_PLAYER_REPLACE, "ERR_PBN_PLAYER_REPLACE"},
  {ERR_PBN_BOARD_REPLACE, "ERR_PBN_BOARD_REPLACE"},
  {ERR_PBN_DECLARER_REPLACE, "ERR_PBN_DECLARER_REPLACE"},
  {ERR_PBN_RESULT_REPLACE, "ERR_PBN_RESULT_REPLACE"},
  {ERR_PBN_SCORE_REPLACE, "ERR_PBN_SCORE_REPLACE"},
  {ERR_PBN_SCORE_IMP_REPLACE, "ERR_PBN_SCORE_IMP_REPLACE"},
  {ERR_PBN_CALL_REPLACE, "ERR_PBN_CALL_REPLACE"},
  {ERR_PBN_ALERT_REPLACE, "ERR_PBN_ALERT_REPLACE"},
  {ERR_PBN_ALERT_INSERT, "ERR_PBN_ALERT_INSERT"},
  {ERR_PBN_ALERT_DELETE, "ERR_PBN_ALERT_DELETE"},
  {ERR_PBN_NOTE_REPLACE, "ERR_PBN_NOTE_REPLACE"},
  {ERR_PBN_NOTE_INSERT, "ERR_PBN_NOTE_INSERT"},
  {ERR_PBN_NOTE_DELETE, "ERR_PBN_NOTE_DELETE"},
  {ERR_PBN_PLAY_REPLACE, "ERR_PBN_PLAY_REPLACE"},
  {ERR_PBN_HAND_AUCTION_LIVE, "ERR_PBN_HAND_AUCTION_LIVE"},
  {ERR_PBN_SYNTAX, "ERR_PBN_SYNTAX"},
  {ERR_PBN_OMIT, "ERR_PBN_OMIT"},

  {ERR_RBN_L_DELETE, "ERR_RBN_L_DELETE"},
  {ERR_RBN_N_REPLACE, "ERR_RBN_N_REPLACE"},
  {ERR_RBN_N_INSERT, "ERR_RBN_N_INSERT"},
  {ERR_RBN_P_REPLACE, "ERR_RBN_P_REPLACE"},
  {ERR_RBN_R_REPLACE, "ERR_RBN_R_REPLACE"},
  {ERR_RBN_HAND_AUCTION_LIVE, "ERR_RBN_HAND_AUCTION_LIVE"},

  {ERR_RBX_L_DELETE, "ERR_RBX_L_DELETE"},
  {ERR_RBX_N_REPLACE, "ERR_RBX_N_REPLACE"},
  {ERR_RBX_N_INSERT, "ERR_RBX_N_INSERT"},
  {ERR_RBX_P_REPLACE, "ERR_RBX_P_REPLACE"},
  {ERR_RBX_R_REPLACE, "ERR_RBX_R_REPLACE"},
  {ERR_RBX_HAND_AUCTION_LIVE, "ERR_RBX_HAND_AUCTION_LIVE"},

  {ERR_EML_PLAY_REPLACE, "ERR_EML_PLAY_REPLACE"},
  {ERR_EML_RESULT_REPLACE, "ERR_EML_RESULT_REPLACE"},
  {ERR_EML_SCORE_REPLACE, "ERR_EML_SCORE_REPLACE"},
  {ERR_EML_SCORE_IMP_REPLACE, "ERR_EML_SCORE_IMP_REPLACE"},
  {ERR_EML_HAND_AUCTION_LIVE, "ERR_EML_HAND_AUCTION_LIVE"},

  {ERR_SIZE, "ERR_SIZE"},
};

#endif
