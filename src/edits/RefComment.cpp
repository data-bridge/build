/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <map>
#include <mutex>

#include "RefComment.h"

#include "Comment.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"

using namespace std;


struct ActionBundle
{
  ActionType val;
  string name;
};

const vector<ActionBundle> ActionList =
{
  {REF_ACTION_REPLACE_GEN, "replace"},
  {REF_ACTION_INSERT_GEN, "insert"},
  {REF_ACTION_DELETE_GEN, "delete"},

  {REF_ACTION_REPLACE_LIN, "replaceLIN"},
  {REF_ACTION_INSERT_LIN, "insertLIN"},
  {REF_ACTION_DELETE_LIN, "deleteLIN"},

  {REF_ACTION_REPLACE_PBN, "replacePBN"},
  {REF_ACTION_INSERT_PBN, "insertPBN"},
  {REF_ACTION_DELETE_PBN, "deletePBN"},

  {REF_ACTION_REPLACE_RBN, "replaceRBN"},
  {REF_ACTION_INSERT_RBN, "insertRBN"},
  {REF_ACTION_DELETE_RBN, "deleteRBN"},

  {REF_ACTION_REPLACE_RBX, "replaceRBX"},
  {REF_ACTION_INSERT_RBX, "insertRBX"},
  {REF_ACTION_DELETE_RBX, "deleteRBX"},

  {REF_ACTION_REPLACE_TXT, "replaceTXT"},
  {REF_ACTION_INSERT_TXT, "insertTXT"},
  {REF_ACTION_DELETE_TXT, "deleteTXT"},

  {REF_ACTION_REPLACE_WORD, "replaceWORD"},
  {REF_ACTION_INSERT_WORD, "insertWORD"},
  {REF_ACTION_DELETE_WORD, "deleteWORD"},

  {REF_ACTION_DELETE_EML, "deleteEML"}
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

static map<string, CommentType> CommentMap;
static map<string, RefTag> TagMap;
static bool ActionCommentOK[REF_ACTION_SIZE][ERR_SIZE];
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
  re.reset();

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
  for (unsigned i = 0; i < REF_ACTION_SIZE; i++)
    for (unsigned j = 0; j < ERR_SIZE; j++)
      ActionCommentOK[i][j] = false;
  
  bool * ACO = ActionCommentOK[REF_ACTION_REPLACE_GEN];
  ACO[ERR_LIN_VHEADER_SYNTAX] = true;
  ACO[ERR_LIN_PLAYERS_REPLACE] = true;
  ACO[ERR_LIN_RESULTS_REPLACE] = true;
  ACO[ERR_TXT_LOCATION_REPLACE] = true;
  ACO[ERR_TXT_PLAY_REPLACE] = true;
  ACO[ERR_TXT_RUNNING_REPLACE] = true;

  ACO = ActionCommentOK[REF_ACTION_INSERT_GEN];
  ACO[ERR_LIN_VHEADER_INSERT] = true;
  ACO[ERR_LIN_RESULTS_INSERT] = true;
  ACO[ERR_LIN_TRICK_INSERT] = true;
  ACO[ERR_PBN_PLAYER_INSERT] = true;
  ACO[ERR_REC_CONTRACT_INSERT] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_GEN];
  ACO[ERR_LIN_TRICK_LINE_DELETE] = true;
  ACO[ERR_LIN_AUCTION_DELETE] = true;
  ACO[ERR_LIN_HAND_AUCTION_LIVE] = true;
  ACO[ERR_LIN_HAND_CARDS_MISSING] = true;
  ACO[ERR_LIN_SYNTAX] = true;
  ACO[ERR_LIN_HAND_CARDS_WRONG] = true;
  ACO[ERR_PBN_AUCTION_DELETE] = true;
  ACO[ERR_PBN_PLAY_DELETE] = true;
  ACO[ERR_PBN_HAND_AUCTION_LIVE] = true;
  ACO[ERR_RBN_HAND_AUCTION_LIVE] = true;
  ACO[ERR_RBX_HAND_AUCTION_LIVE] = true;
  ACO[ERR_TXT_AUCTION_DELETE] = true;
  ACO[ERR_TXT_PLAY_DELETE] = true;
  ACO[ERR_TXT_HAND_AUCTION_LIVE] = true;
  ACO[ERR_EML_HAND_AUCTION_LIVE] = true;
  ACO[ERR_EML_LINE_DELETE] = true;
  ACO[ERR_REC_AUCTION_DELETE] = true;
  ACO[ERR_REC_PLAY_DELETE] = true;
  ACO[ERR_REC_HAND_AUCTION_LIVE] = true;

  ACO = ActionCommentOK[REF_ACTION_REPLACE_LIN];
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

  ACO = ActionCommentOK[REF_ACTION_INSERT_LIN];
  ACO[ERR_LIN_RS_INSERT] = true;
  ACO[ERR_LIN_PN_INSERT] = true;
  ACO[ERR_LIN_SV_INSERT] = true;
  ACO[ERR_LIN_MB_INSERT] = true;
  ACO[ERR_LIN_MB_SYNTAX] = true;
  ACO[ERR_LIN_PC_INSERT] = true;
  ACO[ERR_LIN_MC_INSERT] = true;
  ACO[ERR_LIN_TAG_SYNTAX] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_LIN];
  ACO[ERR_LIN_VG_SYNTAX] = true;
  ACO[ERR_LIN_RS_DELETE] = true;
  ACO[ERR_LIN_RS_DELETE_RANGE] = true;
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
  ACO[ERR_LIN_TAG_SYNTAX] = true;

  ACO = ActionCommentOK[REF_ACTION_REPLACE_PBN];
  ACO[ERR_PBN_SITE_REPLACE] = true;
  ACO[ERR_PBN_ROOM_REPLACE] = true;
  ACO[ERR_PBN_BOARD_REPLACE] = true;
  ACO[ERR_PBN_DECLARER_REPLACE] = true;
  ACO[ERR_PBN_RESULT_REPLACE] = true;
  ACO[ERR_PBN_SCORE_REPLACE] = true;
  ACO[ERR_PBN_SCORE_IMP_REPLACE] = true;

  ACO = ActionCommentOK[REF_ACTION_INSERT_PBN];
  ACO[ERR_PBN_ROOM_INSERT] = true;
  ACO[ERR_PBN_NOTE_INSERT] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_PBN];
  ACO[ERR_PBN_NOTE_DELETE] = true;
  ACO[ERR_PBN_SCORE_IMP_DELETE] = true;

  ACO = ActionCommentOK[REF_ACTION_REPLACE_RBN];
  ACO[ERR_RBN_P_REPLACE] = true;
  ACO[ERR_RBN_R_REPLACE] = true;

  ACO = ActionCommentOK[REF_ACTION_INSERT_RBN];
  ACO[ERR_RBN_N_INSERT] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_RBN];
  ACO[ERR_RBN_L_DELETE] = true;
  ACO[ERR_RBN_A_DELETE] = true;
  ACO[ERR_RBN_P_DELETE] = true;
  ACO[ERR_RBN_R_DELETE] = true;

  ACO = ActionCommentOK[REF_ACTION_REPLACE_RBX];
  ACO[ERR_RBX_P_REPLACE] = true;
  ACO[ERR_RBX_R_REPLACE] = true;

  ACO = ActionCommentOK[REF_ACTION_INSERT_RBX];
  ACO[ERR_RBX_N_INSERT] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_RBX];
  ACO[ERR_RBX_L_DELETE] = true;
  ACO[ERR_RBX_A_DELETE] = true;
  ACO[ERR_RBX_P_DELETE] = true;
  ACO[ERR_RBX_R_DELETE] = true;

  ACO = ActionCommentOK[REF_ACTION_REPLACE_TXT];
  ACO[ERR_TXT_DEALER_REPLACE] = true;
  ACO[ERR_TXT_RESULT_REPLACE] = true;
  ACO[ERR_TXT_SCORE_REPLACE] = true;
  ACO[ERR_TXT_SCORE_IMP_REPLACE] = true;
  ACO[ERR_TXT_RUNNING_REPLACE] = true;
  ACO[ERR_EML_RESULT_REPLACE] = true;
  ACO[ERR_EML_SCORE_REPLACE] = true;
  ACO[ERR_EML_SCORE_IMP_REPLACE] = true;
  ACO[ERR_REC_LEAD_REPLACE] = true;
  ACO[ERR_REC_RESULT_REPLACE] = true;
  ACO[ERR_REC_SCORE_REPLACE] = true;
  ACO[ERR_REC_SCORE_IMP_REPLACE] = true;

  ACO = ActionCommentOK[REF_ACTION_INSERT_TXT];
  ACO[ERR_EML_CONTRACT_INSERT] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_TXT];
  ACO[ERR_REC_SCORE_IMP_DELETE] = true;
  ACO[ERR_EML_PLAY_DELETE] = true;

  ACO = ActionCommentOK[REF_ACTION_REPLACE_WORD];
  ACO[ERR_PBN_PLAY_REPLACE] = true;
  ACO[ERR_PBN_CALL_REPLACE] = true;
  ACO[ERR_PBN_ALERT_REPLACE] = true;
  ACO[ERR_TXT_PLAY_REPLACE] = true;
  ACO[ERR_EML_PLAY_REPLACE] = true;
  ACO[ERR_REC_PLAY_REPLACE] = true;

  ACO = ActionCommentOK[REF_ACTION_INSERT_WORD];
  ACO[ERR_PBN_ALERT_INSERT] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_WORD];
  ACO[ERR_EML_PLAY_DELETE] = true;

  ACO = ActionCommentOK[REF_ACTION_DELETE_EML];
  ACO[ERR_EML_CANVAS_DELETE] = true;
}


void RefComment::setTagTable()
{
  for (unsigned i = 0; i < REF_ACTION_SIZE; i++)
    for (unsigned j = 0; j < REF_TAGS_SIZE; j++)
      TagCommentOK[i][j] = false;

  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_FIRST] = true;
  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_LAST] = true;
  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_VG][ERR_LIN_VG_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_REPLACE] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_INSERT] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_DELETE] = true;
  TagCommentOK[REF_TAGS_LIN_RS][ERR_LIN_RS_DELETE_RANGE] = true;
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

  TagCommentOK[REF_TAGS_LIN_PG][ERR_LIN_TAG_SYNTAX] = true;

  TagCommentOK[REF_TAGS_LIN_NT][ERR_LIN_NT_SYNTAX] = true;
  TagCommentOK[REF_TAGS_LIN_NT][ERR_LIN_TAG_SYNTAX] = true;

  TagCommentOK[REF_TAGS_PBN_SITE][ERR_PBN_SITE_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_ROOM][ERR_PBN_ROOM_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_PLAY][ERR_PBN_PLAYER_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_BOARD][ERR_PBN_BOARD_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_ROOM][ERR_PBN_ROOM_INSERT] = true;
  TagCommentOK[REF_TAGS_PBN_DECLARER][ERR_PBN_DECLARER_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_RESULT][ERR_PBN_RESULT_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_SCORE][ERR_PBN_SCORE_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_SCORE_IMP][ERR_PBN_SCORE_IMP_REPLACE] = true;
  TagCommentOK[REF_TAGS_PBN_SCORE_IMP][ERR_PBN_SCORE_IMP_DELETE] = true;
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
  TagCommentOK[REF_TAGS_RBN_A][ERR_RBN_A_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBN_P_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBN_P_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_R][ERR_RBN_R_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_R][ERR_RBN_R_DELETE] = true;

  TagCommentOK[REF_TAGS_RBN_L][ERR_RBX_L_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_N][ERR_RBX_N_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_N][ERR_RBX_N_INSERT] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBX_A_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBX_P_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_P][ERR_RBX_P_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_R][ERR_RBX_R_REPLACE] = true;
  TagCommentOK[REF_TAGS_RBN_R][ERR_RBX_R_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_A][ERR_RBX_A_DELETE] = true;
  TagCommentOK[REF_TAGS_RBN_A][ERR_RBX_HAND_AUCTION_LIVE] = true;
}


void RefComment::parse(
  const string& refName,
  const string& line,
  const size_t start,
  size_t& end)
{
  // Set commentFlag, category and count1..3.
  // Modifies end.

  size_t openb = line.find_last_of("{");
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

  unsigned count1, count2, count3;
  if (! str2unsigned(match.str(2), count1))
    THROW("Ref file " + refName + ": Bad comment no1 in '" + line + "'");
  if (! str2unsigned(match.str(3), count2))
    THROW("Ref file " + refName + ": Bad comment no2 in '" + line + "'");
  if (! str2unsigned(match.str(4), count3))
    THROW("Ref file " + refName + ": Bad comment no3 in '" + line + "'");

  re.set(count1, count2, count3);
  
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


RefTag RefComment::str2ref(const string& refstr) const
{
  if (refstr == "")
    return REF_TAGS_SIZE;

  auto it = TagMap.find(refstr);
  if (it == TagMap.end())
  {
    // Try lower-case as well.
    string reflc = refstr;
    toLower(reflc);
    it = TagMap.find(reflc);
    if (it == TagMap.end())
      THROW("Ref file " + fileName + ":\n" + 
        " quote '" + quote + "'" + 
        " ref name not found: " + refstr);
  }

  return it->second;
}


bool RefComment::isTag(const string& refstr) const
{
  if (refstr == "")
    return false;

  auto it = TagMap.find(refstr);
  if (it == TagMap.end())
  {
    // Try lower-case as well.
    string reflc = refstr;
    toLower(reflc);
    it = TagMap.find(reflc);
    if (it == TagMap.end())
      return false;
  }

  return true;
}


void RefComment::checkTag(const string& tagstr) const
{
  const RefTag tag = RefComment::str2ref(tagstr);
  if (tag == REF_TAGS_SIZE)
    return;

  if (! TagCommentOK[tag][category])
    THROW("Ref file " + fileName + ":\n" +
      "comment '" + quote + "'" + "\n" +
      CommentList[category].name + "\n" +
      "tag " + tagstr + "\n" + 
      "Tag and comment don't match");
}


RefCountType RefComment::countType() const
{
  if (! setFlag && category == ERR_SIZE)
    THROW("countType not available");

  return CommentList[category].countType;
}


CommentType RefComment::commentType() const
{
  return category;
}


Format RefComment::format() const
{
  if (! setFlag && category == ERR_SIZE)
    THROW("countType not available");

  return CommentList[category].format;
}


bool RefComment::isCommented() const
{
  return setFlag;
}


bool RefComment::isUncommented() const
{
  return ! setFlag;
}


void RefComment::getEntry(
  CommentType& cat,
  RefEntry& reIn) const
{
  if (! setFlag)
    THROW("RefComment not set");

  cat = category;

  reIn = re;
}


string RefComment::refFile() const
{
  return (setFlag ? fileName : "");
}


string RefComment::str() const
{
  if (! setFlag)
    return "";

  stringstream ss;
  ss << setw(14) << "Count detail" << CommentList[category].name << "\n";
  ss << re.strCount();
  return ss.str();
}


string RefComment::strComment() const
{
  if (! setFlag)
    return "";

  return "{" + CommentList[category].name + "(" +
    re.strCountShort() + ")}";
}
