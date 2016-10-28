/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALINT_H
#define BRIDGE_VALINT_H

#include <string>

using namespace std;


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
  "Ref-short"
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
  "Rsht"
};


bool valProgress(
  ifstream& fstr,
  ValSide& side);

#endif
