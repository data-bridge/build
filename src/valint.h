/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALINT_H
#define BRIDGE_VALINT_H

#include <string>

using namespace std;

class Buffer;


const string ValErrorName[] =
{
  "Title",
  "Date",
  "Location",
  "Event",
  "Session",
  "Players list",
  "Board numbers",
  "Scores list",
  "Scoring",
  "Teams",
  "Room",
  "Deal",
  "Vul",
  "Auction",
  "Play",
  "Score",
  "Names-short",
  "TXT-dashes",
  "VG-cards",
  "VG-chat",

  "All-pass",
  "Lin-players2",
  "Lin-rh-ah",
  "Lin-!",
  "Lin-no-st",
  "Lin-no-sv",
  "Play-newline",
  "Play-short",
  "Made-32",
  "TXT-result",
  "Rec-comment",

  "Error",
  "Out-short",
  "Ref-short",
  "VG-mc",
};

const string ValErrorNameShort[] =
{
  "T",
  "D",
  "L",
  "E",
  "S",
  "Plist",
  "Bnos",
  "Slist",
  "F",
  "K",
  "Room",
  "Deal",
  "vul",
  "Auct",
  "Play",
  "Scor",
  "Nsht",
  "Dash",
  "Hlen",
  "Chat",

  "Apass",
  "Psrep",
  "ah-rh",
  "Alert",
  "No-st",
  "No-sv",
  "Pline",
  "Psht",
  "R32",
  "RTXT",
  "Comm",

  "Error",
  "Osht",
  "Rsht",
  "mc"
};


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

#endif
