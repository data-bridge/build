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
  PBN_DESCRIPTION = 0,
  PBN_DATE = 1,
  PBN_SITE = 2,
  PBN_EVENT = 3,
  PBN_STAGE = 4,
  PBN_SCORING = 5,
  PBN_HOMETEAM = 6,
  PBN_VISITTEAM = 7,
  PBN_WEST = 8,
  PBN_NORTH = 9,
  PBN_EAST = 10,
  PBN_SOUTH = 11,
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
  "Description",
  "Date",
  "Site",
  "Event",
  "Stage",
  "Scoring",
  "HomeTeam",
  "VisitTeam",
  "West",
  "North",
  "East",
  "South",
  "Board",
  "Room",
  "Deal",
  "Dealer",
  "Vulnerable",
  "Auction",
  "Declarer",
  "Contract",
  "Play",
  "Result",
  "Score",
  "ScoreIMP",
  "ScoreMP",
  "OptimumResultTable"
};


map<string, PBNlabel> LABEL_TO_LABEL_NO;


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrPBN[PBN_LABELS_SIZE];
BoardPtr boardPtrPBN[PBN_LABELS_SIZE];

struct chunkType
{
  string single[PBN_LABELS_SIZE];
  vector<string> auctionList;
  vector<string> playList;
};


static bool readPBNChunk(
  ifstream& fstr,
  unsigned& lno,
  chunkType& chunk,
  bool& newSegFlag);

static bool tryPBNMethod(
  const chunkType& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);


void setPBNtables()
{
  for (unsigned i = 0; i < PBN_LABELS_SIZE; i++)
    LABEL_TO_LABEL_NO[PBNname[i]] = static_cast<PBNlabel>(i);

  segPtrPBN[PBN_DESCRIPTION] = &Segment::SetTitle;
  segPtrPBN[PBN_DATE] = &Segment::SetDate;
  segPtrPBN[PBN_SITE] = &Segment::SetLocation;
  segPtrPBN[PBN_EVENT] = &Segment::SetEvent;
  segPtrPBN[PBN_STAGE] = &Segment::SetSession;
  segPtrPBN[PBN_SCORING] = &Segment::SetScoring;
  segPtrPBN[PBN_HOMETEAM] = &Segment::SetFirstTeam;
  segPtrPBN[PBN_VISITTEAM] = &Segment::SetSecondTeam;

  segPtrPBN[PBN_WEST] = &Segment::SetWest;
  segPtrPBN[PBN_NORTH] = &Segment::SetNorth;
  segPtrPBN[PBN_EAST] = &Segment::SetEast;
  segPtrPBN[PBN_SOUTH] = &Segment::SetSouth;
  segPtrPBN[PBN_BOARD] = &Segment::SetNumber;
  segPtrPBN[PBN_ROOM] = &Segment::SetRoom;

  boardPtrPBN[PBN_DEAL] = &Board::SetDeal;
  boardPtrPBN[PBN_DEALER] = &Board::SetDealer;
  boardPtrPBN[PBN_VULNERABLE] = &Board::SetVul;
  boardPtrPBN[PBN_DECLARER] = &Board::SetDeclarer;
  boardPtrPBN[PBN_CONTRACT] = &Board::SetContract;
  boardPtrPBN[PBN_RESULT] = &Board::SetResult;
  boardPtrPBN[PBN_SCORE] = &Board::SetScore;
  boardPtrPBN[PBN_SCORE_IMP] = &Board::SetScoreIMP;
  boardPtrPBN[PBN_SCORE_MP] = &Board::SetScoreMP;
  boardPtrPBN[PBN_OPTIMUM_RESULT_TABLE] = &Board::SetTableau;
}


static bool readPBNChunk(
  ifstream& fstr,
  unsigned& lno,
  chunkType& chunk,
  bool& newSegFlag)
{
  string line;
  newSegFlag = false;
  for (unsigned i = 0; i < PBN_LABELS_SIZE; i++)
    chunk.single[i] = "";

  chunk.auctionList.resize(0);
  chunk.playList.resize(0);
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
        chunk.auctionList.push_back(line);
        continue;
      }
    }
    else if (inPlay)
    {
      if (line.at(0) == '[')
        inPlay = false;
      else
      {
        chunk.playList.push_back(line);
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

    auto it = LABEL_TO_LABEL_NO.find(match.str(1));
    if (it == LABEL_TO_LABEL_NO.end())
    {
      LOG("PBN label is illegal or not implemented: '" + line + "'");
      return false;
    }

    const int labelNo = static_cast<int>(it->second);
    if (chunk.single[labelNo] != "")
    {
      LOG("Label already set in line '" + line + "'");
      return false;
    }

    if (labelNo == PBN_AUCTION)
    {
      // Multi-line.
      chunk.auctionList.push_back(match.str(2));
      inAuction = true;
    }
    else if (labelNo == PBN_PLAY)
    {
      // Multi-line.
      chunk.playList.push_back(match.str(2));
      inPlay = true;
    }
    else if (match.str(2) != "#")
    {
      chunk.single[labelNo] = match.str(2);
      if (labelNo <= PBN_VISITTEAM)
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

  chunkType chunk;

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

    if (chunk.single[PBN_BOARD] != "")
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
  const chunkType& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (label <= PBN_ROOM)
  {
    if (chunk.single[label] == "")
      return true;
    if ((segment->*segPtrPBN[label])
        (chunk.single[label], BRIDGE_FORMAT_PBN))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk.single[label] + "'");
      fstr.close();
      return false;
    }
  }
  else if (label == PBN_AUCTION)
  {
    if (chunk.auctionList.size() == 0)
      return true;

    return board->SetAuction(chunk.auctionList, BRIDGE_FORMAT_PBN);
  }
  else if (label == PBN_PLAY)
  {
    if (chunk.playList.size() == 0)
      return true;

    return board->SetPlays(chunk.playList, BRIDGE_FORMAT_PBN);
  }
  else if (label == PBN_CONTRACT)
  {
    // Easiest way to get declarer into Contract.
    const string s = chunk.single[label] + chunk.single[PBN_DECLARER];
    return board->SetContract(s, BRIDGE_FORMAT_PBN);
  }
  else if (chunk.single[label] == "")
    return true;
  else if ((board->*boardPtrPBN[label])
      (chunk.single[label], BRIDGE_FORMAT_PBN))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk.single[label] + "'");
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

