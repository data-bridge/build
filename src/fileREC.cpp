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

#include "Canvas.h"
#include "fileREC.h"
#include "parse.h"
#include "portab.h"
#include "debug.h"

using namespace std;

extern Debug debug;


const unsigned REClineLength = 80;

enum REClabel
{
  REC_SCORING = 0,
  REC_WEST = 1,
  REC_NORTH = 2,
  REC_EAST = 3,
  REC_SOUTH = 4,
  REC_BOARD = 5,
  REC_DEAL = 6,
  REC_DEALER = 7,
  REC_VULNERABLE = 8,
  REC_AUCTION = 9,
  REC_PLAY = 10,
  REC_RESULT = 11,
  REC_SCORE = 12,
  REC_SCORE_IMP = 13,
  REC_LABELS_SIZE = 14
};

const string RECname[] =
{
  "Scoring",
  "West",
  "North",
  "East",
  "South",
  "Board",
  "Deal",
  "Dealer",
  "Vulnerable",
  "Auction",
  "Play",
  "Result",
  "Score",
  "ScoreIMP"
};


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrREC[REC_LABELS_SIZE];
BoardPtr boardPtrREC[REC_LABELS_SIZE];


bool tryRECMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

bool readRECCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

bool getRECCanvasOffset(
  const vector<string>& canvas,
  unsigned& playLine);

bool getRECFields(
  const vector<string>& canvas,
  const unsigned auctionLine,
  vector<string>& chunk);

bool getRECDeal(
  const vector<string>& canvas,
  vector<string>& chunk);

bool getRECAuction(
  const vector<string>& canvas,
  vector<string>& chunk);

bool getRECPlay(
  const vector<string>& canvas,
  const unsigned& offset,
  vector<string>& chunk);

bool readRECChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk);


void setRECtables()
{
  segPtrREC[REC_SCORING] = &Segment::SetScoring;

  segPtrREC[REC_WEST] = &Segment::SetWest;
  segPtrREC[REC_NORTH] = &Segment::SetNorth;
  segPtrREC[REC_EAST] = &Segment::SetEast;
  segPtrREC[REC_SOUTH] = &Segment::SetSouth;

  segPtrREC[REC_BOARD] = &Segment::SetNumber;

  boardPtrREC[REC_DEAL] = &Board::SetDeal;
  boardPtrREC[REC_DEALER] = &Board::SetDealer;
  boardPtrREC[REC_VULNERABLE] = &Board::SetVul;
  boardPtrREC[REC_AUCTION] = &Board::SetAuction;
  boardPtrREC[REC_PLAY] = &Board::SetPlays;

  boardPtrREC[REC_RESULT] = &Board::SetResult;
  boardPtrREC[REC_SCORE] = &Board::SetScore;
  boardPtrREC[REC_SCORE_IMP] = &Board::SetScoreIMP;
}


bool readRECCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas)
{
  string line, prevLine = "";
  while (getline(fstr, line))
  {
    lno++;
    if (line.empty())
    {
      if (prevLine.size() >= 8 && prevLine.at(1) != ' ' && 
          prevLine.at(2) == ' ' && prevLine.at(3) == ' ')
        return true;
      else
        continue;
    }
    else if (line.at(0) == '%')
      continue;

    canvas.push_back(line);
    prevLine = line;
  }
  return true;
}


bool getRECCanvasOffset(
  const vector<string>& canvas,
  unsigned& playLine)
{
  string wd;
  for (playLine = 12; playLine < canvas.size(); playLine++)
  {
    const string& line = canvas[playLine];
    if (line.size() < 8 || line.at(3) != ' ')
      continue;


    const unsigned st = (line.at(0) == ' ' ? 0u : 1u);
    if (! ReadNextWord(line, st, wd))
      continue;

    unsigned u;
    if (StringToUnsigned(wd, u))
      break;
  }

  return (playLine != canvas.size());
}


bool getRECFields(
  const vector<string>& canvas,
  const unsigned pline,
  vector<string>& chunk)
{
  if (canvas[0].size() < 30 || 
      canvas[1].size() < 30 ||
      canvas[3].size() < 30 ||
      canvas[6].size() < 18 ||
      canvas[pline-2].size() < 30)
  {
    LOG("Some deal lines are too short");
    return false;
  }

  if (! ReadNextWord(canvas[0], 0, chunk[REC_SCORING])) return false;
  if (! ReadNextWord(canvas[0], 30, chunk[REC_DEALER])) return false;
  if (! ReadNextWord(canvas[1], 6, chunk[REC_BOARD])) return false;
  if (! ReadNextWord(canvas[1], 30, chunk[REC_VULNERABLE])) return false;

  if (! ReadNextWord(canvas[3], 0, chunk[REC_WEST])) return false;
  if (! ReadNextWord(canvas[0], 18, chunk[REC_NORTH])) return false;
  if (! ReadNextWord(canvas[3], 30, chunk[REC_EAST])) return false;
  if (! ReadNextWord(canvas[6], 18, chunk[REC_SOUTH])) return false;

  if (! getRECDeal(canvas, chunk)) return false;
  if (! getRECAuction(canvas, chunk)) return false;
  if (! getRECPlay(canvas, pline, chunk)) return false;

  chunk[REC_RESULT] = canvas[pline-2].substr(28);

  return true;
}


bool getRECDeal(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! ReadNextSpacedWord(canvas[4], 2, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[5], 2, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[6], 2, std)) std = "";
  if (! ReadNextSpacedWord(canvas[7], 2, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[1], 14, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[2], 14, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[3], 14, std)) std = "";
  if (! ReadNextSpacedWord(canvas[4], 14, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[4], 26, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[5], 26, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[6], 26, std)) std = "";
  if (! ReadNextSpacedWord(canvas[7], 26, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[7], 14, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[8], 14, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[9], 14, std)) std = "";
  if (! ReadNextSpacedWord(canvas[10], 14, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;

  // Void is shown as empty in REC.
  chunk[REC_DEAL] = d.str();
  return true;
}


bool getRECAuction(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  unsigned firstStart = 0;
  while (firstStart < 32 && canvas[15].at(firstStart) == ' ')
    firstStart += 9;

  if (firstStart >= 32)
    return false;

  string wd;
  unsigned no = 0;
  bool done = false;
  unsigned l;
  unsigned numPasses = 0;
  for (l = 15; l < canvas.size(); l++)
  {
    for (unsigned beg = (l == 15 ? firstStart : 0); beg < 32; beg += 9)
    {
      if (! ReadNextWord(canvas[l], beg, wd))
      {
        done = true;
        break;
      }

      if (no > 0 && no % 4 == 0)
        d << ":";

      if (wd == "Dbl")
      {
        d << "X";
        numPasses = 0;
      }
      else if (wd == "Rdbl")
      {
        d << "R";
        numPasses = 0;
      }
      else if (wd.size() == 3 && wd.at(1) == 'N' && wd.at(2) == 'T')
      {
        wd.erase(2, 1);
        d << wd;
        numPasses = 0;
      }
      else if (wd == "Pass")
      {
        d << "P";
        numPasses++;
      }
      else if (wd == "All")
      {
        d << "A";
        done = true;
        break;
      }
      else
      {
        d << wd;
        numPasses = 0;
      }
      no++;

      if (numPasses >= 3)
      {
        done = true;
        break;
      }
    }
    if (done)
      break;
  }

  if (l == canvas.size())
    return false;

  chunk[REC_AUCTION] = d.str();
  return true;
}


bool getRECPlay(
  const vector<string>& canvas,
  const unsigned& offset,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  for (unsigned l = offset; l < canvas.size(); l++)
  {
    const string& line = canvas[l];
    int seen = count(line.begin(), line.end(), ',');
    if (seen > 3)
    {
      LOG("Too many commas in play line");
      return false;
    }

    vector<string> words(4);
    words.clear();
    tokenize(line, words, ",");

    if (words[0].size() != 13)
    {
      LOG("Odd length of first play in line");
      return false;
    }

    if (l != offset)
      d << ":";

    d << words[0].substr(11);
    for (unsigned i = 1; i < words.size(); i++)
      d << words[i];
  }

  chunk[REC_PLAY] = d.str();
  return true;
}


bool readRECChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readRECCanvas(fstr, lno, canvas))
    return false;
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < REC_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned playLine = 0;
  if (! getRECCanvasOffset(canvas, playLine))
    return false;

  return getRECFields(canvas, playLine, chunk);
}


bool readREC(
  Group& group,
  const string& fname)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such REC file");
    return false;
  }

  group.SetFileName(fname);

  const formatType f = BRIDGE_FORMAT_REC;

  Segment * segment = nullptr;
  Board * board = nullptr;
  unsigned bno = 0;
  unsigned lno = 0;

  // Always one segment.
  if (! group.MakeSegment(0))
  {
    LOG("Cannot make segment " + STR(0));
    fstr.close();
    return false;
  }
  segment = group.GetSegment(0);

  string lastBoard = "";

  vector<string> chunk(REC_LABELS_SIZE);
  while (readRECChunk(fstr, lno, chunk))
  {
    if (chunk[REC_BOARD] != "" && chunk[REC_BOARD] != lastBoard)
    {
      // New board.
      lastBoard = chunk[REC_BOARD];
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

    for (unsigned i = 0; i < REC_LABELS_SIZE; i++)
    {
      if (! tryRECMethod(chunk, segment, board, i, fstr, RECname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


bool tryRECMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= REC_BOARD)
  {
    if ((segment->*segPtrREC[label])(chunk[label], BRIDGE_FORMAT_REC))
      return true;
    else
    {
      LOG("Cannot add " + info + " line '" + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else if ((board->*boardPtrREC[label])(chunk[label], BRIDGE_FORMAT_REC))
    return true;
  else
  {
    LOG("Cannot add " + info + " line '" + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writeREC(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such REC file");
    return false;
  }

  fstr << "% REC " << 
    GuessOriginalLine(group.GetFileName(), group.GetCount()) << "\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  const formatType f = BRIDGE_FORMAT_REC;
  Canvas canvas;

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    const unsigned numBoards = segment->GetLength();
    for (unsigned b = 0; b < numBoards; b++)
    {
      Board * board = segment->GetBoard(b);
      if (board == nullptr)
      {
        LOG("Invalid board");
        fstr.close();
        return false;
      }

      const unsigned numInstances = board->GetLength();
      for (unsigned i = 0; i < numInstances; i++)
      {
        if (! board->SetInstance(i))
        {
          LOG("Invalid instance");
          fstr.close();
          return false;
        }

        board->CalculateScore();

        const string dstr = board->DealAsString(BRIDGE_WEST, f);
        const string sstr = segment->ScoringAsString(f);
        const string estr = board->DealerAsString(f);
        const string bstr = segment->NumberAsString(f, b);
        const string vstr = board->VulAsString(f);

        const string west = board->WestAsString(f);
        const string north = board->NorthAsString(f);
        const string east = board->EastAsString(f);
        const string south = board->SouthAsString(f);

        // Convert deal, auction and play from \n to vectors.
        vector<string> deal;
        ConvertMultilineToVector(dstr, deal);

        canvas.SetDimensions(11, 80);
        canvas.SetRectangle(deal, 0, 0);

        canvas.SetLine(sstr, 0, 0);
        canvas.SetLine(estr, 0, 24);
        canvas.SetLine(bstr, 1, 0);
        canvas.SetLine(vstr, 1, 24);

        canvas.SetLine(north, 0, 12);
        canvas.SetLine(west, 3, 0);
        canvas.SetLine(east, 3, 24);
        canvas.SetLine(south, 6, 12);

        fstr << canvas.AsString() << "\n";

        fstr << board->PlayersAsString(f) << "\n";
        fstr << board->AuctionAsString(f) << "\n";

        fstr << board->LeadAsString(f) << "    ";
        fstr << board->ResultAsString(f, false) << "\n";
        fstr << board->ScoreAsString(f, false);
        fstr << board->ScoreIMPAsString(f, i == 1) << "\n\n";

        fstr << board->PlayAsString(f) << "\n";
      }
    }
  }

  fstr.close();
  return true;
}

