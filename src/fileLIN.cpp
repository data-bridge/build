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
#include <assert.h>

#include "Board.h"
#include "Group.h"
#include "Segment.h"
#include "Debug.h"
#include "fileLIN.h"
#include "parse.h"
#include "portab.h"

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
  LIN_VULNERABLE = 9,
  LIN_AUCTION = 10,
  LIN_PLAY = 11,
  LIN_RESULT = 12,
  LIN_LABELS_SIZE = 13
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
  "vulnerable",
  "auction",
  "play",
  "result"
};


map<string, LINlabel> LINmap;
bool LABEL_TO_NEW_SEGMENT[LIN_LABELS_SIZE];


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrLIN[LIN_LABELS_SIZE];
BoardPtr boardPtrLIN[LIN_LABELS_SIZE];

static bool readLINChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag);

static bool tryLINMethod(
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
  LINmap["sv"] = LIN_VULNERABLE;
  LINmap["mb"] = LIN_AUCTION;
  LINmap["pc"] = LIN_PLAY;
  LINmap["mc"] = LIN_RESULT;

  // We ignore some labels.
  LINmap["pf"] = LIN_LABELS_SIZE;
  LINmap["pg"] = LIN_LABELS_SIZE;
  LINmap["st"] = LIN_LABELS_SIZE;
  LINmap["rh"] = LIN_LABELS_SIZE;
  LINmap["ah"] = LIN_LABELS_SIZE;
  LINmap["nt"] = LIN_LABELS_SIZE; // Chat

  
  LABEL_TO_NEW_SEGMENT[LIN_TITLE] = true;
  LABEL_TO_NEW_SEGMENT[LIN_RESULTS_LIST] = true;
  LABEL_TO_NEW_SEGMENT[LIN_PLAYERS_LIST] = true;
  LABEL_TO_NEW_SEGMENT[LIN_SCORES_LIST] = true;
  LABEL_TO_NEW_SEGMENT[LIN_BOARDS_LIST] = true;

  segPtrLIN[LIN_TITLE] = &Segment::SetTitle;
  segPtrLIN[LIN_RESULTS_LIST] = &Segment::SetResultsList;
  segPtrLIN[LIN_PLAYERS_LIST] = &Segment::SetPlayersList;
  segPtrLIN[LIN_PLAYERS_HEADER] = &Segment::SetPlayersHeader;
  segPtrLIN[LIN_SCORES_LIST] = &Segment::SetScoresList;
  segPtrLIN[LIN_BOARDS_LIST] = &Segment::SetBoardsList;

  segPtrLIN[LIN_BOARD_NO] = &Segment::SetNumber;
  segPtrLIN[LIN_PLAYERS_BOARD] = &Segment::SetPlayers;
  
  boardPtrLIN[LIN_DEAL] = &Board::SetDeal;
  boardPtrLIN[LIN_VULNERABLE] = &Board::SetVul;
  boardPtrLIN[LIN_AUCTION] = &Board::SetAuction;
  boardPtrLIN[LIN_PLAY] = &Board::SetPlays;
  boardPtrLIN[LIN_RESULT] = &Board::SetResult;
}


static bool readLINChunk(
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
  regex re("^(\\w\\w)\\|([^|]*)\\|");
  smatch match;

  bool doneFlag = false;
  unsigned cardCount = 0;
  stringstream alerts;
  unsigned aNo = 1;
  while (! doneFlag && getline(fstr, line))
  {
    lno++;

    if (! line.empty())
    {
      const char c = line.at(0);
      if (c == '%')
        continue;
      else if (c == 'q')
        qxSeen = true;
    }

    int i = fstr.peek();
    if (i == EOF || (qxSeen && i == 0x71)) // q
      doneFlag = true;

    if (line.empty())
      continue;
    
    while (regex_search(line, match, re) && match.size() >= 2)
    {
      string label = match.str(1);
      const string value = match.str(2);

      line = regex_replace(line, re, "");

      // Artificial label to disambiguate.
      if (label == "pn" && ! qxSeen)
        label = "px";

      if (label == "an")
      {
        alerts << aNo << " " << value << "\n";
        chunk[LIN_AUCTION] += "^" + STR(aNo);
        aNo++;
        continue;
      }

      auto it = LINmap.find(label);
      if (it == LINmap.end())
      {
cout << "LABEL " << label << endl;
        LOG("Illegal LIN label in line '" + line + "'");
        return false;
      }

      const unsigned labelNo = it->second;

      if (LABEL_TO_NEW_SEGMENT[labelNo])
        newSegFlag = true;

      // We ignore some labels.
      if (labelNo == LIN_LABELS_SIZE)
        continue;

      if (labelNo == LIN_PLAY)
      {
        // This is not rigorously correct
        if (cardCount > 0 && cardCount % 4 == 0)
          chunk[labelNo] += ":";

        cardCount += (value.size() > 2 ? 4 : 1);

        chunk[labelNo] += value;
      }
      else if (labelNo == LIN_AUCTION)
        chunk[labelNo] += value;
      else if (chunk[labelNo] == "")
        chunk[labelNo] = value;
      else
      {
cout << "LABEL repeat " << label << " in line " << lno << ", i " << i << endl;
cout << "line '" << line << "'\n";
cout << "doneFlag " << doneFlag << endl;
cout << "value '" << value << "'" << endl;
cout << "already '" << chunk[labelNo] << "'" << endl;
        LOG("Label already set in line '" + line + "'");
        return false;
      }
    }
  }

  if (alerts.str() != "")
    chunk[LIN_AUCTION] += "\n" + alerts.str();
  return qxSeen;
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
  bool newBoard;

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

    newBoard = false;
    if (chunk[LIN_BOARD_NO] != "" && chunk[LIN_BOARD_NO].at(0) != 'c')
    {
      // New board
      board = segment->AcquireBoard(bno);
      newBoard = true;
      bno++;

      if (board == nullptr)
      {
        LOG("Unknown error");
        fstr.close();
        return false;
      }
    }

    board->NewInstance();

    for (unsigned i = 0; i < LIN_LABELS_SIZE; i++)
    {
      if (! tryLINMethod(chunk, segment, board, i, fstr, LINname[i]))
        return false;
    }

    // Have to wait until after the methods with this.
    // if (! board->PlayersAreSet(board->GetInstance()))
    segment->TransferHeader(bno-1, board->GetInstance()); 

    // if (newBoard)
      // segment->TransferHeader(bno-1);

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


static bool tryLINMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= LIN_PLAYERS_BOARD)
  {
    if ((segment->*segPtrLIN[label])(chunk[label], BRIDGE_FORMAT_LIN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line '" + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else  if ((board->*boardPtrLIN[label])(chunk[label], BRIDGE_FORMAT_LIN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line '" + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writeLIN(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such LIN file");
    return false;
  }

  formatType f = BRIDGE_FORMAT_LIN;

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
        fstr << board->PlayersAsString(f);
        fstr << board->DealAsString(board->GetDealer(), f);
        fstr << segment->NumberAsBoardString(f, b);
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


bool writeLIN_RP(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such LIN file");
    return false;
  }

  formatType f = BRIDGE_FORMAT_LIN_RP;

  fstr << "% LIN " <<
    GuessOriginalLine(group.GetFileName(), group.GetCount()) << "\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << segment->TitleAsString(f);
    fstr << segment->ContractsAsString(f);
    fstr << segment->PlayersAsString(f) << "\n";
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
        fstr << board->DealAsString(board->GetDealer(), f);
        fstr << board->VulAsString(f);

        board->CalculateScore();

        fstr << board->AuctionAsString(f);
        fstr << board->PlayAsString(f);
        fstr << board->ClaimAsString(f) << "\n";
      }
    }
  }

  fstr.close();
  return true;
}


bool writeLIN_TRN(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such LIN file");
    return false;
  }

  formatType f = BRIDGE_FORMAT_LIN_TRN;

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << segment->TitleAsString(f);
    fstr << segment->ContractsAsString(f);
    fstr << segment->PlayersAsString(f);

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
        fstr << board->PlayersAsString(f);
        fstr << board->DealAsString(board->GetDealer(), f);
        fstr << segment->NumberAsBoardString(f, b);
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


bool writeLIN_VG(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such LIN file");
    return false;
  }

  formatType f = BRIDGE_FORMAT_LIN_VG;

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << segment->TitleAsString(f);
    fstr << segment->ContractsAsString(f);
    fstr << segment->PlayersAsString(f);

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
        fstr << board->DealAsString(board->GetDealer(), f);
        fstr << board->VulAsString(f);

        board->CalculateScore();

        fstr << board->AuctionAsString(f) << "\n";
        fstr << board->PlayAsString(f);
        fstr << board->ClaimAsString(f) << "\n";
      }
    }
  }

  fstr.close();
  return true;
}


