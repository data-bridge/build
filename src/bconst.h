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


// Bridge file formats.

enum Format
{
  BRIDGE_FORMAT_LIN = 0,
  BRIDGE_FORMAT_LIN_RP = 1, // A la Pavlicek
  BRIDGE_FORMAT_LIN_VG = 2, // A la BBO Vugraph
  BRIDGE_FORMAT_LIN_TRN = 3, // A la BBO tournament play
  BRIDGE_FORMAT_PBN = 4,
  BRIDGE_FORMAT_RBN = 5,
  BRIDGE_FORMAT_RBX = 6,
  BRIDGE_FORMAT_TXT = 7,
  BRIDGE_FORMAT_EML = 8,
  BRIDGE_FORMAT_REC = 9,
  BRIDGE_FORMAT_PAR = 10, // Not a real file format
  BRIDGE_FORMAT_SIZE = 11
};


const string FORMAT_NAMES[BRIDGE_FORMAT_SIZE] =
{
  "LIN",
  "LIN-RP",
  "LIN-VG",
  "LIN-TRN",
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


// Bridge format labels.

enum Label
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


// Input options.

struct FileOption
{
  bool setFlag;
  string name;
};

struct Options
{
  FileOption fileInput; // -i, --infile
  FileOption dirInput; // -I, --indir

  FileOption fileOutput; // -o, --outfile
  FileOption dirOutput; // -O, --outdir

  FileOption fileRef; // -r, --reffile
  FileOption dirRef; // -R, --refdir

  FileOption fileDigest; // -d, --digfile
  FileOption dirDigest; // -D, --digdir

  FileOption fileLog; // -l, --logfile

  bool tableIMPFlag; // -T, --tableIMP
  bool compareFlag; // -c, --compare
  bool playersFlag; // -p, --players
  bool equalFlag; // -e, --equal
  bool valuationFlag; // -V, --valuation
  bool solveFlag; // -S, --solve
  bool traceFlag; // -T, --trace

  bool formatSetFlag; // -f, --format
  Format format;

  bool statsFlag; // -s, --stats
  bool quoteFlag; // -q, --quote

  unsigned numThreads;

  bool verboseIO;
  bool verboseThrow;
  bool verboseBatch;

  bool verboseValStats;
  bool verboseValDetails;
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


#define STR(x) \
        static_cast<ostringstream*>(&(ostringstream() << x))->str()

// http://stackoverflow.com/a/4030983/211160
// Use to indicate a variable is being intentionally not referred to (which
// usually generates a compiler warning)
#ifndef UNUSED
  #define UNUSED(x) ((void)(true ? 0 : ((x), void(), 0)))
#endif

// Macro to help in concatening strings for output.

#define CONCAT(tp, tmp, ext) \
  tmp = ext; strcpy(tp, tmp.c_str()); tp += tmp.length()

#endif

