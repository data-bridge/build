/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <map>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include "RefComment.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


struct ActionBundle
{
  ActionType val;
  string name;
};

const vector<ActionBundle> ActionList =
{
  {BRIDGE_REF_REPLACE_GEN, "replace"},
  {BRIDGE_REF_INSERT_GEN, "insert"},
  {BRIDGE_REF_DELETE_GEN, "delete"},

  {BRIDGE_REF_REPLACE_LIN, "replaceLIN"},
  {BRIDGE_REF_INSERT_LIN, "insertLIN"},
  {BRIDGE_REF_DELETE_LIN, "deleteLIN"},

  {BRIDGE_REF_REPLACE_PBN, "replacePBN"},
  {BRIDGE_REF_INSERT_PBN, "insertPBN"},
  {BRIDGE_REF_DELETE_PBN, "deletePBN"},

  {BRIDGE_REF_REPLACE_RBN, "replaceRBN"},
  {BRIDGE_REF_INSERT_RBN, "insertRBN"},
  {BRIDGE_REF_DELETE_RBN, "deleteRBN"},

  {BRIDGE_REF_REPLACE_RBX, "replaceRBX"},
  {BRIDGE_REF_INSERT_RBX, "insertRBX"},
  {BRIDGE_REF_DELETE_RBX, "deleteRBX"},

  {BRIDGE_REF_REPLACE_TXT, "replaceTXT"},
  {BRIDGE_REF_INSERT_TXT, "insertTXT"},
  {BRIDGE_REF_DELETE_TXT, "deleteTXT"},

  {BRIDGE_REF_REPLACE_WORD, "replaceWORD"},
  {BRIDGE_REF_INSERT_WORD, "insertWORD"},
  {BRIDGE_REF_DELETE_WORD, "deleteWORD"}
};

struct TagBundle
{
  RefTag val;
  string name;
};

const vector<TagBundle> TagList =
{
  {REF_TAGS_LIN_VG, "vg"},
  {REF_TAGS_LIN_RS, "rs"},
  {REF_TAGS_LIN_PW, "pw"},
  {REF_TAGS_LIN_BN, "bn"},
  {REF_TAGS_LIN_BN, "bn"},
  {REF_TAGS_LIN_QX, "qx"},
  {REF_TAGS_LIN_PN, "pn"},
  {REF_TAGS_LIN_MD, "md"},
  {REF_TAGS_LIN_SV, "sv"},
  {REF_TAGS_LIN_MB, "mb"},
  {REF_TAGS_LIN_AN, "an"},
  {REF_TAGS_LIN_PC, "pc"},
  {REF_TAGS_LIN_MC, "mc"},
  {REF_TAGS_LIN_PG, "pg"},
  {REF_TAGS_LIN_NT, "nt"},

  {REF_TAGS_PBN_SITE, "Site"},
  {REF_TAGS_PBN_BOARD, "Board"},
  {REF_TAGS_PBN_PLAYER, "Player"},
  {REF_TAGS_PBN_DECLARER, "Declarer"},
  {REF_TAGS_PBN_RESULT, "Result"},
  {REF_TAGS_PBN_AUCTION, "Auction"},
  {REF_TAGS_PBN_NOTE, "Note"},
  {REF_TAGS_PBN_PLAY, "Play"},
  {REF_TAGS_PBN_ROOM, "Room"},
  {REF_TAGS_PBN_SCORE, "Score"},
  {REF_TAGS_PBN_SCORE_IMP, "ScoreIMP"},

  {REF_TAGS_RBN_A, "A"},
  {REF_TAGS_RBN_L, "L"},
  {REF_TAGS_RBN_N, "N"},
  {REF_TAGS_RBN_P, "P"},
  {REF_TAGS_RBN_R, "R"}
};

struct CommentBundle
{
  CommentType val;
  string name;
};

const vector<CommentBundle> CommentList =
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

  {ERR_TXT_PLAY_REPLACE, "ERR_TXT_PLAY_REPLACE"},
  {ERR_TXT_RESULT_REPLACE, "ERR_TXT_RESULT_REPLACE"},
  {ERR_TXT_SCORE_REPLACE, "ERR_TXT_SCORE_REPLACE"},
  {ERR_TXT_SCORE_IMP_REPLACE, "ERR_TXT_SCORE_IMP_REPLACE"},
  {ERR_TXT_RUNNING_REPLACE, "ERR_TXT_RUNNING_REPLACE"},
  {ERR_TXT_HAND_AUCTION_LIVE, "ERR_TXT_HAND_AUCTION_LIVE"},

  {ERR_EML_PLAY_REPLACE, "ERR_EML_PLAY_REPLACE"},
  {ERR_EML_RESULT_REPLACE, "ERR_EML_RESULT_REPLACE"},
  {ERR_EML_SCORE_REPLACE, "ERR_EML_SCORE_REPLACE"},
  {ERR_EML_SCORE_IMP_REPLACE, "ERR_EML_SCORE_IMP_REPLACE"},
  {ERR_EML_HAND_AUCTION_LIVE, "ERR_EML_HAND_AUCTION_LIVE"},

  {ERR_REC_PLAY_REPLACE, "ERR_REC_PLAY_REPLACE"},
  {ERR_REC_RESULT_REPLACE, "ERR_REC_RESULT_REPLACE"},
  {ERR_REC_SCORE_REPLACE, "ERR_REC_SCORE_REPLACE"},
  {ERR_REC_SCORE_IMP_REPLACE, "ERR_REC_SCORE_IMP_REPLACE"},
  {ERR_REC_HAND_AUCTION_LIVE, "ERR_REC_HAND_AUCTION_LIVE"},

  {ERR_SIZE, "ERR_SIZE"},
};


static map<string, CommentType> CommentMap;
static map<string, RefTag> TagMap;
static bool ActionCommentOK[BRIDGE_REF_FIX_SIZE][ERR_SIZE];
static bool TagCommentOK[REF_TAGS_SIZE][ERR_SIZE];

static mutex mtx;
static bool setTablesFlag = false;


RefComment::RefComment()
{
  RefComment::reset();
  if (! setTablesFlag)
  {
    mtx.lock();
    if (! setTablesFlag)
      RefComment::setTables();
    setTablesFlag = true;
    mtx.unlock();
  }
}


RefComment::~RefComment()
{
}


void RefComment::reset()
{
  setFlag = false;

  category = ERR_SIZE;
  count1 = 0;
  count2 = 0;
  count3 = 0;

  quote = "";
}


void RefComment::setTables()
{
  RefComment::setCommentMap();
  RefComment::setRefTag();
  RefComment::setActionTable();
  RefComment::setTagTable();
}


void RefComment::setCommentMap()
{
  for (auto &e: CommentList)
    CommentMap[e.name] = e.val;
}


void RefComment::setRefTag()
{
  for (auto &e: TagList)
    TagMap[e.name] = e.val;
}


void RefComment::setActionTable()
{
  for (unsigned i = 0; i < BRIDGE_REF_FIX_SIZE; i++)
    for (unsigned j = 0; j < ERR_SIZE; j++)
      ActionCommentOK[i][j] = false;
  
  bool * ACO = ActionCommentOK[BRIDGE_REF_REPLACE_GEN];
  ACO[ERR_LIN_VHEADER_SYNTAX] = true;
  ACO[ERR_LIN_PLAYERS_REPLACE] = true;
  ACO[ERR_LIN_RESULTS_REPLACE] = true;
  ACO[ERR_PBN_SCORE_REPLACE] = true; // TODO: Get rid of, JEC
  ACO[ERR_PBN_ALERT_REPLACE] = true; // TODO: Get rid of, JEC
  ACO[ERR_TXT_PLAY_REPLACE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_GEN];
  ACO[ERR_LIN_VHEADER_INSERT] = true;
  ACO[ERR_LIN_RESULTS_INSERT] = true;
  ACO[ERR_LIN_TRICK_INSERT] = true;
  ACO[ERR_PBN_NOTE_INSERT] = true; // TODO: Get rid of, JEC

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_GEN];
  ACO[ERR_LIN_TRICK_DELETE] = true;
  ACO[ERR_LIN_HAND_AUCTION_LIVE] = true;
  ACO[ERR_LIN_HAND_CARDS_MISSING] = true;
  ACO[ERR_LIN_HAND_CARDS_WRONG] = true;
  ACO[ERR_PBN_HAND_AUCTION_LIVE] = true;
  ACO[ERR_RBN_HAND_AUCTION_LIVE] = true;
  ACO[ERR_RBX_HAND_AUCTION_LIVE] = true;
  ACO[ERR_TXT_HAND_AUCTION_LIVE] = true;
  ACO[ERR_EML_HAND_AUCTION_LIVE] = true;
  ACO[ERR_REC_HAND_AUCTION_LIVE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_REPLACE_LIN];
  ACO[ERR_LIN_VG_FIRST] = true;
  ACO[ERR_LIN_VG_LAST] = true;
  ACO[ERR_LIN_VG_REPLACE] = true;
  ACO[ERR_LIN_VG_SYNTAX] = true;
  ACO[ERR_LIN_RS_REPLACE] = true;
  ACO[ERR_LIN_RS_DECL_PARD] = true;
  ACO[ERR_LIN_RS_DECL_OPP] = true;
  ACO[ERR_LIN_RS_DENOM] = true;
  ACO[ERR_LIN_RS_LEVEL] = true;
  ACO[ERR_LIN_RS_MULT] = true;
  ACO[ERR_LIN_RS_TRICKS] = true;
  ACO[ERR_LIN_RS_EMPTY] = true;
  ACO[ERR_LIN_RS_INCOMPLETE] = true;
  ACO[ERR_LIN_RS_SYNTAX] = true;
  ACO[ERR_LIN_PN_REPLACE] = true;
  ACO[ERR_LIN_QX_REPLACE] = true;
  ACO[ERR_LIN_MD_REPLACE] = true;
  ACO[ERR_LIN_SV_REPLACE] = true;
  ACO[ERR_LIN_MB_REPLACE] = true;
  ACO[ERR_LIN_MB_SYNTAX] = true;
  ACO[ERR_LIN_AN_REPLACE] = true;
  ACO[ERR_LIN_PC_REPLACE] = true;
  ACO[ERR_LIN_PC_SYNTAX] = true;
  ACO[ERR_LIN_MC_REPLACE] = true;
  ACO[ERR_LIN_MC_SYNTAX] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_LIN];
  ACO[ERR_LIN_RS_INSERT] = true;
  ACO[ERR_LIN_PN_INSERT] = true;
  ACO[ERR_LIN_SV_INSERT] = true;
  ACO[ERR_LIN_MB_INSERT] = true;
  ACO[ERR_LIN_MB_SYNTAX] = true;
  ACO[ERR_LIN_PC_INSERT] = true;
  ACO[ERR_LIN_MC_INSERT] = true;
  ACO[ERR_LIN_SYNTAX] = true;

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_LIN];
  ACO[ERR_LIN_VG_SYNTAX] = true;
  ACO[ERR_LIN_RS_DELETE] = true;
  ACO[ERR_LIN_PN_DELETE] = true;
  ACO[ERR_LIN_MD_SYNTAX] = true;
  ACO[ERR_LIN_NT_SYNTAX] = true;
  ACO[ERR_LIN_SV_DELETE] = true;
  ACO[ERR_LIN_SV_SYNTAX] = true;
  ACO[ERR_LIN_MB_TRAILING] = true;
  ACO[ERR_LIN_MB_DELETE] = true;
  ACO[ERR_LIN_MB_SYNTAX] = true;
  ACO[ERR_LIN_AN_DELETE] = true;
  ACO[ERR_LIN_PC_DELETE] = true;
  ACO[ERR_LIN_MC_DELETE] = true;
  ACO[ERR_LIN_MC_SYNTAX] = true;
  ACO[ERR_LIN_TRICK_DELETE] = true;
  ACO[ERR_LIN_SYNTAX] = true;

  ACO = ActionCommentOK[BRIDGE_REF_REPLACE_PBN];
  ACO[ERR_PBN_SITE_REPLACE] = true;
  ACO[ERR_PBN_BOARD_REPLACE] = true;
  ACO[ERR_PBN_DECLARER_REPLACE] = true;
  ACO[ERR_PBN_RESULT_REPLACE] = true;
  ACO[ERR_PBN_SCORE_REPLACE] = true;
  ACO[ERR_PBN_SCORE_IMP_REPLACE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_PBN];
  ACO[ERR_PBN_ROOM_INSERT] = true;
  ACO[ERR_PBN_NOTE_INSERT] = true;

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_PBN];
  ACO[ERR_PBN_NOTE_DELETE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_REPLACE_RBN];
  ACO[ERR_RBN_P_REPLACE] = true;
  ACO[ERR_RBN_R_REPLACE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_RBN];
  ACO[ERR_RBN_N_INSERT] = true;

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_RBN];
  ACO[ERR_RBN_L_DELETE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_REPLACE_RBX];
  ACO[ERR_RBX_P_REPLACE] = true;
  ACO[ERR_RBX_R_REPLACE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_RBX];
  ACO[ERR_RBX_N_INSERT] = true;

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_RBX];
  ACO[ERR_RBX_L_DELETE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_REPLACE_TXT];
  ACO[ERR_TXT_RESULT_REPLACE] = true;
  ACO[ERR_TXT_SCORE_REPLACE] = true;
  ACO[ERR_TXT_SCORE_IMP_REPLACE] = true;
  ACO[ERR_TXT_RUNNING_REPLACE] = true;
  ACO[ERR_EML_RESULT_REPLACE] = true;
  ACO[ERR_EML_SCORE_REPLACE] = true;
  ACO[ERR_EML_SCORE_IMP_REPLACE] = true;
  ACO[ERR_REC_RESULT_REPLACE] = true;
  ACO[ERR_REC_SCORE_REPLACE] = true;
  ACO[ERR_REC_SCORE_IMP_REPLACE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_TXT];

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_TXT];

  ACO = ActionCommentOK[BRIDGE_REF_REPLACE_WORD];
  ACO[ERR_PBN_PLAY_REPLACE] = true;
  ACO[ERR_PBN_ALERT_REPLACE] = true;
  ACO[ERR_TXT_PLAY_REPLACE] = true;
  ACO[ERR_EML_PLAY_REPLACE] = true;
  ACO[ERR_REC_PLAY_REPLACE] = true;

  ACO = ActionCommentOK[BRIDGE_REF_INSERT_WORD];

  ACO = ActionCommentOK[BRIDGE_REF_DELETE_WORD];
}


void RefComment::setTagTable()
{
  for (unsigned i = 0; i < BRIDGE_REF_FIX_SIZE; i++)
    for (unsigned j = 0; j < REF_TAGS_SIZE; j++)
      TagCommentOK[i][j] = false;

  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_FIRST] = true;
  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_LAST] = true;
  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_DELETE] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_DECL_PARD] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_DECL_OPP] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_DENOM] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_LEVEL] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_MULT] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_TRICKS] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_EMPTY] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_INCOMPLETE] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_PN][ERR_LIN_PN_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_PN][ERR_LIN_PN_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_PN][ERR_LIN_PN_DELETE] = true;

  TagCommentOK[REF_TAGS_LIN_QX][ERR_LIN_QX_REPLACE] = true;

  TagCommentOK[REF_TAGS_LIN_MD][ERR_LIN_MD_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_MD][ERR_LIN_MD_MISSING] = true;
  TagCommentOK[REF_TAGS_LIN_MD][ERR_LIN_MD_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_SV][ERR_LIN_SV_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_SV][ERR_LIN_SV_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_SV][ERR_LIN_SV_DELETE] = true;
  TagCommentOK[REF_TAGS_LIN_SV][ERR_LIN_SV_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_MB][ERR_LIN_MB_TRAILING] = true;
  TagCommentOK[REF_TAGS_LIN_MB][ERR_LIN_MB_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_MB][ERR_LIN_MB_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_MB][ERR_LIN_MB_DELETE] = true;
  TagCommentOK[REF_TAGS_LIN_MB][ERR_LIN_MB_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_AN][ERR_LIN_AN_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_AN][ERR_LIN_AN_DELETE] = true;

  TagCommentOK[REF_TAGS_LIN_PC][ERR_LIN_PC_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_PC][ERR_LIN_PC_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_PC][ERR_LIN_PC_DELETE] = true;
  TagCommentOK[REF_TAGS_LIN_PC][ERR_LIN_PC_SYNTAX] = true;
  TagCommentOK[REF_TAGS_LIN_PC][ERR_LIN_TRICK_DELETE] = true;

  TagCommentOK[REF_TAGS_LIN_MC][ERR_LIN_MC_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_MC][ERR_LIN_MC_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_MC][ERR_LIN_MC_DELETE] = true;
  TagCommentOK[REF_TAGS_LIN_MC][ERR_LIN_MC_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_PG][ERR_LIN_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_NT][ERR_LIN_NT_SYNTAX] = true;
  TagCommentOK[REF_TAGS_LIN_NT][ERR_LIN_SYNTAX] = true;

  TagCommentOK[REF_TAGS_PBN_SITE][ERR_PBN_SITE_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_ROOM][ERR_PBN_ROOM_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_PLAY][ERR_PBN_PLAYER_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_BOARD][ERR_PBN_BOARD_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_ROOM][ERR_PBN_ROOM_INSERT] = true;
  TagCommentOK[REF_TAGS_PBN_DECLARER][ERR_PBN_DECLARER_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_RESULT][ERR_PBN_RESULT_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_SCORE][ERR_PBN_SCORE_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_SCORE_IMP][ERR_PBN_SCORE_IMP_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_AUCTION][ERR_PBN_CALL_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_AUCTION][ERR_PBN_ALERT_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_AUCTION][ERR_PBN_ALERT_INSERT] = true;
  TagCommentOK[REF_TAGS_PBN_AUCTION][ERR_PBN_ALERT_DELETE] = true;
  TagCommentOK[REF_TAGS_PBN_AUCTION][ERR_PBN_HAND_AUCTION_LIVE] = true;
  TagCommentOK[REF_TAGS_PBN_NOTE][ERR_PBN_NOTE_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_NOTE][ERR_PBN_NOTE_INSERT] = true;
  TagCommentOK[REF_TAGS_PBN_NOTE][ERR_PBN_NOTE_DELETE] = true;
  TagCommentOK[REF_TAGS_PBN_PLAY][ERR_PBN_PLAY_REPLACE] = true;

  TagCommentOK[REF_TAGS_RBN_L][ERR_RBN_L_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_N][ERR_RBN_N_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_N][ERR_RBN_N_INSERT] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBN_P_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_R][ERR_RBN_R_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_A][ERR_RBN_HAND_AUCTION_LIVE] = true;

  TagCommentOK[REF_TAGS_RBN_L][ERR_RBX_L_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_N][ERR_RBX_N_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_N][ERR_RBX_N_INSERT] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBX_P_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_R][ERR_RBX_R_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_A][ERR_RBX_HAND_AUCTION_LIVE] = true;
}


void RefComment::parse(
  const string& refName,
  const string& line,
  const unsigned start,
  unsigned& end)
{
  // Set commentFlag, category and count1..3.
  // Modifies end.

  unsigned openb = line.find_last_of("{");
  if (openb == string::npos)
    return;
  if (openb < start)
    THROW("Ref file " + refName + ": Odd opening brace in '" + line + "'");

  end = openb-1;

  const regex rep("\\{(.*)\\((\\d+),(\\d+),(\\d+)\\)\\}\\s*$");
  smatch match;
  const string str = line.substr(openb);
  if (! regex_search(str, match, rep) || match.size() < 4)
    THROW("Ref file " + refName + ": Bad comment in '" + line + "'");

  auto it = CommentMap.find(match.str(1));
  if (it == CommentMap.end())
    THROW("Ref file " + refName + ": Bad comment name in '" + line + "'");

  setFlag = true;

  category = it->second;

  if (! str2unsigned(match.str(2), count1))
    THROW("Ref file " + refName + ": Bad comment no1 in '" + line + "'");
  if (! str2unsigned(match.str(3), count2))
    THROW("Ref file " + refName + ": Bad comment no2 in '" + line + "'");
  if (! str2unsigned(match.str(4), count3))
    THROW("Ref file " + refName + ": Bad comment no3 in '" + line + "'");
  
  fileName = refName;
  quote = line.substr(openb);
}


void RefComment::checkAction(const ActionType action) const
{
  if (! setFlag)
    return;

  if (! ActionCommentOK[action][category])
    THROW("Ref file " + fileName + ":\n" + 
      ActionList[action].name + "\n" +
      "comment '" + quote + "'\n" +
      CommentList[category].name + "\n" +
      "Action and comment don't match");
}


void RefComment::checkTag(const string& tag) const
{
  if (tag == "")
    return;

  auto it = TagMap.find(tag);
  if (it == TagMap.end())
  {
    // Try lower-case as well.
    string taglc = tag;
    toLower(taglc);
    it = TagMap.find(taglc);
    if (it == TagMap.end())
      THROW("Ref file " + fileName + ":\n" + 
        " quote '" + quote + "'" + 
        " tag name not found: " + tag);
  }

  if (! TagCommentOK[it->second][category])
    THROW("Ref file " + fileName + ":\n" +
      "comment '" + quote + "'" + "\n" +
      CommentList[category].name + "\n" +
      "tag " + tag + "\n" + 
      "Tag and comment don't match");
}


bool RefComment::isCommented() const
{
  return setFlag;
}


bool RefComment::isUncommented() const
{
  return ! setFlag;
}


string RefComment::str() const
{
  if (! setFlag)
    return "";

  stringstream ss;
  ss << setw(14) << "Count detail" << CommentList[category].name << "\n";
  ss << setw(14) << "Count tags" << count1 << "\n";
  ss << setw(14) << "Count hands" << count2 << "\n";
  ss << setw(14) << "Count boards" << count3 << "\n";
  return ss.str();
}

