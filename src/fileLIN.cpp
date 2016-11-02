/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <regex>
#include <map>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "fileLIN.h"
#include "Bexcept.h"

using namespace std;


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

      line = regex_replace(line, re, string(""));

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
        THROW("Illegal LIN label in line '" + line + "'");

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
        THROW("Label already set in line '" + line + "'");
    }
  }

  if (alerts.str() != "")
    chunk[BRIDGE_FORMAT_AUCTION] += "\n" + alerts.str();
  return qxSeen;
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


void writeLINBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const formatType f)
{
  fstr << segment->NumberAsString(f, writeInfo.bno);

  if (f == BRIDGE_FORMAT_LIN || f == BRIDGE_FORMAT_LIN_TRN)
    fstr << board->PlayersAsString(f);

  fstr << board->strDeal(f);
  fstr << segment->NumberAsBoardString(f, writeInfo.bno);
  fstr << board->strVul(f);

  board->calculateScore();

  fstr << board->strAuction(f);
  fstr << board->PlayAsString(f);
  fstr << board->ClaimAsString(f);
}

