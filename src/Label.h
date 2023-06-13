/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LABEL_H
#define BRIDGE_LABEL_H

#include <string>

using namespace std;


// Bridge format labels.

enum Label: unsigned
{
  BRIDGE_FORMAT_TITLE = 0,
  BRIDGE_FORMAT_DATE = 1,
  BRIDGE_FORMAT_LOCATION = 2,
  BRIDGE_FORMAT_EVENT = 3,
  BRIDGE_FORMAT_SESSION = 4,
  BRIDGE_FORMAT_SCORING = 5,
  BRIDGE_FORMAT_TEAMS = 6,
  BRIDGE_FORMAT_HOMETEAM = 7,
  BRIDGE_FORMAT_VISITTEAM = 8,

  BRIDGE_FORMAT_RESULTS_LIST = 9,
  BRIDGE_FORMAT_PLAYERS_LIST = 10,
  BRIDGE_FORMAT_PLAYERS_HEADER = 11,
  BRIDGE_FORMAT_SCORES_LIST = 12,
  BRIDGE_FORMAT_BOARDS_LIST = 13,

  BRIDGE_FORMAT_BOARD_NO = 14,
  BRIDGE_FORMAT_ROOM = 15,

  BRIDGE_FORMAT_DEAL = 16,
  BRIDGE_FORMAT_DEALER = 17,
  BRIDGE_FORMAT_VULNERABLE = 18,
  BRIDGE_FORMAT_SCORE_IMP = 19,
  BRIDGE_FORMAT_SCORE_MP = 20,
  BRIDGE_FORMAT_DOUBLE_DUMMY = 21,

  BRIDGE_FORMAT_PLAYERS = 22,
  BRIDGE_FORMAT_WEST = 23,
  BRIDGE_FORMAT_NORTH = 24,
  BRIDGE_FORMAT_EAST = 25,
  BRIDGE_FORMAT_SOUTH = 26,
  BRIDGE_FORMAT_AUCTION = 27,
  BRIDGE_FORMAT_DECLARER = 28,
  BRIDGE_FORMAT_CONTRACT = 29,
  BRIDGE_FORMAT_PLAY = 30,
  BRIDGE_FORMAT_RESULT = 31,
  BRIDGE_FORMAT_SCORE = 32,

  BRIDGE_FORMAT_LABELS_SIZE = 33
};

const string LABEL_NAMES[] =
{
  "Title",
  "Date",
  "Location",
  "Event",
  "Session",
  "Scoring",
  "Teams",
  "Home team",
  "Visit team",
  
  "Results list",
  "Players list",
  "Players header",
  "Scores list",
  "Boards list",

  "Board number",
  "Room",

  "Deal",
  "Dealer",
  "Vulnerable",
  "Score IMP",
  "Score MP",
  "Double dummy",

  "Players",
  "West",
  "North",
  "East",
  "South",
  "Auction",
  "Declarer",
  "Contract",
  "Play",
  "Result",
  "Score",
  "New segment"
};

#endif

