/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <map>

#include <assert.h>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "fileLIN.h"
#include "parse.h"
#include "Debug.h"

using namespace std;

extern Debug debug;


map<string, formatLabelType> LINmap;


void setLINTables()
{
  LINmap["vg"] = BRIDGE_FORMAT_TITLE;
  LINmap["rs"] = BRIDGE_FORMAT_RESULTS_LIST;
  LINmap["pw"] = BRIDGE_FORMAT_PLAYERS_LIST;
  LINmap["px"] = BRIDGE_FORMAT_PLAYERS_HEADER;
  LINmap["mp"] = BRIDGE_FORMAT_SCORES_LIST;
  LINmap["bn"] = BRIDGE_FORMAT_BOARDS_LIST;
  LINmap["qx"] = BRIDGE_FORMAT_BOARD_NO;
  LINmap["pn"] = BRIDGE_FORMAT_PLAYERS_BOARD; // May also occur in header
  LINmap["md"] = BRIDGE_FORMAT_DEAL;
  LINmap["sv"] = BRIDGE_FORMAT_VULNERABLE;
  LINmap["mb"] = BRIDGE_FORMAT_AUCTION;
  LINmap["pc"] = BRIDGE_FORMAT_PLAY;
  LINmap["mc"] = BRIDGE_FORMAT_RESULT;

  // We ignore some labels.
  LINmap["pf"] = BRIDGE_FORMAT_LABELS_SIZE;
  LINmap["pg"] = BRIDGE_FORMAT_LABELS_SIZE;
  LINmap["st"] = BRIDGE_FORMAT_LABELS_SIZE;
  LINmap["rh"] = BRIDGE_FORMAT_LABELS_SIZE;
  LINmap["ah"] = BRIDGE_FORMAT_LABELS_SIZE;
  LINmap["nt"] = BRIDGE_FORMAT_LABELS_SIZE; // Chat
}


bool readLINChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

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
        chunk[BRIDGE_FORMAT_AUCTION] += "^" + STR(aNo);
        aNo++;
        continue;
      }

      auto it = LINmap.find(label);
      if (it == LINmap.end())
      {
        LOG("Illegal LIN label in line '" + line + "'");
        return false;
      }

      const unsigned labelNo = it->second;
      if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
        newSegFlag = true;

      // We ignore some labels.
      if (labelNo == BRIDGE_FORMAT_LABELS_SIZE)
        continue;

      if (labelNo == BRIDGE_FORMAT_PLAY)
      {
        // This is not rigorously correct
        if (cardCount > 0 && cardCount % 4 == 0)
          chunk[labelNo] += ":";

        cardCount += (value.size() > 2 ? 4u : 1u);

        chunk[labelNo] += value;
      }
      else if (labelNo == BRIDGE_FORMAT_AUCTION)
        chunk[labelNo] += value;
      else if (chunk[labelNo] == "")
        chunk[labelNo] = value;
      else
      {
        LOG("Label already set in line '" + line + "'");
        return false;
      }
    }
  }

  if (alerts.str() != "")
    chunk[BRIDGE_FORMAT_AUCTION] += "\n" + alerts.str();
  return qxSeen;
}


void writeHeader(
  ofstream& fstr,
  Group& group,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      break;
    
    case BRIDGE_FORMAT_LIN_RP:
      fstr << "% " << FORMAT_EXTENSIONS[group.GetInputFormat()] << " " <<
        GuessOriginalLine(group.GetFileName(), group.GetCount()) << "\n";
      fstr << "% www.rpbridge.net Richard Pavlicek\n";
      break;

    case BRIDGE_FORMAT_LIN_VG:
      break;

    case BRIDGE_FORMAT_LIN_TRN:
      break;

    default:
      break;
  }
}


void writeLINSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const formatType f)
{
  fstr << segment->TitleAsString(f);
  fstr << segment->ContractsAsString(f);
  fstr << segment->PlayersAsString(f);
  fstr << segment->ScoresAsString(f);
  fstr << segment->BoardsAsString(f);
}


struct writeInfoType
{
  unsigned bno;
  unsigned ino;
  unsigned numBoards;
  unsigned numInst;

  string names;
  string namesOld;

  unsigned score1;
  unsigned score2;
};


void writeLINBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  const writeInfoType& writeInfo,
  const formatType f)
{
  fstr << segment->NumberAsString(f, writeInfo.bno);

  if (f == BRIDGE_FORMAT_LIN || f == BRIDGE_FORMAT_LIN_TRN)
    fstr << board->PlayersAsString(f);

  fstr << board->DealAsString(board->GetDealer(), f);
  fstr << segment->NumberAsBoardString(f, writeInfo.bno);
  fstr << board->VulAsString(f);

  board->CalculateScore();

  fstr << board->AuctionAsString(f);
  fstr << board->PlayAsString(f);
  fstr << board->ClaimAsString(f);
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

  writeInfoType writeInfo;
  writeHeader(fstr, group, f);

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    writeLINSegmentLevel(fstr, segment, f);

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

        writeInfo.bno = b;
        writeLINBoardLevel(fstr, segment, board, writeInfo, f);
      }
    }
  }

  fstr.close();
  return true;
}

