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
#include "filePBN.h"
#include "portab.h"
#include "Bexcept.h"

using namespace std;


map<string, formatLabelType> PBNmap;


void writePBNSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const Format format);


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
}


bool readPBNChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  bool inAuction = false;
  bool inPlay = false;

  while (getline(fstr, line))
  {
    lno++;
    if (line.empty())
      return true;

    if (line.at(0) == '%' || line.at(0) == '*')
      continue;
    else if (inAuction)
    {
      if (line.at(0) == '[')
        inAuction = false;
      else
      {
        chunk[BRIDGE_FORMAT_AUCTION] += line + "\n";
        continue;
      }
    }
    else if (inPlay)
    {
      if (line.at(0) == '[')
        inPlay = false;
      else
      {
        chunk[BRIDGE_FORMAT_PLAY] += line + "\n";
        continue;
      }
    }

    regex re("^\\[(\\w+)\\s+\"(.*)\"\\]$");
    smatch match;
    if (! regex_search(line, match, re) || match.size() < 2)
      THROW("PBN line does not parse: '" + line + "'");

    auto it = PBNmap.find(match.str(1));
    if (it == PBNmap.end())
      THROW("PBN label is illegal or not implemented: '" + line + "'");

    const unsigned labelNo = static_cast<unsigned>(it->second);
    if (chunk[labelNo] != "")
      THROW("Label already set in line '" + line + "'");

    if (labelNo == BRIDGE_FORMAT_CONTRACT)
    {
      // Kludge to get declarer onto contract.  According to the PBN
      // standard, declarer should always come before contract.
      chunk[labelNo] = match.str(2) + chunk[BRIDGE_FORMAT_DECLARER];
    }
    else if (labelNo == BRIDGE_FORMAT_AUCTION)
    {
      // Multi-line.
      chunk[BRIDGE_FORMAT_AUCTION] += match.str(2) + "\n";
      inAuction = true;
    }
    else if (labelNo == BRIDGE_FORMAT_PLAY)
    {
      // Multi-line.
      chunk[BRIDGE_FORMAT_PLAY] += match.str(2) + "\n";
      inPlay = true;
    }
    else if (match.str(2) != "#")
    {
      chunk[labelNo] = match.str(2);
      if (labelNo <= BRIDGE_FORMAT_VISITTEAM)
      {
        // Kludge to avoid new segment on [Site ""].
        if (labelNo != BRIDGE_FORMAT_LOCATION || chunk[labelNo] != "")
          newSegFlag = true;
      }
    }
  }
  return false;
}


void writePBNSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const Format format)
{
  UNUSED(fstr);
  UNUSED(segment);
  UNUSED(format);
}


void writePBNBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const Format format)
{
  board->calculateScore();

  if (writeInfo.bno == 0 && writeInfo.ino == 0)
  {
    fstr << segment->EventAsString(format);
    fstr << segment->LocationAsString(format);
    fstr << segment->DateAsString(format);
  }
  else
  {
    fstr << "[Event \"#\"]\n";
    if (segment->LocationAsString(format) == "[Site \"\"]\n")
      fstr << "[Site \"\"]\n";
    else
      fstr << "[Site \"#\"]\n";
    fstr << "[Date \"#\"]\n";
  }

  if (writeInfo.ino == 0)
    fstr << segment->NumberAsString(format, writeInfo.bno);
  else
    fstr << "[Board \"#\"]\n";

  fstr << board->strPlayer(BRIDGE_WEST, format);
  fstr << board->strPlayer(BRIDGE_NORTH, format);
  fstr << board->strPlayer(BRIDGE_EAST, format);
  fstr << board->strPlayer(BRIDGE_SOUTH, format);

  fstr << board->strDealer(format);
  fstr << board->strVul(format);
  fstr << board->strDeal(BRIDGE_WEST, format);

  if (writeInfo.bno == 0 && writeInfo.ino == 0)
    fstr << segment->ScoringAsString(format);
  else
    fstr << "[Scoring \"#\"]\n";

  fstr << board->strDeclarer(format);
  fstr << board->strContract(format);
  fstr << board->ResultAsString(format, false);
  fstr << board->strAuction(format);
  fstr << board->PlayAsString(format);

  if (writeInfo.bno == 0 && writeInfo.ino == 0)
  {
    fstr << segment->TitleAsString(format);
    fstr << segment->SessionAsString(format);
    fstr << segment->FirstTeamAsString(format);
    fstr << segment->SecondTeamAsString(format);
  }
  else
  {
    fstr << "[Description \"#\"]\n";
    fstr << "[Stage \"#\"]\n";
    fstr << "[HomeTeam \"#\"]\n";
    fstr << "[VisitTeam \"#\"]\n";
  }

  fstr << board->strRoom(0, format);
  fstr << board->ScoreAsString(format, segment->ScoringIsIMPs());
  fstr << board->strTableau(format);

  fstr << "\n";
}

