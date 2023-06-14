/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef REF_CONST_H
#define REF_CONST_H

#include <vector>
#include <string>

#include "Format.h"
#include "Comment.h"

using namespace std;


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

#endif
