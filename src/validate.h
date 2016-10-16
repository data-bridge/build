/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALIDATE_H
#define BRIDGE_VALIDATE_H

#include <string>
#include "bconst.h"

using namespace std;


// Types of differences that can occur.

enum ValDiffs
{
  // This group of errors covers different or missing headers.
  BRIDGE_VAL_TITLE = 0,
  BRIDGE_VAL_DATE = 1,
  BRIDGE_VAL_LOCATION = 2,
  BRIDGE_VAL_EVENT = 3,
  BRIDGE_VAL_SESSION = 4,
  BRIDGE_VAL_BOARDS_HEADER = 5,
  BRIDGE_VAL_SCORING = 6,
  BRIDGE_VAL_TEAMS = 7,
  BRIDGE_VAL_NAMES = 8,

  BRIDGE_VAL_NAMES_SHORT = 9,
  BRIDGE_VAL_TXT_ALL_PASS = 10,
  BRIDGE_VAL_LIN_EXCLAIM = 11,
  BRIDGE_VAL_LIN_PLAY_NL = 12,
  BRIDGE_VAL_PLAY_SHORT = 13,
  BRIDGE_VAL_REC_MADE_32 = 14,
  BRIDGE_VAL_VG_CHAT = 15,
  BRIDGE_VAL_RECORD_NUMBER = 16,

  BRIDGE_VAL_ERROR = 17,
  BRIDGE_VAL_OUT_SHORT = 18,
  BRIDGE_VAL_REF_SHORT = 19,
  BRIDGE_VAL_SIZE = 20
};

const string valNames[] =
{
  "Title",
  "Date",
  "Location",
  "Event",
  "Session",
  "Board numbers",
  "Scoring",
  "Teams",
  "Names",

  "Names-short",
  "All-pass",
  "Lin-!",
  "Play-newline",
  "Play-short",
  "Made-32",
  "VG-chat",
  "Rec-comment",

  "Error",
  "Out-short",
  "Ref-short"
};

const string valNamesShort[] =
{
  "T",
  "D",
  "L",
  "E",
  "S",
  "Bnos",
  "F",
  "K",
  "N",

  "Nsht",
  "Apass",
  "Alert",
  "Pline",
  "Psht",
  "R32",
  "Chat",
  "%",

  "Error",
  "Osht",
  "Rsht"
};


// File-level stats.

struct ValStatType
{
  unsigned numFiles;

  unsigned numIdentical;
  unsigned numExpectedDiffs;
  unsigned numErrors;

  unsigned details[BRIDGE_VAL_SIZE];
};


void validate(
  const string& fileOut,
  const string& fileRef,
  const formatType fOrig,
  const formatType fRef,
  const OptionsType& options,
  ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE]);

void printOverallStats(
  const ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE],
  const bool detailsFlag);

#endif
