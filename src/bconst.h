/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CONSTANTS_H
#define BRIDGE_CONSTANTS_H

#include <string>
#include <vector>

using namespace std;

#define MAXNOOFCORES 12

#define BRIDGE_PLAYERS 4
#define BRIDGE_SUITS 4
#define BRIDGE_DENOMS 5
#define BRIDGE_TRICKS 13
#define BRIDGE_VULS 4

#define Max(x, y) (((x) >= (y)) ? (x) : (y))
#define Min(x, y) (((x) <= (y)) ? (x) : (y))


// Bridge file formats.

enum formatType
{
  BRIDGE_FORMAT_LIN = 0,
  BRIDGE_FORMAT_LIN_RP = 1, // A la Pavlicek
  BRIDGE_FORMAT_LIN_VG = 2, // A la BBO Vugraph
  BRIDGE_FORMAT_LIN_TRN = 3, // A la BBO tournament play
  BRIDGE_FORMAT_LIN_EXT = 4, // Extended
  BRIDGE_FORMAT_PBN = 5,
  BRIDGE_FORMAT_RBN = 6,
  BRIDGE_FORMAT_RBX = 7,
  BRIDGE_FORMAT_TXT = 8,
  BRIDGE_FORMAT_EML = 9,
  BRIDGE_FORMAT_REC = 10,
  BRIDGE_FORMAT_PAR = 11, // Not a real file format -- eliminate?
  BRIDGE_FORMAT_SIZE = 12
};

typedef formatType Format;


const string FORMAT_NAMES[BRIDGE_FORMAT_SIZE] =
{
  "LIN",
  "LIN-RP",
  "LIN-VG",
  "LIN-TRN",
  "LIN-EXT",
  "PBN", 
  "RBN", 
  "RBX", 
  "TXT", 
  "EML",
  "REC",
  "PAR"
};

const string FORMAT_EXTENSIONS[BRIDGE_FORMAT_SIZE] =
{
  "LIN",
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

// Bridge format labels.

enum formatLabelType
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
  BRIDGE_FORMAT_WEST = 12,
  BRIDGE_FORMAT_NORTH = 13,
  BRIDGE_FORMAT_EAST = 14,
  BRIDGE_FORMAT_SOUTH = 15,
  BRIDGE_FORMAT_SCORES_LIST = 16,
  BRIDGE_FORMAT_BOARDS_LIST = 17,

  BRIDGE_FORMAT_BOARD_NO = 18,
  BRIDGE_FORMAT_PLAYERS_BOARD = 19,
  BRIDGE_FORMAT_ROOM = 20,
  BRIDGE_FORMAT_DEAL = 21,
  BRIDGE_FORMAT_DEALER = 22,
  BRIDGE_FORMAT_VULNERABLE = 23,
  BRIDGE_FORMAT_AUCTION = 24,
  BRIDGE_FORMAT_DECLARER = 25,
  BRIDGE_FORMAT_CONTRACT = 26,
  BRIDGE_FORMAT_PLAY = 27,
  BRIDGE_FORMAT_RESULT = 28,
  BRIDGE_FORMAT_SCORE = 29,
  BRIDGE_FORMAT_SCORE_IMP = 30,
  BRIDGE_FORMAT_SCORE_MP = 31,

  BRIDGE_FORMAT_DOUBLE_DUMMY = 32,
  BRIDGE_FORMAT_LABELS_SIZE = 33
};

const string formatLabelNames[] =
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
  "West",
  "North",
  "East",
  "South",
  "Scores list",
  "Boards list",

  "Board number",
  "Players",
  "Room",
  "Deal",
  "Dealer",
  "Vulnerable",
  "Auction",
  "Declarer",
  "Contract",
  "Play",
  "Result",
  "Score",
  "Score IMP",
  "Score MP"
};


// Useful for write functions.

struct writeInfoType
{
  unsigned bno;
  unsigned ino;
  unsigned numBoards;
  unsigned numInst;

  string namesOld[2];

  int score1;
  int score2;
};


// Input options.

struct FileOptionType
{
  bool setFlag;
  string name;
};

struct OptionsType
{
  FileOptionType fileInput; // -i, --infile
  FileOptionType dirInput; // -I, --indir

  FileOptionType fileOutput; // -o, --outfile
  FileOptionType dirOutput; // -O, --outdir

  FileOptionType fileRef; // -r, --reffile
  FileOptionType dirRef; // -R, --refdir

  FileOptionType fileLog; // -l, --logfile

  bool formatSetFlag; // -f, --format
  formatType format;
};


struct FileOutputTaskType
{
  string fileOutput;
  formatType formatOutput;

  bool refFlag;
  string fileRef;
};


struct FileTaskType
{
  string fileInput;
  formatType formatInput;
  bool removeOutputFlag;

  vector<FileOutputTaskType> taskList;
};


#define STR(x) \
        static_cast<ostringstream*>(&(ostringstream() << x))->str()


// This is the same encoding as in DDS.

enum vulType
{
  BRIDGE_VUL_NONE = 0,
  BRIDGE_VUL_BOTH = 1,
  BRIDGE_VUL_NORTH_SOUTH = 2,
  BRIDGE_VUL_EAST_WEST = 3,
  BRIDGE_VUL_SIZE = 4
};

enum multiplierType
{
  BRIDGE_MULT_UNDOUBLED = 0,
  BRIDGE_MULT_DOUBLED = 1,
  BRIDGE_MULT_REDOUBLED = 2
};

// This is the same encoding as in DDS.

enum playerType
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

typedef playerType Player;


const string PLAYER_NAMES_LONG[BRIDGE_PLAYER_SIZE] =
{
  "North", "East", "South", "West", "NS", "EW", "none"
};

const string PLAYER_NAMES_SHORT[BRIDGE_PLAYER_SIZE] =
{
  "N", "E", "S", "W", "NS", "EW", "-"
};

const playerType PLAYER_LIN_TO_DDS[BRIDGE_PLAYERS] =
{
  BRIDGE_SOUTH, BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST
};

const unsigned PLAYER_DDS_TO_LIN_DEALER[BRIDGE_PLAYERS] =
{
  3, 4, 1, 2
};

const playerType PLAYER_RBN_TO_DDS[BRIDGE_PLAYERS] =
{
  BRIDGE_NORTH, BRIDGE_SOUTH, BRIDGE_EAST, BRIDGE_WEST
};

// This is the same encoding as in DDS.

enum denomType
{
  BRIDGE_SPADES = 0,
  BRIDGE_HEARTS = 1,
  BRIDGE_DIAMONDS = 2,
  BRIDGE_CLUBS = 3,
  BRIDGE_NOTRUMP = 4
};

const denomType DENOM_RBN_TO_DDS[BRIDGE_DENOMS] =
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

enum roomType
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

