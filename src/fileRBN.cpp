/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <regex>

#include "Group.h"
#include "Segment.h"
#include "Bexcept.h"
#include "fileRBN.h"
#include "parse.h"

using namespace std;


formatLabelType RBNmap[128];


void setRBNTables()
{
  for (unsigned char c = 0; c < 128; c++)
    RBNmap[c] = BRIDGE_FORMAT_LABELS_SIZE;

  // Segment-level
  RBNmap['T'] = BRIDGE_FORMAT_TITLE;
  RBNmap['D'] = BRIDGE_FORMAT_DATE;
  RBNmap['L'] = BRIDGE_FORMAT_LOCATION;
  RBNmap['E'] = BRIDGE_FORMAT_EVENT;
  RBNmap['S'] = BRIDGE_FORMAT_SESSION;
  RBNmap['F'] = BRIDGE_FORMAT_SCORING;
  RBNmap['K'] = BRIDGE_FORMAT_TEAMS;

  // Board-level but relevant in Segment
  RBNmap['N'] = BRIDGE_FORMAT_PLAYERS_BOARD;
  RBNmap['B'] = BRIDGE_FORMAT_BOARD_NO;

  // Purely Board-level
  RBNmap['H'] = BRIDGE_FORMAT_DEAL;
  RBNmap['A'] = BRIDGE_FORMAT_AUCTION;
  RBNmap['C'] = BRIDGE_FORMAT_CONTRACT;
  RBNmap['P'] = BRIDGE_FORMAT_PLAY;
  RBNmap['R'] = BRIDGE_FORMAT_RESULT;
  RBNmap['M'] = BRIDGE_FORMAT_DOUBLE_DUMMY;
}


bool readRBNChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  while (getline(fstr, line))
  {
    lno++;
    if (line.empty())
      return true;
    
    const char c = line.at(0);
    if (c == '%')
      continue;

    const formatLabelType labelNo = RBNmap[static_cast<int>(c)];
    if (labelNo == BRIDGE_FORMAT_LABELS_SIZE)
      THROW("Illegal RBN label in line:\n" + line);

    if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
      newSegFlag = true;

    if (chunk[labelNo] != "")
      THROW("RBN label already set in line:\n" + line);

    if (line.length() < 2)
      chunk[labelNo] = "";
    else if (line.at(1) != ' ')
      THROW("Need RBN space as second character in line:\n" + line);
    else
      chunk[labelNo] = line.substr(2);
  }
  return false;
}


bool readRBXChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  if (! getline(fstr, line))
    return false;

  lno++;
  regex re("^(.)\\{([^\\}]*)\\}");
  smatch match;
  while (regex_search(line, match, re) && match.size() >= 2)
  {
    const char c = match.str(1).at(0);
    const string value = match.str(2);

    line = regex_replace(line, re, string(""));

    if (c == '%')
      continue;

    const formatLabelType labelNo = RBNmap[static_cast<int>(c)];
    if (labelNo == BRIDGE_FORMAT_LABELS_SIZE)
      THROW("Illegal RBX label in line:\n" + line);

    if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
      newSegFlag = true;

    if (chunk[labelNo] != "")
      THROW("RBN label already set in line:\n" + line);

    chunk[labelNo] = value;
  }

  return (line.size() == 0);
}


void writeRBNSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const Format format)
{
  fstr << segment->TitleAsString(format);
  fstr << segment->DateAsString(format);
  fstr << segment->LocationAsString(format);
  fstr << segment->EventAsString(format);
  fstr << segment->SessionAsString(format);
  fstr << segment->ScoringAsString(format);
  fstr << segment->TeamsAsString(format);
}


void writeRBNBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const Format format)
{
  string names = board->strPlayers(format);
  if (names != writeInfo.namesOld[writeInfo.ino])
  {
    fstr << names;
    writeInfo.namesOld[writeInfo.ino] = names;
  }
        
  if (writeInfo.ino == 0)
  {
    fstr << segment->NumberAsString(format, writeInfo.bno);
    fstr << board->strDeal(BRIDGE_WEST, format);
  }

  board->calculateScore();

  fstr << board->strAuction(format);
  fstr << board->strContract(format);
  fstr << board->strPlay(format);
  fstr << board->strResult(format, segment->ScoringIsIMPs());
  fstr << "\n";
}

