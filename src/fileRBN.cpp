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

#include "fileRBN.h"
#include "parse.h"
#include "portab.h"
#include "debug.h"

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


RBNlabel CHAR_TO_LABEL_NO[128];
bool CHAR_TO_NEW_SEGMENT[128];


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtr[RBN_LABELS_SIZE];
BoardPtr boardPtr[RBN_LABELS_SIZE];

bool tryMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const RBNlabel label,
  ifstream& fstr,
  const string& info);

bool readChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool newSegFlag);


void setRBNtables()
{
  for (char c = 0; c < 128; c++)
  {
    CHAR_TO_LABEL_NO[c] = RBN_LABELS_SIZE;
    CHAR_TO_NEW_SEGMENT[c] = false;
  }

  // Segment-level
  CHAR_TO_LABEL_NO['T'] = RBN_TITLE_AND_AUTHOR;
  CHAR_TO_LABEL_NO['D'] = RBN_DATE_AND_TIME;
  CHAR_TO_LABEL_NO['L'] = RBN_LOCATION;
  CHAR_TO_LABEL_NO['E'] = RBN_EVENT;
  CHAR_TO_LABEL_NO['S'] = RBN_SESSION;
  CHAR_TO_LABEL_NO['F'] = RBN_SCORING;
  CHAR_TO_LABEL_NO['K'] = RBN_TEAMS;

  // Board-level but relevant in Segment
  CHAR_TO_LABEL_NO['N'] = RBN_PLAYERS;
  CHAR_TO_LABEL_NO['B'] = RBN_BOARD_NO;

  // Purely Board-level
  CHAR_TO_LABEL_NO['H'] = RBN_DEAL;
  CHAR_TO_LABEL_NO['A'] = RBN_AUCTION;
  CHAR_TO_LABEL_NO['C'] = RBN_CONTRACT;
  CHAR_TO_LABEL_NO['P'] = RBN_PLAY;
  CHAR_TO_LABEL_NO['R'] = RBN_RESULT;
  CHAR_TO_LABEL_NO['M'] = RBN_DOUBLE_DUMMY;

  CHAR_TO_NEW_SEGMENT['T'] = true;
  CHAR_TO_NEW_SEGMENT['D'] = true;
  CHAR_TO_NEW_SEGMENT['L'] = true;
  CHAR_TO_NEW_SEGMENT['E'] = true;
  CHAR_TO_NEW_SEGMENT['S'] = true;
  CHAR_TO_NEW_SEGMENT['F'] = true;
  CHAR_TO_NEW_SEGMENT['K'] = true;

  segPtr[RBN_TITLE_AND_AUTHOR] = &Segment::SetTitle;
  segPtr[RBN_DATE_AND_TIME] = &Segment::SetDate;
  segPtr[RBN_LOCATION] = &Segment::SetLocation;
  segPtr[RBN_EVENT] = &Segment::SetEvent;
  segPtr[RBN_SESSION] = &Segment::SetSession;
  segPtr[RBN_SCORING] = &Segment::SetScoring;
  segPtr[RBN_TEAMS] = &Segment::SetTeams;

  segPtr[RBN_PLAYERS] = &Segment::SetPlayers;
  segPtr[RBN_BOARD_NO] = &Segment::SetNumber;
  
  boardPtr[RBN_DEAL] = &Board::SetDeal;
  boardPtr[RBN_AUCTION] = &Board::SetAuction;
  boardPtr[RBN_CONTRACT] = &Board::SetContract;
  boardPtr[RBN_PLAY] = &Board::SetPlays;
  boardPtr[RBN_RESULT] = &Board::SetResult;
  boardPtr[RBN_DOUBLE_DUMMY] = &Board::SetTableau;

}


bool readChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool newSegFlag)
{
  string line;
  newSegFlag = false;
  chunk.clear();

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
    if (CHAR_TO_LABEL_NO[c] == RBN_LABELS_SIZE)
    {
      LOG("Illegal RBN label in line '" + line + "'");
      return false;
    }

    if (CHAR_TO_NEW_SEGMENT[c])
      newSegFlag = true;

    const RBNlabel labelNo = CHAR_TO_LABEL_NO[c];
    if (chunk[labelNo] != "")
    {
      LOG("Label already set in line '" + line + "'");
      return false;
    }

    chunk[labelNo] = line;
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

  Segment * segment;
  unsigned segno = 0;
  bool newSegFlag;

  Board * board;
  unsigned bno = 0;

  unsigned lno = 0;

  // TODO
  // Have to deal with board instances
  // Need to set assorted vulnerabilities, dealers etc as well

  while (readChunk(fstr, lno, chunk, newSegFlag))
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

    board = segment->GetBoard(bno);
    bno++;
    if (board == nullptr)
    {
      LOG("Unknown error");
      fstr.close();
      return false;
    }

    for (unsigned i = 0; i < RBN_LABELS_SIZE; i++)
    {
      if (! tryMethod(chunk, segment, board, 
          static_cast<RBNlabel>(i), fstr, RBNname[i]))
        return false;
    }
  }

  fstr.close();
  return true;
}


bool tryMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const RBNlabel label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= RBN_BOARD_NO)
  {
    if ((segment->*segPtr[label])(chunk[label], BRIDGE_FORMAT_RBN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else  if ((board->*boardPtr[label])(chunk[label], BRIDGE_FORMAT_RBN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk[label] + "'");
    fstr.close();
    return false;
  }
}

