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

#include "Refline.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


enum RefTags
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


static map<string, FixType> FixMap;
static string FixTable[BRIDGE_REF_FIX_SIZE];

static map<string, RefErrorsType> CommentMap;

typedef void (Refline::*ParsePtr)(
  const string& refName, 
  const string& quote);
static ParsePtr ParseList[BRIDGE_REF_FIX_SIZE];

typedef void (Refline::*ModifyPtr)(string& line) const;
static ModifyPtr ModifyList[BRIDGE_REF_FIX_SIZE];

static bool ActionCommentOK[BRIDGE_REF_FIX_SIZE][ERR_SIZE];

static map<string, RefTags> refTags;

static bool TagCommentOK[REF_TAGS_SIZE][ERR_SIZE];

static mutex mtx;
static bool setReflineTables = false;


Refline::Refline()
{
  Refline::reset();
  if (! setReflineTables)
  {
    mtx.lock();
    if (! setReflineTables)
      Refline::setTables();
    setReflineTables = true;
    mtx.unlock();
  }
}


Refline::~Refline()
{
}


void Refline::reset()
{
  setFlag = false;

  range.lno = 0;
  range.lcount = 0;

  fix = BRIDGE_REF_FIX_SIZE;

  edit.type = EDIT_TYPE_SIZE;
  edit.tagno = 0;
  edit.reverseFlag = false;
  edit.tagcount = 0;
  edit.fieldno = 0;
  edit.charno = 0;
  edit.was = "";
  edit.is = "";

  comment.setFlag = false;
  comment.category = ERR_SIZE;
  comment.count1 = 0;
  comment.count2 = 0;
  comment.count3 = 0;
}


void Refline::setFixTables()
{
  FixMap["replace"] = BRIDGE_REF_REPLACE_GEN;
  FixMap["insert"] = BRIDGE_REF_INSERT_GEN;
  FixMap["delete"] = BRIDGE_REF_DELETE_GEN;

  FixMap["replaceLIN"] = BRIDGE_REF_REPLACE_LIN;
  FixMap["insertLIN"] = BRIDGE_REF_INSERT_LIN;
  FixMap["deleteLIN"] = BRIDGE_REF_DELETE_LIN;

  FixMap["replacePBN"] = BRIDGE_REF_REPLACE_PBN;
  FixMap["insertPBN"] = BRIDGE_REF_INSERT_PBN;
  FixMap["deletePBN"] = BRIDGE_REF_DELETE_PBN;

  FixMap["replaceRBN"] = BRIDGE_REF_REPLACE_RBN;
  FixMap["insertRBN"] = BRIDGE_REF_INSERT_RBN;
  FixMap["deleteRBN"] = BRIDGE_REF_DELETE_RBN;

  FixMap["replaceRBX"] = BRIDGE_REF_REPLACE_RBX;
  FixMap["insertRBX"] = BRIDGE_REF_INSERT_RBX;
  FixMap["deleteRBX"] = BRIDGE_REF_DELETE_RBX;

  FixMap["replaceTXT"] = BRIDGE_REF_REPLACE_TXT;
  FixMap["insertTXT"] = BRIDGE_REF_INSERT_TXT;
  FixMap["deleteTXT"] = BRIDGE_REF_DELETE_TXT;

  FixMap["replaceWORD"] = BRIDGE_REF_REPLACE_WORD;
  FixMap["insertWORD"] = BRIDGE_REF_INSERT_WORD;
  FixMap["deleteWORD"] = BRIDGE_REF_DELETE_WORD;

  for (auto &s: FixMap)
    FixTable[s.second] = s.first;
}


void Refline::setRefTags()
{
  refTags["vg"] = REF_TAGS_LIN_VG;
  refTags["rs"] = REF_TAGS_LIN_RS;
  refTags["pw"] = REF_TAGS_LIN_PW;
  refTags["bn"] = REF_TAGS_LIN_BN;
  refTags["qx"] = REF_TAGS_LIN_QX;
  refTags["pn"] = REF_TAGS_LIN_PN;
  refTags["md"] = REF_TAGS_LIN_MD;
  refTags["sv"] = REF_TAGS_LIN_SV;
  refTags["mb"] = REF_TAGS_LIN_MB;
  refTags["an"] = REF_TAGS_LIN_AN;
  refTags["pc"] = REF_TAGS_LIN_PC;
  refTags["mc"] = REF_TAGS_LIN_MC;
  refTags["pg"] = REF_TAGS_LIN_PG;
  refTags["nt"] = REF_TAGS_LIN_NT;

  refTags["Site"] = REF_TAGS_PBN_SITE;
  refTags["Room"] = REF_TAGS_PBN_ROOM;
  refTags["Board"] = REF_TAGS_PBN_BOARD;
  refTags["Player"] = REF_TAGS_PBN_PLAYER;
  refTags["Auction"] = REF_TAGS_PBN_AUCTION;
  refTags["Declarer"] = REF_TAGS_PBN_DECLARER;
  refTags["Note"] = REF_TAGS_PBN_NOTE;
  refTags["Play"] = REF_TAGS_PBN_PLAY;
  refTags["Result"] = REF_TAGS_PBN_RESULT;
  refTags["Score"] = REF_TAGS_PBN_SCORE;
  refTags["ScoreIMP"] = REF_TAGS_PBN_SCORE_IMP;

  refTags["A"] = REF_TAGS_RBN_A;
  refTags["N"] = REF_TAGS_RBN_N;
  refTags["L"] = REF_TAGS_RBN_L;
  refTags["P"] = REF_TAGS_RBN_P;
  refTags["R"] = REF_TAGS_RBN_R;
}


void Refline::setDispatch()
{
  ParseList[BRIDGE_REF_REPLACE_GEN] = &Refline::parseReplaceGen;
  ParseList[BRIDGE_REF_INSERT_GEN] = &Refline::parseInsertGen;
  ParseList[BRIDGE_REF_DELETE_GEN] = &Refline::parseDeleteGen;

  ParseList[BRIDGE_REF_REPLACE_LIN] = &Refline::parseReplaceLIN;
  ParseList[BRIDGE_REF_INSERT_LIN] = &Refline::parseInsertLIN;
  ParseList[BRIDGE_REF_DELETE_LIN] = &Refline::parseDeleteLIN;

  ParseList[BRIDGE_REF_REPLACE_PBN] = &Refline::parseReplacePBN;
  ParseList[BRIDGE_REF_INSERT_PBN] = &Refline::parseInsertPBN;
  ParseList[BRIDGE_REF_DELETE_PBN] = &Refline::parseDeletePBN;

  ParseList[BRIDGE_REF_REPLACE_RBN] = &Refline::parseReplaceRBN;
  ParseList[BRIDGE_REF_INSERT_RBN] = &Refline::parseInsertRBN;
  ParseList[BRIDGE_REF_DELETE_RBN] = &Refline::parseDeleteRBN;

  ParseList[BRIDGE_REF_REPLACE_RBX] = &Refline::parseReplaceRBN;
  ParseList[BRIDGE_REF_INSERT_RBX] = &Refline::parseInsertRBN;
  ParseList[BRIDGE_REF_DELETE_RBX] = &Refline::parseDeleteRBN;

  ParseList[BRIDGE_REF_REPLACE_TXT] = &Refline::parseReplaceTXT;
  ParseList[BRIDGE_REF_INSERT_TXT] = &Refline::parseInsertTXT;
  ParseList[BRIDGE_REF_DELETE_TXT] = &Refline::parseDeleteTXT;

  ParseList[BRIDGE_REF_REPLACE_WORD] = &Refline::parseReplaceWORD;
  ParseList[BRIDGE_REF_INSERT_WORD] = &Refline::parseInsertWORD;
  ParseList[BRIDGE_REF_DELETE_WORD] = &Refline::parseDeleteWORD;


  ModifyList[BRIDGE_REF_REPLACE_GEN] = &Refline::modifyReplaceGen;
  ModifyList[BRIDGE_REF_INSERT_GEN] = &Refline::modifyInsertGen;
  ModifyList[BRIDGE_REF_DELETE_GEN] = &Refline::modifyDeleteGen;

  ModifyList[BRIDGE_REF_REPLACE_LIN] = &Refline::modifyReplaceLIN;
  ModifyList[BRIDGE_REF_INSERT_LIN] = &Refline::modifyInsertLIN;
  ModifyList[BRIDGE_REF_DELETE_LIN] = &Refline::modifyDeleteLIN;

  ModifyList[BRIDGE_REF_REPLACE_PBN] = &Refline::modifyReplacePBN;
  ModifyList[BRIDGE_REF_INSERT_PBN] = &Refline::modifyInsertPBN;
  ModifyList[BRIDGE_REF_DELETE_PBN] = &Refline::modifyDeletePBN;

  ModifyList[BRIDGE_REF_REPLACE_RBN] = &Refline::modifyReplaceRBN;
  ModifyList[BRIDGE_REF_INSERT_RBN] = &Refline::modifyInsertRBN;
  ModifyList[BRIDGE_REF_DELETE_RBN] = &Refline::modifyDeleteRBN;

  ModifyList[BRIDGE_REF_REPLACE_RBX] = &Refline::modifyReplaceRBX;
  ModifyList[BRIDGE_REF_INSERT_RBX] = &Refline::modifyInsertRBX;
  ModifyList[BRIDGE_REF_DELETE_RBX] = &Refline::modifyDeleteRBX;

  ModifyList[BRIDGE_REF_REPLACE_TXT] = &Refline::modifyReplaceTXT;
  ModifyList[BRIDGE_REF_INSERT_TXT] = &Refline::modifyInsertTXT;
  ModifyList[BRIDGE_REF_DELETE_TXT] = &Refline::modifyDeleteTXT;

  ModifyList[BRIDGE_REF_REPLACE_WORD] = &Refline::modifyReplaceWORD;
  ModifyList[BRIDGE_REF_INSERT_WORD] = &Refline::modifyInsertWORD;
  ModifyList[BRIDGE_REF_DELETE_WORD] = &Refline::modifyDeleteWORD;
}


void Refline::setCommentMap()
{
  for (auto &e: RefErrors)
    CommentMap[e.name] = e.val;
}


void Refline::setCommentAction()
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


void Refline::setCommentTag()
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


void Refline::setTables()
{
  Refline::setFixTables();
  Refline::setRefTags();
  Refline::setDispatch();
  Refline::setCommentMap();
  Refline::setCommentAction();
  Refline::setCommentTag();
}


bool Refline::isSpecial(const string& word) const
{
  if (word == "orderCOCO" ||
      word == "orderOOCC" ||
      word == "skip" ||
      word == "noval" ||
      word == "results")
    return true;
  else
    return false;
}


void Refline::parseRange(
  const string& refName,
  const string& line,
  const string& rtext,
  const unsigned start,
  const unsigned end)
{
  // Start with either upos (unsigned positive number) or upos-upos.
  // Set lno and count.

  if (start >= end)
    THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

  unsigned dash = rtext.find("-");
  if (dash == string::npos || dash == rtext.length()-1)
  {
    // upos.
    if (! str2upos(rtext, range.lno))
      THROW("Ref file " + refName + ": No line number in '" + line + "'");
    range.lcount = 1;
  }
  else
  {
    // upos-upos.
    if (! str2upos(rtext, range.lno))
      THROW("Ref file " + refName + ": No line number in '" + line + "'");
    unsigned c;
    if (! str2upos(rtext.substr(dash+1), c))
      THROW("Ref file " + refName + ": No end line in '" + line + "'");
    if (c <= range.lno)
      THROW("Ref file " + refName + ": Bad range in '" + line + "'");
    range.lcount = c + 1 - range.lno;
  }
}


FixType Refline::parseAction(
  const string& refName,
  const string& line,
  const string& action)
{
  // Set fix.

  auto it = FixMap.find(action);
  if (it == FixMap.end())
    THROW("Ref file " + refName + ": Bad action in '" + line + "'");

  return it->second;
}


void Refline::parseComment(
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

  comment.setFlag = true;

  comment.category = it->second;

  if (! str2unsigned(match.str(2), comment.count1))
    THROW("Ref file " + refName + ": Bad comment no1 in '" + line + "'");
  if (! str2unsigned(match.str(3), comment.count2))
    THROW("Ref file " + refName + ": Bad comment no2 in '" + line + "'");
  if (! str2unsigned(match.str(4), comment.count3))
    THROW("Ref file " + refName + ": Bad comment no3 in '" + line + "'");
}


bool Refline::parse(
  const string& refName,
  const string& line)
{
  if (setFlag)
    THROW("Refline already set: " + line);

  inputLine = line;

  string r, a, q;
  unsigned start = 0;
  unsigned end = line.length()-1;

  if (! readNextWord(line, 0, r))
    THROW("Ref file " + refName + ": Line start '" + line + "'");

  // If the first word is special, we kick it back upstairs.
  if (Refline::isSpecial(r))
    return false;

  // Otherwise it should be a number or a range.
  start = r.length()+1;
  Refline::parseRange(refName, line, r, start, end);

  // Then the action word, e.g. replaceLIN.
  if (! readNextWord(line, start, a))
    THROW("Ref file " + refName + ": No action in '" + line + "'");

  start += a.length()+1;
  fix = Refline::parseAction(refName, line, a);

  // Check whether there is a comment.
  Refline::parseComment(refName, line, start, end);

  if (start >= end)
  {
    setFlag = true;
    return true;
  }

  if (line.at(start) != '"')
    THROW("Ref file " + refName + ": No opening quote in '" + line + "'");

  while (line.at(end) != '"')
    end--;

  if (start == end)
    THROW("Ref file " + refName + ": No closing quote in '" + line + "'");

  // The details of the quoted string depend heavily on the action.
  q = line.substr(start+1, end-start-1);
  (this->*ParseList[fix])(refName, q);

  setFlag = true;
  return true;
}


void Refline::parseFlexibleNumber(
  const string& refName,
  const string& field)
{
  if (field.at(0) == '-')
  {
    // Permit tag counts from the back of the line as well.
    edit.reverseFlag = true;
    if (! str2upos(field.substr(1), edit.tagno))
      THROW("Ref file " + refName + ": Bad negative tag '" + field + "'");
  }
  else if (! str2upos(field, edit.tagno))
    THROW("Ref file " + refName + ": Bad tag number '" + field + "'");
}


string Refline::unquote(const string& entry) const
{
  const unsigned l = entry.length();
  if (l == 0)
    return "";

  if (entry.at(0) == '\'')
  {
    if (l == 1)
      THROW("Single quote");
    if (entry.at(l-1) != '\'')
      THROW("Not ending on single quote");
    if (l == 2)
      return "";
    else
      return entry.substr(1, l-2);
  }
  else
    return entry;
}


void Refline::commonCheck(
  const string& refName,
  const string& quote,
  const string& tag) const
{
  if (comment.setFlag && ! ActionCommentOK[fix][comment.category])
    THROW("Ref file " + refName + ": " + FixTable[fix] + 
        " comment '" + quote + "'\n" +
        RefErrors[comment.category].name + "\n");

  if (tag == "")
    return;

  auto it = refTags.find(tag);
  if (it == refTags.end())
  {
    // Try lower-case as well.
    string taglc = tag;
    toLower(taglc);
    it = refTags.find(taglc);
    if (it == refTags.end())
      THROW("Ref file " + refName + ": " + FixTable[fix] + 
          " quote '" + quote + "'" + " tag name " + tag);
  }

  if (! TagCommentOK[it->second][comment.category])
    THROW("Ref file " + refName + ": " + FixTable[fix] + 
        " combo '" + quote + "'" + "\n" +
        RefErrors[comment.category].name + "\n" +
        "tag " + tag + ", " + STR(it->second));
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseGen functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void Refline::parseReplaceGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": replace line count '" + quote + "'");

  if (comment.setFlag && 
      ! ActionCommentOK[BRIDGE_REF_REPLACE_GEN][comment.category])
    THROW("Ref file " + refName + ": replace comment name '" + quote + "'");

  edit.is = quote;
}


void Refline::parseInsertGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": insert line count '" + quote + "'");

  if (comment.setFlag && 
      ! ActionCommentOK[BRIDGE_REF_INSERT_GEN][comment.category])
    THROW("Ref file " + refName + ": insert comment name '" + quote + "'");

  edit.is = quote;
}


void Refline::parseDeleteGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": delete line count '" + quote + "'");

  if (comment.setFlag && 
      ! ActionCommentOK[BRIDGE_REF_DELETE_GEN][comment.category])
    THROW("Ref file " + refName + ": delete comment name '" + quote + "'");

  edit.is = quote;
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseLIN functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::parseReplaceLIN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen <= 1)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  Refline::parseFlexibleNumber(refName, v[0]);

  unsigned n = 1;
  if (str2upos(v[1], edit.fieldno))
    n = 2;

  if (vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[n]);
  edit.type = EDIT_TAG_ONLY;
  edit.tag = v[n];
  edit.was = Refline::unquote(v[n+1]);
  edit.is = Refline::unquote(v[n+2]);
}


void Refline::parseInsertLIN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen <= 1)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  Refline::parseFlexibleNumber(refName, v[0]);

  unsigned n = 1;
  if (str2upos(v[1], edit.fieldno))
    n = 2;

  if (vlen == n+1)
  {
    // Short version.  The inserted value may or may not be a tag.
    edit.type = EDIT_TAG_ONLY;
    edit.is = Refline::unquote(v[n]);
    return;
  }
  else if (vlen == n+2)
  {
    // Long version, "2,mb,p".
    Refline::commonCheck(refName, quote, v[n]);
    edit.type = EDIT_TAG_ONLY;
    edit.tag = v[n];
    edit.is = Refline::unquote(v[n+1]);
  }
  else
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");
}


void Refline::parseDeleteLIN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen == 0)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  Refline::parseFlexibleNumber(refName, v[0]);

  // delete "3" is allowed for desperate cases.
  if (vlen == 1)
    return;

  unsigned n = 1;
  if (str2upos(v[1], edit.fieldno))
    n = 2;

  if (vlen == n+1)
  {
    // Special case "deleteLIN "1,general text", for serious cases.
    Refline::commonCheck(refName, quote, "");
    edit.was = Refline::unquote(v[n]);
    return;
  }

  if (vlen != n+2 && vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[n]);
  edit.tag = v[n];
  edit.was = Refline::unquote(v[n+1]);

  // Kludge to recognize the case where an empty field is deleted.
  edit.is = (edit.was == "" ? "non-empty" : "");

  // deleteLIN "1,7,rs,3NW,4"
  // deleteLIN "3,mb,p,2"
  if (vlen == n+2)
    edit.type = EDIT_TAG_ONLY;
  else if (str2upos(v[n+2], edit.tagcount))
    edit.type = EDIT_TAG_FIELD;
  else
    THROW("Ref file " + refName + ": Bad tag/field count '" + quote + "'");
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// parsePBN functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::parseReplacePBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[0]);
  edit.type = EDIT_TAG_ONLY;
  edit.tag = v[0];
  edit.was = v[1];
  edit.is = v[2];
}


void Refline::parseInsertPBN(
  const string& refName,
  const string& quote)
{
  const size_t pos = quote.find(",");
  if (pos == 0 || pos == string::npos)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  const string t = quote.substr(0, pos);
  const string v = quote.substr(pos+1);
  Refline::commonCheck(refName, quote, t);
  edit.type = EDIT_TAG_ONLY;
  edit.tag = t;
  edit.is = v;
}


void Refline::parseDeletePBN(
  const string& refName,
  const string& quote)
{
  const size_t pos = quote.find(",");
  if (pos == 0 || pos == string::npos)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  const string t = quote.substr(0, pos);
  const string v = quote.substr(pos+1);
  Refline::commonCheck(refName, quote, t);
  edit.type = EDIT_TAG_ONLY;
  edit.tag = t;
  edit.was = v;
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseRBN functions (also RBX)                                      //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::parseReplaceRBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3 && vlen != 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[0]);
  edit.tag = v[0];
  if (vlen == 3)
  {
    edit.type = EDIT_TAG_ONLY;
    edit.was = v[1];
    edit.is = v[2];
  }
  else
  {
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");

    edit.was = v[2];
    edit.is = v[3];
  }
}


void Refline::parseInsertRBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3 && vlen != 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[0]);
  edit.tag = v[0];

  if (vlen == 3)
  {
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");

    edit.is = v[2];
  }
  else
  {
    // N,7,3,C.  Should only be used for RBX.
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.tagno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    if (! str2upos(v[2], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");

    edit.is = v[3];
  }
}


void Refline::parseDeleteRBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen < 1 && vlen > 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[0]);
  edit.type = EDIT_TAG_FIELD;
  edit.tag = v[0];
  if (vlen == 2)
    edit.was = v[1];
  else if (vlen == 3)
  {
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.was = v[2];
  }
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseTXT functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void Refline::parseReplaceTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen < 2 && vlen > 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  edit.type = EDIT_CHAR;
  if (vlen == 3)
  {
    if (! str2upos(v[0], edit.charno))
      THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

    edit.was = v[1];
    edit.is = v[2];
  }
  else
  {
    edit.was = v[0];
    edit.is = v[1];
  }
}


void Refline::parseInsertTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  edit.type = EDIT_CHAR;
  if (! str2upos(v[0], edit.charno))
    THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

  edit.is = v[1];
}


void Refline::parseDeleteTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 1 && vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  edit.type = EDIT_CHAR;
  if (vlen == 1)
    edit.is = v[0];
  else
  {
    if (! str2upos(v[0], edit.charno))
      THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

    edit.is = v[1];
  }
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseWORD functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void Refline::parseReplaceWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, "");
  edit.type = EDIT_WORD;
  if (! str2upos(v[0], edit.tagno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
    
  edit.was = v[1];
  edit.is = v[2];
}


void Refline::parseInsertWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[0]);
  edit.type = EDIT_WORD;
  if (! str2upos(v[0], edit.tagno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
  edit.is = v[1];
}


void Refline::parseDeleteWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  Refline::commonCheck(refName, quote, v[0]);
  edit.type = EDIT_WORD;
  if (! str2upos(v[0], edit.tagno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
  edit.was = v[1];
}


unsigned Refline::lineno() const
{
  return range.lno;
}


bool Refline::isSet() const
{
  return setFlag;
}


string Refline::line() const
{
  return inputLine;
}


bool Refline::isCommented() const
{
  return comment.setFlag;
}


string Refline::tag() const
{
  return edit.tag;
}


string Refline::is() const
{
  return edit.is;
}


string Refline::was() const
{
  return edit.was;
}


bool Refline::isUncommented() const
{
  return ! comment.setFlag;
}


FixType Refline::type() const
{
  return fix;
}


unsigned Refline::deletion() const
{
  return range.lcount;
}


void Refline::modifyFail(
  const string& line,
  const string& reason) const
{
   THROW("Modify fail: " + line + "\n" + reason + "\n\n" + Refline::str());
}


void Refline::modify(string& line) const
{
  if (! setFlag)
    THROW("Refline not set: " + line);

  (this->* ModifyList[fix])(line);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyGen functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::modifyReplaceGen(string& line) const
{
  line = edit.is;
}


void Refline::modifyInsertGen(string& line) const
{
  line = edit.is;
}


void Refline::modifyDeleteGen(string& line) const
{
  UNUSED(line);
  THROW("modifyDeleteGen not implemented");
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyLIN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::modifyLINCommon(
  const string& line,
  unsigned& start,
  vector<string>& v,
  vector<string>& f,
  bool& endsOnPipe) const
{
  v.clear();
  tokenizeMinus(line, v, "|");
  const unsigned vlen = v.size();

  if (edit.tagno == 0)
    modifyFail(line, "No tag number");

  if (2 * edit.tagno > vlen)
  {
    if (edit.is == "" && 
       edit.tag == "" && 
       2 * edit.tagno == vlen+1)
    {
      // Last tag, no argument.
    }
    else
      Refline::modifyFail(line, "Tag number too large");
  }

  endsOnPipe = (line.length() > 0 && line.at(line.length()-1) == '|');

  if (edit.reverseFlag)
    start = vlen - 2 * edit.tagno;
  else
    start = 2 * (edit.tagno-1);

  if (edit.fieldno > 0)
  {
    f.clear();
    tokenize(v[start+1], f, ",");

    if (edit.fieldno >= f.size()+1)
    {
      if (fix != BRIDGE_REF_INSERT_LIN)
        modifyFail(line, "Field too far");
      else if (edit.fieldno >= f.size()+2)
        modifyFail(line, "Insert field too far");
    }
  }
}


void Refline::modifyReplaceLIN(string& line) const
{
  unsigned start;
  vector<string> v, f;
  bool endsOnPipe;
  Refline::modifyLINCommon(line, start, v, f, endsOnPipe);

  if (v[start] != edit.tag)
    modifyFail(line, "Different LIN tags");

  if (edit.fieldno > 0)
  {
    if (f[edit.fieldno-1] != edit.was)
      modifyFail(line, "Old field wrong");

    f[edit.fieldno-1] = edit.is;
    v[start+1] = concat(f, ",");
  }
  else
  {
    if (v[start+1] != edit.was)
    {
      // Permit a not too short prefix.
      const unsigned l = edit.was.length();
      if (l < 2 ||
          l >= v[start+1].length() ||
          v[start+1].substr(0, l) != edit.was)
        modifyFail(line, "Old value wrong: " + v[start+1]);
    }
    v[start+1] = edit.is;
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


void Refline::modifyInsertLIN(string& line) const
{
  unsigned start;
  vector<string> v, f;
  bool endsOnPipe;
  Refline::modifyLINCommon(line, start, v, f, endsOnPipe);

  if (edit.fieldno > 0)
  {
    if (v[start] != edit.tag)
      modifyFail(line, "Different LIN tags");

    f.insert(f.begin() + static_cast<int>(edit.fieldno-1), edit.is);
    v[start+1] = concat(f, ",");
  }
  else if (edit.tag == "")
  {
    // Single insertion, i.e. could be a tag or a value.
    v.insert(v.begin() + static_cast<int>(start), edit.is);
  }
  else
  {
    v.insert(v.begin() + static_cast<int>(start), edit.is);
    v.insert(v.begin() + static_cast<int>(start), edit.tag);
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


void Refline::modifyDeleteLIN(string& line) const
{
  unsigned start;
  vector<string> v, f;
  bool endsOnPipe;
  Refline::modifyLINCommon(line, start, v, f, endsOnPipe);

  if (edit.fieldno > 0)
  {
    if (v[start] != edit.tag)
      modifyFail(line, "Different LIN tags");

    if (f[edit.fieldno-1] != edit.was)
      modifyFail(line, "Old field wrong");

    auto sf = f.begin() + static_cast<int>(edit.fieldno-1);
    const unsigned d = (edit.tagcount == 0 ? 1 : edit.tagcount);
    f.erase(sf, sf + static_cast<int>(d));

    v[start+1] = concat(f, ",");
  }
  else if (edit.fieldno == 0 && edit.tag == "")
  {
    // Delete a single entry without checking it.
    // Only use this when the entry is seriously messed up.
    if (edit.was != "" && v[start] != edit.was)
      modifyFail(line, "Old value wrong");
      
    v.erase(v.begin() + static_cast<int>(start));
  }
  else
  {
    if (v[start] != edit.tag)
      modifyFail(line, "Different LIN tags");
    
    if (edit.is == "" && edit.tag == "")
    {
      // Delete a single entry without checking it.
      // Only use this when the entry is seriously messed up.
      v.erase(v.begin() + static_cast<int>(start));
    }
    else
    {
      if (v[start+1] != edit.was)
        modifyFail(line, "Old value wrong");
      
      auto s = v.begin() + static_cast<int>(start);
      const unsigned d = (edit.tagcount == 0 ? 2 : 2*edit.tagcount);
      v.erase(s, s + static_cast<int>(d));
    }
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyPBN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::modifyReplacePBN(string& line) const
{
  const regex rep("^\\[(\\w+)\\s+\"(.*)\"\\]\\s*$");
  smatch match;
  if (! regex_search(line, match, rep))
    THROW("Bad PBN line: " + line);

  if (edit.tag != match.str(1))
    modifyFail(line, "Different PBN tags");

  if (edit.was != match.str(2))
    modifyFail(line, "Old value wrong");

  line = "[" + edit.tag + " \"" + edit.is + "\"" + "]";
}


void Refline::modifyInsertPBN(string& line) const
{
  line = "[" + edit.tag + " \"" + edit.is + "\"" + "]";
}


void Refline::modifyDeletePBN(string& line) const
{
  const regex rep("^\\[(\\w+)\\s+\"(.*)\"\\]\\s*$");
  smatch match;
  if (! regex_search(line, match, rep))
    THROW("Bad PBN line: " + line);

  if (edit.tag != match.str(1))
    modifyFail(line, "Different PBN tags");

  if (edit.was != match.str(2))
    modifyFail(line, "Old value wrong");

  line = "";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyRBN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void Refline::modifyRBNCommon(
  const string& line,
  string& s) const
{
  const unsigned l = line.length();
  if (l == 0)
    THROW("RBN line too short: " + line);

  if (line.substr(0, 1) != edit.tag)
    THROW("RBN tag wrong: " + line);

  if (l > 1 && line.at(1) != ' ')
    THROW("RBN syntax: " + line);

  if (l <= 2)
    s = "";
  else
    s = line.substr(2);
}


void Refline::modifyReplaceRBN(string& line) const
{
  string s;
  Refline::modifyRBNCommon(line, s);

  if (edit.fieldno == 0)
  {
    // R,5+400:+14,6+420:14
    if (s != edit.was)
      THROW("RBN old value: " + line);
    line = edit.tag + " " + edit.is;
  }
  else
  {
    // P,2,SJ235,SJ234
    vector<string> f;
    f.clear();
    tokenize(s, f, ":");
    const unsigned flen = f.size();
    if (edit.fieldno > flen)
      THROW("RBN field number too large: " + line);
    if (f[edit.fieldno-1] != edit.was)
      THROW("RBN old field value: " + line);
    f[edit.fieldno-1] = edit.is;
    line = edit.tag + " " + concat(f, ":");
  }
}

void Refline::modifyInsertRBN(string& line) const
{
  string s;
  Refline::modifyRBNCommon(line, s);

  if (edit.fieldno == 0)
    THROW("Not a field insertion: " + line);

  vector<string> f;
  f.clear();
  tokenize(s, f, ":");
  const unsigned flen = f.size();
  if (edit.fieldno > flen+1)
    THROW("RBN field number too large: " + line);
  else if (edit.fieldno == flen+1)
    f.push_back(edit.is);
  else
    f.insert(f.begin() + static_cast<int>(edit.fieldno-1), edit.is);
  line = edit.tag + " " + concat(f, ":");
}


void Refline::modifyDeleteRBN(string& line) const
{
  string s;
  if (line != "" || edit.was != "")
    Refline::modifyRBNCommon(line, s);

  if (edit.fieldno == 0)
  {
    // Could delete without checking.  Only use in dire cases.
    if (edit.was != "" && s != edit.was)
      THROW("RBN old value: " + line);
    line = ""; 
  }
  else
  {
    vector<string> f;
    f.clear();
    tokenize(s, f, ":");
    const unsigned flen = f.size();
    if (edit.fieldno > flen)
      THROW("RBN field number too large: " + line);
    if (f[edit.fieldno-1] != edit.was)
      THROW("RBN old field value: " + line);
    f.erase(f.begin() + static_cast<int>(edit.fieldno-1));
    line = edit.tag + " " + concat(f, ":");
  }
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyRBX functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


bool Refline::modifyCommonRBX(
  const string& line,
  vector<string>& v,
  string& s,
  unsigned& pos) const
{
  v.clear();
  tokenize(line, v, "}");
  v.pop_back(); // Last empty field
  const unsigned vlen = v.size();

  for (unsigned i = 0; i < vlen; i++)
  {
    const unsigned flen = v[i].length();
    if (flen <= 1)
      continue;

    if (v[i].substr(0, 1) == edit.tag && v[i].at(1) == '{')
    {
      pos = i;
      if (flen == 2)
        s = "";
      else
        s = edit.tag + " " + v[i].substr(2);
      return true;
    }
  }
  return false;
}


void Refline::modifyReplaceRBX(string& line) const
{
  vector<string> v;
  string s;
  unsigned pos;

  if (! Refline::modifyCommonRBX(line, v, s, pos))
    THROW("RBN tag not found: " + line);

  Refline::modifyReplaceRBN(s);
  if (s.length() <= 2)
    v[pos] = edit.tag + "{";
  else
    v[pos] = edit.tag + "{" + s.substr(2);

  line = concat(v, "}") + "}";
}


void Refline::modifyInsertRBX(string& line) const
{
  vector<string> v;
  string s;
  unsigned pos;

  if (Refline::modifyCommonRBX(line, v, s, pos))
  {
    // Insertion of a field in an existing tag.
    if (edit.fieldno == 0)
      THROW("RBX tag already present: " + line);
    Refline::modifyInsertRBN(s);
    if (s.length() <= 2)
      v[pos] = edit.tag + "{";
    else
      v[pos] = edit.tag + "{" + s.substr(2);
  }
  else
  {
    // Insertion of a new tag.
    if (edit.tagno > v.size()+1)
      THROW("RBX tag number too large: " + line);
    if (edit.tagno == 0)
      THROW("RBX tag number too small: " + line);
      
    s = edit.tag + "{" + edit.is;
    if (edit.tagno == v.size()+1)
      v.push_back(s);
    else
      v.insert(v.begin() + static_cast<int>(edit.tagno-1), s);
  }
  line = concat(v, "}") + "}";
}


void Refline::modifyDeleteRBX(string& line) const
{
  vector<string> v;
  string s;
  unsigned pos;

  if (! Refline::modifyCommonRBX(line, v, s, pos))
    THROW("RBN tag not found: " + line);

  Refline::modifyDeleteRBN(s);
  v.erase(v.begin() + static_cast<int>(pos));

  line = concat(v, "}") + "}";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyTXT functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

unsigned Refline::modifyCommonTXT(const string& line) const
{
  if (edit.charno == 0)
  {
    const unsigned p = line.find(edit.was);
    if (p == string::npos)
      return 0;
    else
      return p+1;
  }
  else
    return edit.charno; // As already 1-based
}


void Refline::modifyReplaceTXT(string& line) const
{
  const unsigned p = modifyCommonTXT(line);
  if (p == 0)
    THROW("No character position and string not found: " + line);

  unsigned lw = edit.was.length();
  unsigned li = edit.is.length();

  if (line.substr(p-1, lw) != edit.was)
    THROW("Old TXT value: " + line.substr(p-1, lw) + " vs " + edit.was);

  if (lw > li)
    line.erase(p-1, lw-li);
  else if (lw < li)
    line.insert(p-1, " ", li-lw);

  line.replace(p-1, li, edit.is);
}


void Refline::modifyInsertTXT(string& line) const
{
  if (edit.charno == 0)
    THROW("No character position");

  if (edit.charno > line.length())
    THROW("Character position too large");
  else if (edit.charno == line.length())
    line += edit.is;
  else
    line.insert(edit.charno, edit.is);
}


void Refline::modifyDeleteTXT(string& line) const
{
  const unsigned p = modifyCommonTXT(line);
  if (p == 0)
    THROW("No character position and string not found");

  if (line.substr(p-1, edit.was.length()) != edit.was)
    THROW("Old TXT value");

  line.erase(p-1, edit.was.length());
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyWORD functions                                               //
//                                                                    //
////////////////////////////////////////////////////////////////////////

unsigned Refline::modifyCommonWORD(const string& line) const
{
  unsigned pos = 0;
  const unsigned l = line.length();
  for (unsigned wno = 0; wno < edit.tagno; wno++)
  {
    while (pos < l && line.at(pos) == ' ')
      pos++;

    if (wno == edit.tagno-1)
      break;

    while (pos < l && line.at(pos) != ' ')
      pos++;
  }

  if (pos == l)
    return 0;
  else
    return pos+1;
}


void Refline::modifyReplaceWORD(string& line) const
{
  const unsigned pos = Refline::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "replaceWORD: Too few words");

  const unsigned lw = edit.was.length();
  const unsigned li = edit.is.length();

  if (line.substr(pos-1, lw) != edit.was)
    modifyFail(line, "Old value wrong");

  if (lw > li)
    line.erase(pos-1, lw-li);
  else if (lw < li)
    line.insert(pos-1, " ", li-lw);

  line.replace(pos-1, li, edit.is);
}


void Refline::modifyInsertWORD(string& line) const
{
  const unsigned pos = Refline::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "insertWORD: Too few words");

  line.insert(pos-1, edit.is);
}

void Refline::modifyDeleteWORD(string& line) const
{
  const unsigned pos = Refline::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "replaceWORD: Too few words");

  const unsigned lw = edit.was.length();
  if (line.substr(pos-1, lw) != edit.was)
    modifyFail(line, "Old value wrong");

  line.erase(pos-1, lw);
}


string Refline::str() const
{
  if (! setFlag)
    return "Refline not set\n";
  if (range.lno == 0)
    return "Line number not set\n";
    
  stringstream ss;
  ss << setw(14) << left << "Action" << FixTable[fix] << "\n";

  if (range.lcount <= 1)
    ss << setw(14) << "Line number" << range.lno << "\n";
  else
    ss << setw(14) << "Lines" << range.lno << " to " << 
      range.lno + range.lcount - 1 << "\n";

  if (edit.reverseFlag)
    ss << setw(14) << "Tag number" << edit.tagno << " (from the back)\n";
  else
    ss << setw(14) << "Tag number" << edit.tagno << "\n";

  ss << setw(14) << "Tag" << edit.tag << "\n";
  if (edit.fieldno > 0)
  {
    if (edit.tagcount == 0)
      ss << setw(14) << "Field" << edit.fieldno << "\n";
    else
      ss << setw(14) << "Fields" << edit.fieldno << " to " <<
        edit.fieldno + edit.tagcount - 1 << "\n";
  }

  if (edit.charno > 0)
    ss << setw(14) << "Char. number" << edit.charno << "\n";
  
  ss << setw(14) << "String was" << "'" << edit.was << "'" << "\n";
  ss << setw(14) << "String is" << "'" << edit.is << "'" << "\n";

  if (comment.setFlag)
  {
    ss << setw(14) << "Comment" << RefErrors[comment.category].name << "\n";
    ss << setw(14) << "Count tags" << comment.count1 << "\n";
    ss << setw(14) << "Count hands" << comment.count2 << "\n";
    ss << setw(14) << "Count boards" << comment.count2 << "\n";
  }

  return ss.str();
}

