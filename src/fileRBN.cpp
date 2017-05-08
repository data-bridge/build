/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "Group.h"
#include "Segment.h"
#include "Bexcept.h"
#include "fileRBN.h"
#include "parse.h"

using namespace std;


Label RBNmap[128];


void setRBNTables()
{
  for (unsigned char c = 0; c < 128; c++)
    RBNmap[c] = BRIDGE_FORMAT_LABELS_SIZE;

  // Segment-level
  RBNmap['T'] = BRIDGE_FORMAT_TITLE;
  RBNmap['D'] = BRIDGE_FORMAT_DATE;
  RBNmap['L'] = BRIDGE_FORMAT_LOCATION;
  RBNmap['E'] = BRIDGE_FORMAT_EVENT;
  RBNmap['S'] = BRIDGE_FORMAT_SESSION;
  RBNmap['F'] = BRIDGE_FORMAT_SCORING;
  RBNmap['K'] = BRIDGE_FORMAT_TEAMS;

  // Board-level but relevant in Segment
  RBNmap['N'] = BRIDGE_FORMAT_PLAYERS_BOARD;
  RBNmap['B'] = BRIDGE_FORMAT_BOARD_NO;

  // Purely Board-level
  RBNmap['H'] = BRIDGE_FORMAT_DEAL;
  RBNmap['A'] = BRIDGE_FORMAT_AUCTION;
  RBNmap['C'] = BRIDGE_FORMAT_CONTRACT;
  RBNmap['P'] = BRIDGE_FORMAT_PLAY;
  RBNmap['R'] = BRIDGE_FORMAT_RESULT;
  RBNmap['M'] = BRIDGE_FORMAT_DOUBLE_DUMMY;
}


void readRBNChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  Chunk& chunk,
  bool& newSegFlag)
{
  LineData lineData;
  string line;

  while (buffer.next(lineData))
  {
    if (lineData.type == BRIDGE_BUFFER_EMPTY)
      return;
    
    if (lineData.type == BRIDGE_BUFFER_COMMENT)
      continue;
    else if (lineData.type != BRIDGE_BUFFER_STRUCTURED)
      THROW("Not implemented yet");

    const char c = lineData.label.at(0);
    const Label labelNo = RBNmap[static_cast<int>(c)];
    if (labelNo == BRIDGE_FORMAT_LABELS_SIZE)
      THROW("Illegal RBN label in line:\n" + lineData.line);

    if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
      newSegFlag = true;

    if (chunk.isSet(labelNo))
      THROW("RBN label already set in line:\n" + lineData.line);

    chunk.set(labelNo, lineData.value);
    lno[labelNo] = lineData.no;
  }
}


void writeRBNSegmentLevel(
  string& st,
  Segment& segment,
  const Format format)
{
  st += segment.strTitle(format);
  st += segment.strDate(format);
  st += segment.strLocation(format);
  st += segment.strEvent(format);
  st += segment.strSession(format);
  st += segment.strScoring(format);
  st += segment.strTeams(format);
}


void writeRBNBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  string names = board.strPlayers(format);
  if (names != writeInfo.namesOld[writeInfo.ino])
  {
    st += names;
    writeInfo.namesOld[writeInfo.ino] = names;
  }
        
  if (writeInfo.ino == 0)
  {
    st += segment.strNumber(writeInfo.bno, format);
    st += board.strDeal(BRIDGE_WEST, format);
  }

  board.calculateScore();

  st += board.strAuction(format);
  st += board.strContract(format);
  st += board.strPlay(format);
  st += board.strResult(format, segment.scoringIsIMPs(), segment.getCOCO());
  st += "\n";
}

