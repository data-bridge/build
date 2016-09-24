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

#include "fileRBX.h"
#include "parse.h"
#include "portab.h"
#include "debug.h"

using namespace std;

extern Debug debug;


enum RBXlabel
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


RBXlabel CHAR_TO_LABEL_NO_RBX[128];
bool CHAR_TO_NEW_SEGMENT_RBX[128];


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrRBX[RBN_LABELS_SIZE];
BoardPtr boardPtrRBX[RBN_LABELS_SIZE];

bool readRBXChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool tryRBXMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);


void setRBXtables()
{
  for (unsigned char c = 0; c < 128; c++)
  {
    CHAR_TO_LABEL_NO_RBX[c] = RBN_LABELS_SIZE;
    CHAR_TO_NEW_SEGMENT_RBX[c] = false;
  }

  // Segment-level
  CHAR_TO_LABEL_NO_RBX['T'] = RBN_TITLE_AND_AUTHOR;
  CHAR_TO_LABEL_NO_RBX['D'] = RBN_DATE_AND_TIME;
  CHAR_TO_LABEL_NO_RBX['L'] = RBN_LOCATION;
  CHAR_TO_LABEL_NO_RBX['E'] = RBN_EVENT;
  CHAR_TO_LABEL_NO_RBX['S'] = RBN_SESSION;
  CHAR_TO_LABEL_NO_RBX['F'] = RBN_SCORING;
  CHAR_TO_LABEL_NO_RBX['K'] = RBN_TEAMS;

  // Board-level but relevant in Segment
  CHAR_TO_LABEL_NO_RBX['N'] = RBN_PLAYERS;
  CHAR_TO_LABEL_NO_RBX['B'] = RBN_BOARD_NO;

  // Purely Board-level
  CHAR_TO_LABEL_NO_RBX['H'] = RBN_DEAL;
  CHAR_TO_LABEL_NO_RBX['A'] = RBN_AUCTION;
  CHAR_TO_LABEL_NO_RBX['C'] = RBN_CONTRACT;
  CHAR_TO_LABEL_NO_RBX['P'] = RBN_PLAY;
  CHAR_TO_LABEL_NO_RBX['R'] = RBN_RESULT;
  CHAR_TO_LABEL_NO_RBX['M'] = RBN_DOUBLE_DUMMY;

  CHAR_TO_NEW_SEGMENT_RBX['T'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['D'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['L'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['E'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['S'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['F'] = true;
  CHAR_TO_NEW_SEGMENT_RBX['K'] = true;

  segPtrRBX[RBN_TITLE_AND_AUTHOR] = &Segment::SetTitle;
  segPtrRBX[RBN_DATE_AND_TIME] = &Segment::SetDate;
  segPtrRBX[RBN_LOCATION] = &Segment::SetLocation;
  segPtrRBX[RBN_EVENT] = &Segment::SetEvent;
  segPtrRBX[RBN_SESSION] = &Segment::SetSession;
  segPtrRBX[RBN_SCORING] = &Segment::SetScoring;
  segPtrRBX[RBN_TEAMS] = &Segment::SetTeams;

  segPtrRBX[RBN_PLAYERS] = &Segment::SetPlayers;
  segPtrRBX[RBN_BOARD_NO] = &Segment::SetNumber;
  
  boardPtrRBX[RBN_DEAL] = &Board::SetDeal;
  boardPtrRBX[RBN_AUCTION] = &Board::SetAuction;
  boardPtrRBX[RBN_CONTRACT] = &Board::SetContract;
  boardPtrRBX[RBN_PLAY] = &Board::SetPlays;
  boardPtrRBX[RBN_RESULT] = &Board::SetResult;
  boardPtrRBX[RBN_DOUBLE_DUMMY] = &Board::SetTableau;

}


bool readRBXChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < RBN_LABELS_SIZE; i++)
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

    if (CHAR_TO_LABEL_NO_RBX[c] == RBN_LABELS_SIZE)
    {
      LOG("Illegal RBN label in line '" + line + "'");
      return false;
    }

    if (CHAR_TO_NEW_SEGMENT_RBX[c])
      newSegFlag = true;

    const RBXlabel labelNo = CHAR_TO_LABEL_NO_RBX[c];
    if (chunk[labelNo] != "")
    {
      LOG("Label already set in line '" + line + "'");
      return false;
    }

    chunk[labelNo] = value;
  }

  return (line.size() == 0);
}




bool readRBX(
  Group& group,
  const string& fname)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such RBN file");
    return false;
  }

  group.SetFileName(fname);

  const formatType f = BRIDGE_FORMAT_RBN;

  vector<string> chunk(RBN_LABELS_SIZE);

  Segment * segment = nullptr;
  unsigned segno = 0;
  bool newSegFlag = false;

  Board * board = nullptr;
  unsigned bno = 0;

  unsigned lno = 0;

  while (readRBXChunk(fstr, lno, chunk, newSegFlag))
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
      if (! tryRBXMethod(chunk, segment, board, i, fstr, RBNname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


bool tryRBXMethod(
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
    if ((segment->*segPtrRBX[label])(chunk[label], BRIDGE_FORMAT_RBN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else  if ((board->*boardPtrRBX[label])(chunk[label], BRIDGE_FORMAT_RBN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writeRBX(
  Group& group,
  const string& fname)
{
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

