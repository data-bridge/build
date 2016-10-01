/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <regex>

#include "Group.h"
#include "Segment.h"
#include "Debug.h"
#include "fileRBN.h"
#include "parse.h"
#include "portab.h"

using namespace std;

extern Debug debug;


enum RBNlabel
{
  RBN_TITLE_AND_AUTHOR = 0,
  RBN_DATE_AND_TIME = 1,
  RBN_LOCATION = 2,
  RBN_EVENT = 3,
  RBN_SESSION = 4,
  RBN_SCORING = 5,
  RBN_TEAMS = 6,
  RBN_PLAYERS = 7,
  RBN_BOARD_NO = 8,
  RBN_DEAL = 9,
  RBN_AUCTION = 10,
  RBN_CONTRACT = 11,
  RBN_PLAY = 12,
  RBN_RESULT = 13,
  RBN_DOUBLE_DUMMY = 14,
  RBN_LABELS_SIZE = 15 // Item list is not implemented
};

const string RBNname[] =
{
  "title",
  "date",
  "location",
  "event",
  "session",
  "scoring",
  "teams",
  "players",
  "board number",
  "deal",
  "auction",
  "contract",
  "play",
  "result",
  "tableau"
};


formatLabelType CHAR_TO_LABEL_NO[128];
bool CHAR_TO_NEW_SEGMENT[128];


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrRBN[RBN_LABELS_SIZE];
BoardPtr boardPtrRBN[RBN_LABELS_SIZE];

static bool tryRBNMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);


void setRBNtables()
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

  segPtrRBN[RBN_TITLE_AND_AUTHOR] = &Segment::SetTitle;
  segPtrRBN[RBN_DATE_AND_TIME] = &Segment::SetDate;
  segPtrRBN[RBN_LOCATION] = &Segment::SetLocation;
  segPtrRBN[RBN_EVENT] = &Segment::SetEvent;
  segPtrRBN[RBN_SESSION] = &Segment::SetSession;
  segPtrRBN[RBN_SCORING] = &Segment::SetScoring;
  segPtrRBN[RBN_TEAMS] = &Segment::SetTeams;

  segPtrRBN[RBN_PLAYERS] = &Segment::SetPlayers;
  segPtrRBN[RBN_BOARD_NO] = &Segment::SetNumber;
  
  boardPtrRBN[RBN_DEAL] = &Board::SetDeal;
  boardPtrRBN[RBN_AUCTION] = &Board::SetAuction;
  boardPtrRBN[RBN_CONTRACT] = &Board::SetContract;
  boardPtrRBN[RBN_PLAY] = &Board::SetPlays;
  boardPtrRBN[RBN_RESULT] = &Board::SetResult;
  boardPtrRBN[RBN_DOUBLE_DUMMY] = &Board::SetTableau;

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

    if (CHAR_TO_LABEL_NO[static_cast<int>(c)] == RBN_LABELS_SIZE)
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




bool readRBN(
  Group& group,
  const string& fname)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such RBN file");
    return false;
  }

  const formatType f = BRIDGE_FORMAT_RBN;

  vector<string> chunk(RBN_LABELS_SIZE);

  Segment * segment = nullptr;
  unsigned segno = 0;
  bool newSegFlag = false;

  Board * board = nullptr;
  unsigned bno = 0;

  unsigned lno = 0;

  while (readRBNChunk(fstr, lno, chunk, newSegFlag))
  {
    if (newSegFlag)
    {
      if (! group.MakeSegment(segno))
      {
        LOG("Cannot make segment " + STR(segno));
        fstr.close();
        return false;
      }

      segment = group.GetSegment(segno);
      segno++;
      bno = 0;
    }

    if (chunk[RBN_BOARD_NO] != "")
    {
      // New board
      board = segment->AcquireBoard(bno);
      bno++;

      if (board == nullptr)
      {
        LOG("Unknown error");
        fstr.close();
        return false;
      }
    }

    board->NewInstance();
    segment->CopyPlayers();

    for (unsigned i = 0; i < RBN_LABELS_SIZE; i++)
    {
      if (! tryRBNMethod(chunk, segment, board, i, fstr, RBNname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


static bool tryRBNMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= RBN_BOARD_NO)
  {
    if ((segment->*segPtrRBN[label])(chunk[label], BRIDGE_FORMAT_RBN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else  if ((board->*boardPtrRBN[label])(chunk[label], BRIDGE_FORMAT_RBN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk[label] + "'");
    fstr.close();
    return false;
  }
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

