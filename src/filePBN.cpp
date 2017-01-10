/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <unordered_map>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "filePBN.h"
#include "Bexcept.h"

using namespace std;


unordered_map<string, Label> PBNmap;


void setPBNTables()
{
  PBNmap["Description"] = BRIDGE_FORMAT_TITLE;
  PBNmap["Date"] = BRIDGE_FORMAT_DATE;
  PBNmap["Site"] = BRIDGE_FORMAT_LOCATION;
  PBNmap["Event"] = BRIDGE_FORMAT_EVENT;
  PBNmap["Stage"] = BRIDGE_FORMAT_SESSION;
  PBNmap["Scoring"] = BRIDGE_FORMAT_SCORING;

  PBNmap["HomeTeam"] = BRIDGE_FORMAT_HOMETEAM;
  PBNmap["VisitTeam"] = BRIDGE_FORMAT_VISITTEAM;
  PBNmap["West"] = BRIDGE_FORMAT_WEST;
  PBNmap["North"] = BRIDGE_FORMAT_NORTH;
  PBNmap["East"] = BRIDGE_FORMAT_EAST;
  PBNmap["South"] = BRIDGE_FORMAT_SOUTH;

  PBNmap["Board"] = BRIDGE_FORMAT_BOARD_NO;
  PBNmap["Room"] = BRIDGE_FORMAT_ROOM;
  PBNmap["Deal"] = BRIDGE_FORMAT_DEAL;
  PBNmap["Dealer"] = BRIDGE_FORMAT_DEALER;
  PBNmap["Vulnerable"] = BRIDGE_FORMAT_VULNERABLE;
  PBNmap["Auction"] = BRIDGE_FORMAT_AUCTION;
  PBNmap["Declarer"] = BRIDGE_FORMAT_DECLARER;
  PBNmap["Contract"] = BRIDGE_FORMAT_CONTRACT;
  PBNmap["Play"] = BRIDGE_FORMAT_PLAY;
  PBNmap["Result"] = BRIDGE_FORMAT_RESULT;
  PBNmap["Score"] = BRIDGE_FORMAT_SCORE;
  PBNmap["ScoreIMP"] = BRIDGE_FORMAT_SCORE_IMP;
  PBNmap["ScoreMP"] = BRIDGE_FORMAT_SCORE_MP;
  PBNmap["OptimumResultTable"] = BRIDGE_FORMAT_DOUBLE_DUMMY;

  // These are discarded.
  PBNmap["Round"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["Table"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["ScoreMP"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["Seq"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["Link"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["Time"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["Filters"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["ScorePercentage"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["OptimumContract"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["OptimumScore"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["OptimumDeclarer"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["OptimumResult"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["ValidDeal"] = BRIDGE_FORMAT_LABELS_SIZE;
  PBNmap["DDS_Filters"] = BRIDGE_FORMAT_LABELS_SIZE;
}


static void setModifiedContract(
  const string& st,
  vector<string>& chunk)
{
  const unsigned l = st.length();
  // Can be P, or 1NT.
  if (l == 1 || (l == 4 && st == "Pass"))
    chunk[BRIDGE_FORMAT_CONTRACT] = st;
  else if (l <= 2 || 
    (l == 3 && st.at(2) == 'T'))
    chunk[BRIDGE_FORMAT_CONTRACT] = st + chunk[BRIDGE_FORMAT_DECLARER];
  else
  {
    // Could be of form 4HX or even 4H X.
    const unsigned start = (st.at(2) == 'T' ? 3u : 2u);
    unsigned p = start;
    while (p < l && st.at(p) == ' ')
      p++;

    if (p == l)
      chunk[BRIDGE_FORMAT_CONTRACT] = 
        st.substr(0, start) + chunk[BRIDGE_FORMAT_DECLARER];
    else
      chunk[BRIDGE_FORMAT_CONTRACT] = 
        st.substr(0, start) + chunk[BRIDGE_FORMAT_DECLARER] + st.substr(p);
  }
}


void readPBNChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  LineData lineData;
  bool inAuction = false;
  bool inPlay = false;
  bool inDD = false;
  string alerts = "";

  while (buffer.next(lineData))
  {
    if (lineData.type == BRIDGE_BUFFER_EMPTY)
      break;

    if (lineData.type == BRIDGE_BUFFER_COMMENT ||
        lineData.line.at(0) == '*')
      continue;
    else if (inAuction)
    {
      if (lineData.type == BRIDGE_BUFFER_STRUCTURED)
      {
        if (lineData.label == "Note")
        {
          alerts += lineData.line + "\n";
          continue;
        }
        else
          inAuction = false;
      }
      else
      {
        chunk[BRIDGE_FORMAT_AUCTION] += lineData.line + "\n";
        lno[BRIDGE_FORMAT_AUCTION] = lineData.no; // Could be range...
        continue;
      }
    }
    else if (inPlay)
    {
      if (lineData.type == BRIDGE_BUFFER_STRUCTURED)
      {
        // Bug in some Double Dummy Captain files:
        // Note is repeated in Play section.
        if (lineData.label == "Note")
          continue;
        else
          inPlay = false;
      }
      else
      {
        chunk[BRIDGE_FORMAT_PLAY] += lineData.line + "\n";
        lno[BRIDGE_FORMAT_PLAY] = lineData.no; // Could be range...
        continue;
      }
    }
    else if (inDD)
    {
      if (lineData.type == BRIDGE_BUFFER_STRUCTURED)
        inDD = false;
      else
      {
        chunk[BRIDGE_FORMAT_DOUBLE_DUMMY] += lineData.line + "\n";
        lno[BRIDGE_FORMAT_DOUBLE_DUMMY] = lineData.no; // Could be range...
        continue;
      }
    }

    if (lineData.type != BRIDGE_BUFFER_STRUCTURED)
      THROW("PBN line does not parse: '" + lineData.line + "'");

    auto it = PBNmap.find(lineData.label);
    if (it == PBNmap.end())
      THROW("Unknown PBN label: '" + lineData.label + "'");

    const unsigned labelNo = static_cast<unsigned>(it->second);

    // Skip certain labels.
    if (labelNo == BRIDGE_FORMAT_LABELS_SIZE)
      continue;

    if (chunk[labelNo] != "")
      THROW("Label already set in line '" + lineData.line + "'");

    if (labelNo == BRIDGE_FORMAT_CONTRACT)
    {
      // Kludge to get declarer onto contract.  According to the PBN
      // standard, declarer should always come before contract.
      if (lineData.value.length() <= 2)
        chunk[labelNo] = lineData.value + chunk[BRIDGE_FORMAT_DECLARER];
      else
      {
        // Could be of form 4HX or even 4H X.
      }
      setModifiedContract(lineData.value, chunk);
      lno[labelNo] = lineData.no;
    }
    else if (labelNo == BRIDGE_FORMAT_AUCTION)
    {
      // Multi-line.
      chunk[BRIDGE_FORMAT_AUCTION] += lineData.value + "\n";
      lno[labelNo] = lineData.no; // Could be range...
      inAuction = true;
    }
    else if (labelNo == BRIDGE_FORMAT_PLAY)
    {
      // Multi-line.
      chunk[BRIDGE_FORMAT_PLAY] += lineData.value + "\n";
      lno[labelNo] = lineData.no; // Could be range...
      inPlay = true;
    }
    else if (labelNo == BRIDGE_FORMAT_DOUBLE_DUMMY)
    {
      // Multi-line.
      chunk[BRIDGE_FORMAT_DOUBLE_DUMMY] += lineData.value + "\n";
      lno[labelNo] = lineData.no; // Could be range...
      inDD = true;
    }
    else if (lineData.value != "#")
    {
      if (lineData.value.length() > 2 &&
          lineData.value.at(0) == '#' &&
          lineData.value.at(1) == '#')
        chunk[labelNo] = lineData.value.substr(2);
      else
        chunk[labelNo] = lineData.value;

      lno[labelNo] = lineData.no; // Could be range...
      if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
      {
        // Kludge to avoid new segment on [Site ""].
        if (labelNo != BRIDGE_FORMAT_LOCATION || chunk[labelNo] != "")
          newSegFlag = true;
      }
    }
  }

  chunk[BRIDGE_FORMAT_AUCTION] += alerts;
}


void writePBNSegmentLevel(
  string& st,
  Segment& segment,
  const Format format)
{
  UNUSED(st);
  UNUSED(segment);
  UNUSED(format);
}


void writePBNBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  board.calculateScore();

  if (writeInfo.bno == 0 && writeInfo.ino == 0)
  {
    st += segment.strEvent(format);
    st += segment.strLocation(format);
    st += segment.strDate(format);
  }
  else
  {
    st += "[Event \"#\"]\n";
    if (segment.strLocation(format) == "[Site \"\"]\n")
      st += "[Site \"\"]\n";
    else
      st += "[Site \"#\"]\n";
    st += "[Date \"#\"]\n";
  }

  if (writeInfo.ino == 0)
    st += segment.strNumber(writeInfo.bno, format);
  else
    st += "[Board \"#\"]\n";

  st += board.strPlayer(BRIDGE_WEST, format);
  st += board.strPlayer(BRIDGE_NORTH, format);
  st += board.strPlayer(BRIDGE_EAST, format);
  st += board.strPlayer(BRIDGE_SOUTH, format);

  st += board.strDealer(format);
  st += board.strVul(format);
  st += board.strDeal(BRIDGE_WEST, format);

  if (writeInfo.bno == 0 && writeInfo.ino == 0)
    st += segment.strScoring(format);
  else
    st += "[Scoring \"#\"]\n";

  st += board.strDeclarer(format);
  st += board.strContract(format);
  st += board.strResult(format, false);
  st += board.strAuction(format);
  st += board.strPlay(format);

  if (writeInfo.bno == 0 && writeInfo.ino == 0)
  {
    st += segment.strTitle(format);
    st += segment.strSession(format);
    st += segment.strFirstTeam(format);
    st += segment.strSecondTeam(format);
  }
  else
  {
    st += "[Description \"#\"]\n";
    st += "[Stage \"#\"]\n";
    st += "[HomeTeam \"#\"]\n";
    st += "[VisitTeam \"#\"]\n";
  }

  st += board.strRoom(0, format);
  st += board.strScore(format, segment.scoringIsIMPs());
  if (writeInfo.ino == 0)
    st += board.strGivenScore(format);
  st += board.strTableau(format);

  st += "\n";
}

