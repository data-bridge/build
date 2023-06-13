/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef REF_ACTIONCONST_H
#define REF_ACTIONCONST_H

#include "RefEntry.h"

/*
struct RefCount
{
  unsigned lines;
  unsigned units;
  unsigned hands; // qx
  unsigned boards; // bd
};

struct RefEntry
{
  unsigned files;
  unsigned noRefLines;
  RefCount count;
};
*/


enum ActionType
{
  REF_ACTION_REPLACE_GEN = 0,
  REF_ACTION_INSERT_GEN = 1,
  REF_ACTION_DELETE_GEN = 2,

  REF_ACTION_REPLACE_LIN = 3,
  REF_ACTION_INSERT_LIN = 4,
  REF_ACTION_DELETE_LIN = 5,

  REF_ACTION_REPLACE_PBN = 6,
  REF_ACTION_INSERT_PBN = 7,
  REF_ACTION_DELETE_PBN = 8,

  REF_ACTION_REPLACE_RBN = 9,
  REF_ACTION_INSERT_RBN = 10,
  REF_ACTION_DELETE_RBN = 11,

  REF_ACTION_REPLACE_RBX = 12,
  REF_ACTION_INSERT_RBX = 13,
  REF_ACTION_DELETE_RBX = 14,

  REF_ACTION_REPLACE_TXT = 15,
  REF_ACTION_INSERT_TXT = 16,
  REF_ACTION_DELETE_TXT = 17,

  REF_ACTION_REPLACE_WORD = 18,
  REF_ACTION_INSERT_WORD = 19,
  REF_ACTION_DELETE_WORD = 20,

  REF_ACTION_REPLACE_FROM = 21,
  REF_ACTION_INSERT_FROM = 22,

  REF_ACTION_DELETE_EML = 23,

  REF_ACTION_SIZE = 24
};


enum RefTag
{
  REF_TAGS_LIN_VG = 0,
  REF_TAGS_LIN_RS = 1,
  REF_TAGS_LIN_PW = 2,
  REF_TAGS_LIN_BN = 3,
  REF_TAGS_LIN_QX = 4,
  REF_TAGS_LIN_PN = 5,
  REF_TAGS_LIN_MD = 6,
  REF_TAGS_LIN_SV = 7,
  REF_TAGS_LIN_MB = 8,
  REF_TAGS_LIN_AN = 9,
  REF_TAGS_LIN_PC = 10,
  REF_TAGS_LIN_MC = 11,
  REF_TAGS_LIN_PG = 12,
  REF_TAGS_LIN_NT = 13,

  REF_TAGS_PBN_SITE = 14,
  REF_TAGS_PBN_BOARD = 15,
  REF_TAGS_PBN_PLAYER = 16,
  REF_TAGS_PBN_DECLARER = 17,
  REF_TAGS_PBN_RESULT = 18,
  REF_TAGS_PBN_AUCTION = 19,
  REF_TAGS_PBN_NOTE = 20,
  REF_TAGS_PBN_PLAY = 21,
  REF_TAGS_PBN_ROOM = 22,
  REF_TAGS_PBN_SCORE = 23,
  REF_TAGS_PBN_SCORE_IMP = 24,

  REF_TAGS_RBN_A = 25,
  REF_TAGS_RBN_L = 26,
  REF_TAGS_RBN_N = 27,
  REF_TAGS_RBN_P = 28,
  REF_TAGS_RBN_R = 29,

  REF_TAGS_SIZE = 30
};


enum CommentType
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
  ERR_LIN_RS_DELETE_RANGE, 
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

  ERR_LIN_MD_REPLACE,
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
  ERR_LIN_TRICK_LINE_DELETE,

  ERR_LIN_AUCTION_DELETE,

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

  ERR_LIN_ORDER_COCO_INFER,
  ERR_LIN_ORDER_OOCC_INFER,
  ERR_LIN_UNORDERED,

  ERR_LIN_TAG_SYNTAX,
  ERR_LIN_DUPLICATE,
  ERR_LIN_SUBSET,
  ERR_LIN_MERGED,
  ERR_LIN_SYNTAX,
  ERR_LIN_OMIT,

  ERR_PBN_SITE_REPLACE,
  ERR_PBN_ROOM_REPLACE,
  ERR_PBN_ROOM_INSERT,
  ERR_PBN_PLAYER_INSERT,
  ERR_PBN_PLAYER_REPLACE,
  ERR_PBN_BOARD_REPLACE,
  ERR_PBN_DECLARER_REPLACE,
  ERR_PBN_RESULT_REPLACE,
  ERR_PBN_SCORE_REPLACE,
  ERR_PBN_SCORE_IMP_REPLACE,
  ERR_PBN_SCORE_IMP_DELETE,
  ERR_PBN_CALL_REPLACE,
  ERR_PBN_ALERT_REPLACE,
  ERR_PBN_ALERT_INSERT,
  ERR_PBN_ALERT_DELETE,
  ERR_PBN_NOTE_REPLACE,
  ERR_PBN_NOTE_INSERT,
  ERR_PBN_NOTE_DELETE,
  ERR_PBN_AUCTION_DELETE,
  ERR_PBN_PLAY_REPLACE,
  ERR_PBN_PLAY_DELETE,
  ERR_PBN_HAND_AUCTION_LIVE,
  ERR_PBN_ORDER_COCO_INFER,
  ERR_PBN_ORDER_OOCC_INFER,
  ERR_PBN_SYNTAX,
  ERR_PBN_OMIT,

  ERR_RBN_L_DELETE,
  ERR_RBN_N_REPLACE,
  ERR_RBN_N_INSERT,
  ERR_RBN_A_DELETE,
  ERR_RBN_P_REPLACE,
  ERR_RBN_P_DELETE,
  ERR_RBN_R_REPLACE,
  ERR_RBN_R_DELETE,
  ERR_RBN_HAND_AUCTION_LIVE,
  ERR_RBN_ORDER_COCO_INFER,
  ERR_RBN_ORDER_OOCC_INFER,

  ERR_RBX_L_DELETE,
  ERR_RBX_N_REPLACE,
  ERR_RBX_N_INSERT,
  ERR_RBX_A_DELETE,
  ERR_RBX_P_REPLACE,
  ERR_RBX_P_DELETE,
  ERR_RBX_R_REPLACE,
  ERR_RBX_R_DELETE,
  ERR_RBX_HAND_AUCTION_LIVE,
  ERR_RBX_ORDER_COCO_INFER,
  ERR_RBX_ORDER_OOCC_INFER,

  ERR_TXT_LOCATION_REPLACE,
  ERR_TXT_DEALER_REPLACE,
  ERR_TXT_AUCTION_DELETE,
  ERR_TXT_PLAY_REPLACE,
  ERR_TXT_PLAY_DELETE,
  ERR_TXT_RESULT_REPLACE,
  ERR_TXT_SCORE_REPLACE,
  ERR_TXT_SCORE_IMP_REPLACE,
  ERR_TXT_RUNNING_REPLACE,
  ERR_TXT_HAND_AUCTION_LIVE,
  ERR_TXT_ORDER_COCO,
  ERR_TXT_ORDER_OOCC,

  ERR_EML_CONTRACT_INSERT,
  ERR_EML_PLAY_REPLACE,
  ERR_EML_PLAY_DELETE,
  ERR_EML_RESULT_REPLACE,
  ERR_EML_SCORE_REPLACE,
  ERR_EML_SCORE_IMP_REPLACE,
  ERR_EML_HAND_AUCTION_LIVE,
  ERR_EML_CANVAS_DELETE,
  ERR_EML_LINE_DELETE,
  ERR_EML_ORDER_COCO,
  ERR_EML_ORDER_OOCC,

  ERR_REC_AUCTION_DELETE,
  ERR_REC_CONTRACT_INSERT,
  ERR_REC_PLAY_REPLACE,
  ERR_REC_PLAY_DELETE,
  ERR_REC_LEAD_REPLACE,
  ERR_REC_RESULT_REPLACE,
  ERR_REC_SCORE_REPLACE,
  ERR_REC_SCORE_IMP_REPLACE,
  ERR_REC_SCORE_IMP_DELETE,
  ERR_REC_HAND_AUCTION_LIVE,
  ERR_REC_ORDER_COCO,
  ERR_REC_ORDER_OOCC,

  ERR_SIZE
};

enum RefCountType
{
  REF_COUNT_INACTIVE = 0,
  REF_COUNT_HEADER = 1, // Single-tag header, (1,n,m)
  REF_COUNT_HANDS = 2, // multi-line delete, (0,c,d) 
  REF_COUNT_SINGLE = 3, // (1,1,1)
  REF_COUNT_LIN_IS = 4, // count isVal, (c,1,1)
  REF_COUNT_LIN_REPEAT = 5, // tag count, (c,1,1)
  REF_COUNT_LIN_FIELDS = 6, // tag count, (c,1,1)
  REF_COUNT_SIZE = 7
};

#endif
