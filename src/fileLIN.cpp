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
#include <map>

#include "fileLIN.h"
#include "parse.h"
#include "portab.h"
#include "debug.h"

using namespace std;

extern Debug debug;


enum LINlabel
{
  LIN_TITLE = 0,
  LIN_RESULTS_LIST = 1,
  LIN_PLAYERS_LIST = 2,
  LIN_PLAYERS_HEADER = 3,
  LIN_SCORES_LIST = 4,
  LIN_BOARDS_LIST = 5,
  LIN_BOARD_NO = 6,
  LIN_PLAYERS_BOARD = 7,
  LIN_DEAL = 8,
  LIN_AUCTION = 9,
  LIN_PLAY = 10,
  LIN_RESULT = 11,
  LIN_LABELS_SIZE = 12
};

const string LINname[] =
{
  "title",
  "result list",
  "player list",
  "player header",
  "score list",
  "board list",
  "board number",
  "players",
  "deal",
  "auction",
  "play",
  "result",
};


map<string, LINlabel> LINmap;
bool LABEL_TO_NEW_SEGMENT[LIN_LABELS_SIZE];


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrLIN[LIN_LABELS_SIZE];
BoardPtr boardPtrLIN[LIN_LABELS_SIZE];

bool readLINChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

bool tryLINMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);


void setLINtables()
{
  LINmap["vg"] = LIN_TITLE;
  LINmap["rs"] = LIN_RESULTS_LIST;
  LINmap["pw"] = LIN_PLAYERS_LIST;
  LINmap["px"] = LIN_PLAYERS_HEADER;
  LINmap["mp"] = LIN_SCORES_LIST;
  LINmap["bn"] = LIN_BOARDS_LIST;
  LINmap["qx"] = LIN_BOARD_NO;
  LINmap["pn"] = LIN_PLAYERS_BOARD; // But may also occur in header
  LINmap["md"] = LIN_DEAL;
  LINmap["mb"] = LIN_AUCTION;
  LINmap["pc"] = LIN_PLAY;
  LINmap["mc"] = LIN_RESULT;

  
  LABEL_TO_NEW_SEGMENT[LIN_TITLE] = true;
  LABEL_TO_NEW_SEGMENT[LIN_RESULTS_LIST] = true;
  LABEL_TO_NEW_SEGMENT[LIN_PLAYERS_LIST] = true;
  LABEL_TO_NEW_SEGMENT[LIN_SCORES_LIST] = true;
  LABEL_TO_NEW_SEGMENT[LIN_BOARDS_LIST] = true;

  segPtrLIN[LIN_TITLE] = &Segment::SetTitle;
  segPtrLIN[LIN_RESULTS_LIST] = &Segment::SetResultsList;
  segPtrLIN[LIN_PLAYERS_LIST] = &Segment::SetPlayersHeader;
  segPtrLIN[LIN_SCORES_LIST] = &Segment::SetScoresList;
  segPtrLIN[LIN_BOARDS_LIST] = &Segment::SetBoardsList;

  segPtrLIN[LIN_BOARD_NO] = &Segment::SetNumber;
  segPtrLIN[LIN_PLAYERS_BOARD] = &Segment::SetPlayers;
  
  boardPtrLIN[LIN_DEAL] = &Board::SetDeal;
  boardPtrLIN[LIN_AUCTION] = &Board::SetAuction;
  boardPtrLIN[LIN_PLAY] = &Board::SetPlays;
  boardPtrLIN[LIN_RESULT] = &Board::SetResult;
}


bool readLINChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < LIN_LABELS_SIZE; i++)
    chunk[i] = "";

  string oneLiner;
  bool qxSeen = false;
  regex re("^(\\w\\w)|([^|]*)|");
  smatch match;

  while (getline(fstr, line))
  {
    lno++;
    if (line.empty())
      continue;
    
    const char c = line.at(0);
    if (c == '%')
      continue;
    else if (c == 'q')
      qxSeen = true;

    while (regex_search(line, match, re) && match.size() >= 2)
    {
      string label = match.str(1);
      const string value = match.str(2);

      // Artificial label to disambiguate.
      if (label == "pn" && ! qxSeen)
        label = "px";

      auto it = LINmap.find(label);
      if (it == LINmap.end())
      {
        LOG("Illegal LIN label in line '" + line + "'");
        return false;
      }

      const unsigned labelNo = it->second;

      if (LABEL_TO_NEW_SEGMENT[labelNo])
        newSegFlag = true;

      if (chunk[labelNo] != "")
      {
        LOG("Label already set in line '" + line + "'");
        return false;
      }

      chunk[labelNo] = value;
    }

    int i = fstr.peek();
    if (i == EOF)
      return qxSeen;
    else if (qxSeen && i == 0x71) // q
      return true;
  }
  return false;
}


bool readLIN(
  Group& group,
  const string& fname)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such LIN file");
    return false;
  }

  group.SetFileName(fname);

  const formatType f = BRIDGE_FORMAT_LIN;

  vector<string> chunk(LIN_LABELS_SIZE);

  Segment * segment = nullptr;
  unsigned segno = 0;
  bool newSegFlag = false;

  Board * board = nullptr;
  unsigned bno = 0;

  unsigned lno = 0;

  while (readLINChunk(fstr, lno, chunk, newSegFlag))
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

    if (chunk[LIN_BOARD_NO] != "")
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

    for (unsigned i = 0; i < LIN_LABELS_SIZE; i++)
    {
      if (! tryLINMethod(chunk, segment, board, i, fstr, LINname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


bool tryLINMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= LIN_BOARD_NO)
  {
    if ((segment->*segPtrLIN[label])(chunk[label], BRIDGE_FORMAT_LIN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else  if ((board->*boardPtrLIN[label])(chunk[label], BRIDGE_FORMAT_LIN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writeLIN(
  Group& group,
  const string& fname,
  const formatType f)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such LIN file");
    return false;
  }

  if (f == BRIDGE_FORMAT_LIN_RP)
  {
    fstr << "% LIN " <<
      GuessOriginalLine(group.GetFileName(), group.GetCount()) << "\n";
    fstr << "% www.rpbridge.net Richard Pavlicek\n";
  }

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << segment->TitleAsString(f);
    fstr << segment->ContractsAsString(f);
    fstr << segment->PlayersAsString(f);
    fstr << segment->ScoresAsString(f);
    fstr << segment->BoardsAsString(f);

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
      for (unsigned i = 0; i < numInst; i++)
      {
        if (! board->SetInstance(i))
        {
          LOG("Invalid instance");
          fstr.close();
          return false;
        }

        fstr << segment->NumberAsString(f, b);
        fstr << board->DealAsString(BRIDGE_WEST, f);
        fstr << board->VulAsString(f);

        board->CalculateScore();

        fstr << board->AuctionAsString(f);
        fstr << board->PlayAsString(f);
        fstr << board->ClaimAsString(f);
      }
    }
  }

  fstr.close();
  return true;
}

