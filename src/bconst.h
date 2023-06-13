/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CONSTANTS_H
#define BRIDGE_CONSTANTS_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#include <vector>
#pragma warning(pop)

#include "Options.h"
#include "Format.h"
#include "Label.h"

using namespace std;


#define MAXNOOFCORES 12

#define BRIDGE_PLAYERS 4
#define BRIDGE_SUITS 4
#define BRIDGE_DENOMS 5
#define BRIDGE_TRICKS 13
#define BRIDGE_VULS 4

#define Max(x, y) (((x) >= (y)) ? (x) : (y))
#define Min(x, y) (((x) <= (y)) ? (x) : (y))

#define BIGNUM 9999999


const string FORMAT_EXTENSIONS[BRIDGE_FORMAT_SIZE] =
{
  "LIN",
  "LIN",
  "LIN",
  "LIN",
  "PBN", 
  "RBN", 
  "RBX", 
  "TXT", 
  "EML",
  "REC",
  "PAR"
};


const Format FORMAT_INPUT_MAP[] =
{
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_PBN,
  BRIDGE_FORMAT_RBN,
  BRIDGE_FORMAT_RBX,
  BRIDGE_FORMAT_TXT,
  BRIDGE_FORMAT_EML,
  BRIDGE_FORMAT_REC,
  BRIDGE_FORMAT_SIZE,
  BRIDGE_FORMAT_SIZE
};


const vector<Format> FORMAT_ACTIVE =
{
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN_RP,
  BRIDGE_FORMAT_LIN_VG,
  BRIDGE_FORMAT_LIN_TRN,
  BRIDGE_FORMAT_PBN,
  BRIDGE_FORMAT_RBN,
  BRIDGE_FORMAT_RBX,
  BRIDGE_FORMAT_TXT,
  BRIDGE_FORMAT_EML,
  BRIDGE_FORMAT_REC
};


// Useful for write functions.

struct WriteInfo
{
  unsigned bno;
  unsigned instNo;
  unsigned ino;
  unsigned numBoards;
  unsigned numInst;
  unsigned numInstActive;
  bool first;
  bool last;

  string namesOld[2];

  int score1;
  int score2;
};


struct FileOutputTask
{
  string fileOutput;
  Format formatOutput;

  bool refFlag;
  string fileRef;
};


struct FileTask
{
  string fileInput;
  Format formatInput;
  bool removeOutputFlag;

  vector<FileOutputTask> taskList;
};

enum DDInfoType
{
  BRIDGE_DD_INFO_SOLVE = 0,
  BRIDGE_DD_INFO_TRACE = 1,
  BRIDGE_DD_INFO_SIZE = 2
};

const string DDInfoNames[]
{
  "tableaux.log",
  "traces.log"
};


// This is used in dispatch.

struct Counts
{
  unsigned segno;
  unsigned bno;
  unsigned prevno;
  bool openFlag;
};

enum BoardOrder
{
  ORDER_OCOC = 0,
  ORDER_COCO = 1,
  ORDER_OOCC = 2,
  ORDER_GENERAL = 3
};

const string orderNames[] =
{
  "OCOC",
  "COCO",
  "OOCC",
  "General"
};


// This is used for header-level LIN data.

struct LINInstData // TODO: Can later be called LINData
{
  string contract;
  string players[4];
  string mp;
};

struct LINData
{
  LINInstData data[2];
  string no;
};


// This is used in Buffer and validate.

enum LineType
{
  BRIDGE_BUFFER_STRUCTURED = 0,
  BRIDGE_BUFFER_EMPTY = 1,
  BRIDGE_BUFFER_DASHES = 2,
  BRIDGE_BUFFER_COMMENT = 3,
  BRIDGE_BUFFER_GENERAL = 4,
  BRIDGE_BUFFER_SIZE = 5
};

struct LineData
{
  string line;
  unsigned len;
  unsigned no;
  LineType type;
  string label;
  string value;
};


// This is the same encoding as in DDS.

enum Vul
{
  BRIDGE_VUL_NONE = 0,
  BRIDGE_VUL_BOTH = 1,
  BRIDGE_VUL_NORTH_SOUTH = 2,
  BRIDGE_VUL_EAST_WEST = 3,
  BRIDGE_VUL_SIZE = 4
};

enum Multiplier
{
  BRIDGE_MULT_UNDOUBLED = 0,
  BRIDGE_MULT_DOUBLED = 1,
  BRIDGE_MULT_REDOUBLED = 2
};

// This is the same encoding as in DDS.

enum Player
{
  BRIDGE_NORTH = 0,
  BRIDGE_EAST = 1,
  BRIDGE_SOUTH = 2,
  BRIDGE_WEST = 3,
  BRIDGE_NORTH_SOUTH = 4,
  BRIDGE_EAST_WEST = 5,
  BRIDGE_PLAYER_NONE = 6,
  BRIDGE_PLAYER_SIZE = 7
};


const string PLAYER_NAMES_LONG[BRIDGE_PLAYER_SIZE] =
{
  "North", "East", "South", "West", "NS", "EW", "none"
};

const string PLAYER_NAMES_SHORT[BRIDGE_PLAYER_SIZE] =
{
  "N", "E", "S", "W", "NS", "EW", "-"
};

const Player PLAYER_LIN_TO_DDS[BRIDGE_PLAYERS] =
{
  BRIDGE_SOUTH, BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST
};

const Player PLAYER_DDS_TO_LIN[BRIDGE_PLAYERS] =
{
  BRIDGE_SOUTH, BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST
};

const Player PLAYER_DDS_TO_TXT[BRIDGE_PLAYERS] =
{
  BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST, BRIDGE_SOUTH
};

const unsigned PLAYER_DDS_TO_LIN_DEALER[BRIDGE_PLAYERS] =
{
  3, 4, 1, 2
};

const Player PLAYER_LIN_DEALER_TO_DDS[BRIDGE_PLAYERS+1] =
{
  BRIDGE_PLAYER_SIZE, BRIDGE_SOUTH, BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST 
};

const Player PLAYER_RBN_TO_DDS[BRIDGE_PLAYERS] =
{
  BRIDGE_NORTH, BRIDGE_SOUTH, BRIDGE_EAST, BRIDGE_WEST
};

// This is the same encoding as in DDS.

enum Denom
{
  BRIDGE_SPADES = 0,
  BRIDGE_HEARTS = 1,
  BRIDGE_DIAMONDS = 2,
  BRIDGE_CLUBS = 3,
  BRIDGE_NOTRUMP = 4
};

const Denom DENOM_RBN_TO_DDS[BRIDGE_DENOMS] =
{
  BRIDGE_NOTRUMP, 
  BRIDGE_SPADES, 
  BRIDGE_HEARTS, 
  BRIDGE_DIAMONDS, 
  BRIDGE_CLUBS
};

const char DENOM_NAMES_LONG[BRIDGE_DENOMS][9] =
{
  "Spades", "Hearts", "Diamonds", "Clubs", "NT"
};

const char DENOM_NAMES_SHORT[BRIDGE_DENOMS] =
{
  'S', 'H', 'D', 'C', 'N'
};

const string DENOM_NAMES_SHORT_PBN[BRIDGE_DENOMS] =
{
  "S", "H", "D", "C", "NT"
};

const string DENOM_NAMES_SHORT_RBN[BRIDGE_DENOMS] =
{
  "NT", "S", "H", "D", "C"
};

const string VUL_NAMES_LIN[BRIDGE_SUITS] =
{
  "o", "b", "n", "e"
};
const string VUL_NAMES_LIN_RP[BRIDGE_SUITS] =
{
  "0", "B", "N", "E"
};
const string VUL_NAMES_PBN[BRIDGE_SUITS] =
{
  "None", "All", "NS", "EW"
};
const string VUL_NAMES_RBN[BRIDGE_SUITS] =
{
  "Z", "B", "N", "E"
};

const string VUL_NAMES_TXT[BRIDGE_SUITS] =
{
  "None", "Both", "N-S", "E-W"
};

// TODO: For starters, to expand later.

enum PlayError
{
  BRIDGE_PLAY_WRONG_CARD = 0,
  BRIDGE_PLAY_SIZE = 1
};

enum Room
{
  BRIDGE_ROOM_OPEN = 0,
  BRIDGE_ROOM_CLOSED = 1,
  BRIDGE_ROOM_UNDEFINED = 2
};

const string DATE_MONTHS[] =
{
  "None",
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December"
};

#endif

