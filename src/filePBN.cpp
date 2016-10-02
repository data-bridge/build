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

#include "Group.h"
#include "Segment.h"
#include "Debug.h"
#include "filePBN.h"
#include "parse.h"
#include "portab.h"

using namespace std;

extern Debug debug;


enum PBNlabel
{
  PBN_BOARD = 12,
  PBN_ROOM = 13,
  PBN_DEAL = 14,
  PBN_DEALER = 15,
  PBN_VULNERABLE = 16,
  PBN_AUCTION = 17,
  PBN_DECLARER = 18,
  PBN_CONTRACT = 19,
  PBN_PLAY = 20,
  PBN_RESULT = 21,
  PBN_SCORE = 22,
  PBN_SCORE_IMP = 23,
  PBN_SCORE_MP = 24,
  PBN_OPTIMUM_RESULT_TABLE = 25,
  PBN_LABELS_SIZE = 26
};

const string PBNname[] =
{
  "OptimumResultTable"
};


map<string, formatLabelType> PBNmap;


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrPBN[PBN_LABELS_SIZE];
BoardPtr boardPtrPBN[PBN_LABELS_SIZE];


static bool tryPBNMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);


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

    regex re("^\\[(\\w+)\\s+\"(.+)\"\\]$");
    smatch match;
    if (! regex_search(line, match, re) || match.size() < 2)
    {
      LOG("PBN line does not parse: '" + line + "'");
      return false;
    }

    auto it = PBNmap.find(match.str(1));
    if (it == PBNmap.end())
    {
      LOG("PBN label is illegal or not implemented: '" + line + "'");
      return false;
    }

    const unsigned labelNo = static_cast<unsigned>(it->second);
    if (chunk[labelNo] != "")
    {
      LOG("Label already set in line '" + line + "'");
      return false;
    }

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
        newSegFlag = true;

    }
  }
  return false;
}


bool readPBN(
  Group& group,
  const string& fname)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such PBN file");
    return false;
  }

  const formatType f = BRIDGE_FORMAT_PBN;

  vector<string> chunk(BRIDGE_FORMAT_LABELS_SIZE);

  Segment * segment = nullptr;
  unsigned segno = 0;
  bool newSegFlag = false;

  Board * board = nullptr;
  unsigned bno = 0;

  unsigned lno = 0;

  while (readPBNChunk(fstr, lno, chunk, newSegFlag))
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

    if (chunk[PBN_BOARD] != "")
    {
      // Might be a new board
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

    for (unsigned i = 0; i < PBN_LABELS_SIZE; i++)
    {
      if (! tryPBNMethod(chunk, segment, board, i, fstr, PBNname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


static bool tryPBNMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (label <= PBN_ROOM)
  {
    if (chunk[label] == "")
      return true;
    if ((segment->*segPtrPBN[label]) (chunk[label], BRIDGE_FORMAT_PBN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else if (label == PBN_AUCTION)
  {
    if (chunk[label] == "")
      return true;

    return board->SetAuction(chunk[label], BRIDGE_FORMAT_PBN);
  }
  else if (label == PBN_PLAY)
  {
    if (chunk[label] == "")
      return true;

    return board->SetPlays(chunk[label], BRIDGE_FORMAT_PBN);
  }
  else if (label == PBN_CONTRACT)
  {
    // Easiest way to get declarer into Contract.
    const string s = chunk[label] + chunk[PBN_DECLARER];
    return board->SetContract(s, BRIDGE_FORMAT_PBN);
  }
  else if (chunk[label] == "")
    return true;
  else if ((board->*boardPtrPBN[label]) (chunk[label], BRIDGE_FORMAT_PBN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writePBN(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such PBN file");
    return false;
  }

  fstr << "% PBN\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  const formatType f = BRIDGE_FORMAT_PBN;

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);
    for (unsigned b = 0; b < segment->GetLength(); b++)
    {
      Board * board = segment->GetBoard(b);
      if (board == nullptr)
      {
        LOG("Invalid board");
        fstr.close();
        return false;
      }

      for (unsigned i = 0; i < board->GetLength(); i++)
      {
        if (! board->SetInstance(i))
        {
          LOG("Invalid instance");
          fstr.close();
          return false;
        }

        board->CalculateScore();

        if (b == 0 && i == 0)
        {
          fstr << segment->EventAsString(f);
          fstr << segment->LocationAsString(f);
          fstr << segment->DateAsString(f);
        }
        else
        {
          fstr << "[Event \"#\"]\n";
          fstr << "[Site \"#\"]\n";
          fstr << "[Date \"#\"]\n";
        }

        if (i == 0)
          fstr << segment->NumberAsString(f, b);
        else
          fstr << "[Board \"#\"]\n";

        fstr << board->PlayerAsString(BRIDGE_WEST, f);
        fstr << board->PlayerAsString(BRIDGE_NORTH, f);
        fstr << board->PlayerAsString(BRIDGE_EAST, f);
        fstr << board->PlayerAsString(BRIDGE_SOUTH, f);

        fstr << board->DealerAsString(f);
        fstr << board->VulAsString(f);
        fstr << board->DealAsString(BRIDGE_WEST, f);

        if (b == 0 && i == 0)
          fstr << segment->ScoringAsString(f);
        else
          fstr << "[Scoring \"#\"]\n";

        fstr << board->DeclarerAsString(f);
        fstr << board->ContractAsString(f);
        fstr << board->ResultAsString(f, false);
        fstr << board->AuctionAsString(f);
        fstr << board->PlayAsString(f);

        if (b == 0 && i == 0)
        {
          fstr << segment->TitleAsString(f);
          fstr << segment->SessionAsString(f);
          fstr << segment->FirstTeamAsString(f);
          fstr << segment->SecondTeamAsString(f);
        }
        else
        {
          fstr << "[Description \"#\"]\n";
          fstr << "[Stage \"#\"]\n";
          fstr << "[HomeTeam \"#\"]\n";
          fstr << "[VisitTeam \"#\"]\n";
        }

        fstr << board->RoomAsString(0, f);
        fstr << board->ScoreAsString(f, segment->ScoringIsIMPs());
        fstr << board->TableauAsString(f);

        fstr << "\n";
      }
    }
  }

  fstr.close();
  return true;
}

