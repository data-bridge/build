/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CONSTANTS_H
#define BRIDGE_CONSTANTS_H

#include <string>

using namespace std;

#define MAXNOOFCORES 12

#define BRIDGE_PLAYERS 4
#define BRIDGE_SUITS 4
#define BRIDGE_DENOMS 5
#define BRIDGE_TRICKS 13
#define BRIDGE_VULS 4

#define Max(x, y) (((x) >= (y)) ? (x) : (y))
#define Min(x, y) (((x) <= (y)) ? (x) : (y))


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

enum formatType
{
  BRIDGE_FORMAT_LIN = 0,
  BRIDGE_FORMAT_PBN = 1,
  BRIDGE_FORMAT_RBN = 2,
  BRIDGE_FORMAT_TXT = 3,
  BRIDGE_FORMAT_PAR = 4,
  BRIDGE_FORMAT_EML = 5,
  BRIDGE_FORMAT_SIZE = 6
};

const string FORMAT_NAMES[BRIDGE_FORMAT_SIZE] =
{
  "LIN", "PBN", "RBN", "TXT", "PAR"
};

#endif

