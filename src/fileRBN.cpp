/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Group.h"
#include "Segment.h"
#include "fileRBN.h"
#include "Debug.h"

using namespace std;

extern Debug debug;


formatLabelType CHAR_TO_LABEL_NO[128];
bool CHAR_TO_NEW_SEGMENT[128];


void setRBNTables()
{
  for (unsigned char c = 0; c < 128; c++)
  {
    CHAR_TO_LABEL_NO[c] = BRIDGE_FORMAT_LABELS_SIZE;
    CHAR_TO_NEW_SEGMENT[c] = false;
  }

  // Segment-level
  CHAR_TO_LABEL_NO['T'] = BRIDGE_FORMAT_TITLE;
  CHAR_TO_LABEL_NO['D'] = BRIDGE_FORMAT_DATE;
  CHAR_TO_LABEL_NO['L'] = BRIDGE_FORMAT_LOCATION;
  CHAR_TO_LABEL_NO['E'] = BRIDGE_FORMAT_EVENT;
  CHAR_TO_LABEL_NO['S'] = BRIDGE_FORMAT_SESSION;
  CHAR_TO_LABEL_NO['F'] = BRIDGE_FORMAT_SCORING;
  CHAR_TO_LABEL_NO['K'] = BRIDGE_FORMAT_TEAMS;

  // Board-level but relevant in Segment
  CHAR_TO_LABEL_NO['N'] = BRIDGE_FORMAT_PLAYERS_BOARD;
  CHAR_TO_LABEL_NO['B'] = BRIDGE_FORMAT_BOARD_NO;

  // Purely Board-level
  CHAR_TO_LABEL_NO['H'] = BRIDGE_FORMAT_DEAL;
  CHAR_TO_LABEL_NO['A'] = BRIDGE_FORMAT_AUCTION;
  CHAR_TO_LABEL_NO['C'] = BRIDGE_FORMAT_CONTRACT;
  CHAR_TO_LABEL_NO['P'] = BRIDGE_FORMAT_PLAY;
  CHAR_TO_LABEL_NO['R'] = BRIDGE_FORMAT_RESULT;
  CHAR_TO_LABEL_NO['M'] = BRIDGE_FORMAT_DOUBLE_DUMMY;

  CHAR_TO_NEW_SEGMENT['T'] = true;
  CHAR_TO_NEW_SEGMENT['D'] = true;
  CHAR_TO_NEW_SEGMENT['L'] = true;
  CHAR_TO_NEW_SEGMENT['E'] = true;
  CHAR_TO_NEW_SEGMENT['S'] = true;
  CHAR_TO_NEW_SEGMENT['F'] = true;
  CHAR_TO_NEW_SEGMENT['K'] = true;
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
    
    if (line.at(1) != ' ')
    {
      LOG("Need RBN space as second character in line '" + line + "'");
      return false;
    }

    const char c = line.at(0);
    if (c == '%')
      continue;

    if (CHAR_TO_LABEL_NO[static_cast<int>(c)] == BRIDGE_FORMAT_LABELS_SIZE)
    {
      LOG("Illegal RBN label in line '" + line + "'");
      return false;
    }

    if (CHAR_TO_NEW_SEGMENT[static_cast<int>(c)])
      newSegFlag = true;

    const formatLabelType labelNo = CHAR_TO_LABEL_NO[static_cast<int>(c)];
    if (chunk[labelNo] != "")
    {
      LOG("Label already set in line '" + line + "'");
      return false;
    }

    chunk[labelNo] = line.substr(2, string::npos);
  }
  return false;
}


bool writeRBN(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such RBN file");
    return false;
  }

  fstr << "% RBN\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  const formatType f = BRIDGE_FORMAT_RBN;
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

