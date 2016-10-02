/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
// #include <sstream>
// #include <string>
// #include <vector>
#include <regex>
#include <assert.h>

#include "Group.h"
#include "Segment.h"
#include "fileRBX.h"
#include "parse.h"
#include "Debug.h"

using namespace std;

extern Debug debug;


formatLabelType CHAR_TO_LABEL_NO_RBX[128];
bool CHAR_TO_NEW_SEGMENT_RBX[128];


void setRBXTables()
{
  for (unsigned char c = 0; c < 128; c++)
  {
    CHAR_TO_LABEL_NO_RBX[c] = BRIDGE_FORMAT_LABELS_SIZE;
    CHAR_TO_NEW_SEGMENT_RBX[c] = false;
  }

  // Segment-level
  CHAR_TO_LABEL_NO_RBX['T'] = BRIDGE_FORMAT_TITLE;
  CHAR_TO_LABEL_NO_RBX['D'] = BRIDGE_FORMAT_DATE;
  CHAR_TO_LABEL_NO_RBX['L'] = BRIDGE_FORMAT_LOCATION;
  CHAR_TO_LABEL_NO_RBX['E'] = BRIDGE_FORMAT_EVENT;
  CHAR_TO_LABEL_NO_RBX['S'] = BRIDGE_FORMAT_SESSION;
  CHAR_TO_LABEL_NO_RBX['F'] = BRIDGE_FORMAT_SCORING;
  CHAR_TO_LABEL_NO_RBX['K'] = BRIDGE_FORMAT_TEAMS;

  // Board-level but relevant in Segment
  CHAR_TO_LABEL_NO_RBX['N'] = BRIDGE_FORMAT_PLAYERS_BOARD;
  CHAR_TO_LABEL_NO_RBX['B'] = BRIDGE_FORMAT_BOARD_NO;

  // Purely Board-level
  CHAR_TO_LABEL_NO_RBX['H'] = BRIDGE_FORMAT_DEAL;
  CHAR_TO_LABEL_NO_RBX['A'] = BRIDGE_FORMAT_AUCTION;
  CHAR_TO_LABEL_NO_RBX['C'] = BRIDGE_FORMAT_CONTRACT;
  CHAR_TO_LABEL_NO_RBX['P'] = BRIDGE_FORMAT_PLAY;
  CHAR_TO_LABEL_NO_RBX['R'] = BRIDGE_FORMAT_RESULT;
  CHAR_TO_LABEL_NO_RBX['M'] = BRIDGE_FORMAT_DOUBLE_DUMMY;

  CHAR_TO_NEW_SEGMENT_RBX['T'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['D'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['L'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['E'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['S'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['F'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['K'] = true;
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

    line = regex_replace(line, re, "");

    if (c == '%')
      continue;

    const int ci = static_cast<int>(c);
    if (CHAR_TO_LABEL_NO_RBX[ci] == BRIDGE_FORMAT_LABELS_SIZE)
    {
      LOG("Illegal RBX label in line '" + line + "'");
      return false;
    }

    if (CHAR_TO_NEW_SEGMENT_RBX[ci])
      newSegFlag = true;

    const formatLabelType labelNo = CHAR_TO_LABEL_NO_RBX[ci];
    if (chunk[labelNo] != "")
    {
      LOG("Label already set in line '" + line + "'");
      return false;
    }

    chunk[labelNo] = value;
  }

  return (line.size() == 0);
}


void writeRBXSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const formatType f)
{
  fstr << segment->TitleAsString(f);
  fstr << segment->DateAsString(f);
  fstr << segment->LocationAsString(f);
  fstr << segment->EventAsString(f);
  fstr << segment->SessionAsString(f);
  fstr << segment->ScoringAsString(f);
  fstr << segment->TeamsAsString(f);
}


void writeRBXBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const formatType f)
{
  string names = board->PlayersAsString(f);
  if (names != writeInfo.namesOld[writeInfo.ino])
  {
    fstr << names;
    writeInfo.namesOld[writeInfo.ino] = names;
  }
        
  if (writeInfo.ino == 0)
  {
    fstr << segment->NumberAsString(f, writeInfo.bno);
    fstr << board->DealAsString(BRIDGE_WEST, f);
  }

  board->CalculateScore();

  fstr << board->AuctionAsString(f);
  fstr << board->ContractAsString(f);
  fstr << board->PlayAsString(f);
  fstr << board->ResultAsString(f, segment->ScoringIsIMPs());
  fstr << "\n";
}


bool writeRBX(
  Group& group,
  const string& fname)
{
assert(false);
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such RBX file");
    return false;
  }

  fstr << "%{RBX " << 
    GuessOriginalLine(group.GetFileName(), group.GetCount()) << "}";
  fstr << "%{www.rpbridge.net Richard Pavlicek}";

  const formatType f = BRIDGE_FORMAT_RBX;
  unsigned numOld = 2;
  vector<string> oldPlayers(numOld);
  oldPlayers[0] = "";
  oldPlayers[1] = "";

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << segment->TitleAsString(f);
    fstr << segment->DateAsString(f);
    fstr << segment->LocationAsString(f);
    fstr << segment->EventAsString(f);
    fstr << segment->SessionAsString(f);
    fstr << segment->ScoringAsString(f);
    fstr << segment->TeamsAsString(f);

    for (unsigned b = 0; b < segment->GetLength(); b++)
    {
      Board * board = segment->GetBoard(b);
      if (board == nullptr)
      {
        LOG("Invalid board");
        fstr.close();
        return false;
      }

      unsigned numInst = board->GetLength();
      if (numInst > numOld)
      {
        oldPlayers.resize(numInst);
        for (unsigned j = numOld; j < numInst; j++)
          oldPlayers[j] = "";
        numOld = numInst;
      }

      for (unsigned i = 0; i < board->GetLength(); i++)
      {
        if (! board->SetInstance(i))
        {
          LOG("Invalid instance");
          fstr.close();
          return false;
        }

        string names = board->PlayersAsString(f);
        if (names != oldPlayers[i])
        {
          fstr << names;
          oldPlayers[i] = names;
        }
        
        if (i == 0)
        {
          fstr << segment->NumberAsString(f, b);
          fstr << board->DealAsString(BRIDGE_WEST, f);
        }

        board->CalculateScore();

        fstr << board->AuctionAsString(f);
        fstr << board->ContractAsString(f);
        fstr << board->PlayAsString(f);
        fstr << board->ResultAsString(f, segment->ScoringIsIMPs());
        fstr << "\n";
      }
    }
  }

  fstr.close();
  return true;
}

