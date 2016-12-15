/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <sstream>
#include <map>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "fileLIN.h"
#include "Bexcept.h"

using namespace std;


map<string, Label> LINmap;


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


static bool nextLINPair(
  const string& line,
  unsigned& pos,
  string& label,
  string& value)
{
  if (pos == line.size())
    return false;

  size_t lpos = line.find('|', pos);
  if (lpos == string::npos)
    return false;
  size_t vpos = line.find('|', lpos+1);
  if (vpos == string::npos)
    return false;

  label = line.substr(pos, lpos-pos);
  value = line.substr(lpos+1, vpos-lpos-1);
  pos = static_cast<unsigned>(vpos)+1;
  return true;
}


void readLINChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  LineData lineData;
  bool qxSeen = false;
  bool doneFlag = false;
  unsigned cardCount = 0;
  stringstream alerts;
  unsigned aNo = 1;
  while (! doneFlag && buffer.next(lineData))
  {
    if (lineData.type != BRIDGE_BUFFER_EMPTY)
    {
      if (lineData.type == BRIDGE_BUFFER_COMMENT)
        continue;
      else if (lineData.line.at(0) == 'q')
        qxSeen = true;
    }

    int i = buffer.peek();
    if (i == 0x00 || (qxSeen && i == 0x71)) // q
      doneFlag = true;

    if (lineData.type == BRIDGE_BUFFER_EMPTY)
      continue;
    
    unsigned pos = 0;
    string label, value;
    while (nextLINPair(lineData.line, pos, label, value))
    {
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
        THROW("Illegal LIN label in line '" + lineData.line + "'");

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
        THROW("Label already set in line '" + lineData.line + "'");

      lno[labelNo] = lineData.no;
    }
  }

  if (alerts.str() != "")
    chunk[BRIDGE_FORMAT_AUCTION] += "\n" + alerts.str();
  if (! qxSeen && buffer.peek() != 0x00)
    THROW("No deal found");
  if (chunk[BRIDGE_FORMAT_BOARD_NO] != "" &&
      chunk[BRIDGE_FORMAT_DEAL] == "")
    THROW("No LIN cards found (md)");
}


void writeLINSegmentLevel(
  string& st,
  Segment& segment,
  const Format format)
{
  st += segment.strTitle(format);
  st += segment.strContracts(format);
  st += segment.strPlayers(format);
  st += segment.strScores(format);
  st += segment.strBoards(format);
}


void writeLINBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  st += segment.strNumber(writeInfo.bno, format);

  if (format == BRIDGE_FORMAT_LIN || format == BRIDGE_FORMAT_LIN_TRN)
    st += board.strPlayers(format);

  st += board.strDeal(format);

  if (format == BRIDGE_FORMAT_LIN || format == BRIDGE_FORMAT_LIN_TRN)
    st += segment.strNumber(writeInfo.bno, format);

  st += board.strVul(format);

  board.calculateScore();

  st += board.strAuction(format);
  st += board.strPlay(format);
  st += board.strClaim(format);
}

