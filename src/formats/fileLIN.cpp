/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <sstream>
#include <map>

#include "fileLIN.h"
#include "WriteInfo.h"

#include "../records/Segment.h"
#include "../records/Board.h"

#include "../files/Buffer.h"
#include "../files/Chunk.h"

#include "../parse.h"

#include "../handling/Bexcept.h"

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
  LINmap["pn"] = BRIDGE_FORMAT_PLAYERS; // May also occur in header
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
  Chunk& chunk,
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
    if (i == 0x00 || (qxSeen && (i == 0x71 || i == 0x76))) // q(x), v(g)
      doneFlag = true;

    if (lineData.type == BRIDGE_BUFFER_EMPTY)
      continue;
    
    unsigned pos = 0;
    string label, value;
    while (nextLINPair(lineData.line, pos, label, value))
    {
      if (label == "pn")
      {
        if (! qxSeen)
        {
          // Artificial label to disambiguate.
          if (chunk.isSet(BRIDGE_FORMAT_PLAYERS_LIST))
            continue;
          else
            label = "px";
        }
        else if (chunk.isSet(BRIDGE_FORMAT_AUCTION) ||
            chunk.isSet(BRIDGE_FORMAT_RESULT))
        {
          // Kludge to skip some late pn's.
          continue;
        }
      }

      if (label == "an")
      {
        alerts << aNo << " " << value << "\n";
        chunk.append(BRIDGE_FORMAT_AUCTION, "^" + to_string(aNo));
        aNo++;
        continue;
      }

      auto it = LINmap.find(label);
      if (it == LINmap.end())
      {
        toLower(label);
        it = LINmap.find(label);
        if (it == LINmap.end())
          THROW("Illegal LIN label in line '" + lineData.line + "'");
      }

      const Label labelNo = it->second;
      if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
        newSegFlag = true;

      // We ignore some labels.
      if (labelNo == BRIDGE_FORMAT_LABELS_SIZE)
        continue;

      if (labelNo == BRIDGE_FORMAT_PLAY)
      {
        // This is not rigorously correct
        if (cardCount > 0 && cardCount % 4 == 0)
          chunk.append(labelNo, ":");

        cardCount += (value.size() > 2 ? 4u : 1u);

        chunk.append(labelNo, value);
      }
      else if (labelNo == BRIDGE_FORMAT_AUCTION)
      {
        if (value.length() > 4)
        {
          trimLeading(value, '-');
          value = trimTrailing(value, '-');
        }
        chunk.append(labelNo, value);
      }
      else if (chunk.isEmpty(labelNo))
        chunk.set(labelNo, value, lineData.no);
      else if (labelNo == BRIDGE_FORMAT_PLAYERS_HEADER)
      {
        value = trimTrailing(value, ',');
        const string ch = chunk.get(labelNo);
        if (value.length() < ch.length() &&
          ch.substr(0, value.length()) == value)
        {
          // Ignore if a substring.
        }
        else if (ch.length() < value.length() &&
          value.substr(0, ch.length()) == ch)
        {
          // Swap if the other way round (very rare).
          chunk.set(labelNo, value, lineData.no);
        }
        else
          THROW("Label already set in line '" + lineData.line + "'");
      }
      else
        THROW("Label already set in line '" + lineData.line + "'");
    }
  }

  if (alerts.str() != "")
    chunk.append(BRIDGE_FORMAT_AUCTION, "\n" + alerts.str());
  if (! qxSeen && buffer.peek() != 0x00)
    THROW("No deal found");
  if (chunk.isSet(BRIDGE_FORMAT_BOARD_NO) &&
      chunk.isEmpty(BRIDGE_FORMAT_DEAL))
    THROW("No LIN cards found (md)");
}


void writeLINSegmentLevel(
  string& st,
  const Segment& segment,
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
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  const Instance& instance = board.getInstance(writeInfo.instNo);

  if (format != BRIDGE_FORMAT_LIN_RP && board.skipped(writeInfo.instNo))
    return;

  st += instance.strRoom(writeInfo.bno, BRIDGE_FORMAT_LIN_RP);

  if (format == BRIDGE_FORMAT_LIN)
    st += "pn|" + board.strPlayersBoard(format, segment.scoringIsIMPs()) + "|";
  else if (format == BRIDGE_FORMAT_LIN_TRN)
    st += instance.strPlayers(format);

  st += board.strDeal(format);

  if (format == BRIDGE_FORMAT_LIN || format == BRIDGE_FORMAT_LIN_TRN)
    st += segment.strNumber(writeInfo.bno, format);

  st += instance.strVul(format);

  if (! board.skipped(writeInfo.instNo))
  {
    st += instance.strAuction(format);
    st += instance.strPlay(format);
  }

  st += instance.strClaim(format);
}

