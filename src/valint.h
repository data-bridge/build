/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

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
  "Board numbers",
  "Scoring",
  "Teams",
  "Names-short",
  "TXT-dashes",
  "VG-chat",

  "All-pass",
  "Lin-!",
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
  "Bnos",
  "F",
  "K",
  "Nsht",
  "Dash",
  "Chat",

  "Apass",
  "Alert",
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
