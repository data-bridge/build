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
#include "fileTXT.h"
#include "parse.h"
#include "portab.h"
#include "debug.h"

using namespace std;

extern Debug debug;


const unsigned TXTlineLength = 80;

enum TXTlabel
{
  TXT_TITLE = 0,
  TXT_DATE = 1,
  TXT_LOCATION = 2,
  TXT_EVENT = 3,
  TXT_SESSION = 4,
  TXT_SCORING = 5,
  TXT_TEAMS = 6,
  TXT_WEST = 7,
  TXT_NORTH = 8,
  TXT_EAST = 9,
  TXT_SOUTH = 10,
  TXT_BOARD = 11,
  TXT_DEAL = 12,
  TXT_DEALER = 13,
  TXT_VULNERABLE = 14,
  TXT_AUCTION = 15,
  TXT_CONTRACT = 16,
  TXT_PLAY = 18,
  TXT_RESULT = 19,
  TXT_SCORE = 20,
  TXT_SCORE_IMP = 21,
  TXT_LABELS_SIZE = 22
};

const string TXTname[] =
{
  "Title",
  "Date",
  "Location",
  "Event",
  "Session",
  "Scoring",
  "Teams",
  "West",
  "North",
  "East",
  "South",
  "Board",
  "Deal",
  "Dealer",
  "Vulnerable",
  "Auction",
  "Contract",
  "Play",
  "Result",
  "Score",
  "ScoreIMP"
};

string TXTdashes;
string TXTshortDashes;


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrTXT[TXT_LABELS_SIZE];
BoardPtr boardPtrTXT[TXT_LABELS_SIZE];


bool tryTXTMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

bool readTXTCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

bool getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine);

bool getTXTFields(
  const vector<string>& canvas,
  const unsigned auctionLine,
  vector<string>& chunk);

bool getTXTDeal(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk);

bool getTXTAuction(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk);

bool getTXTPlay(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk);

bool readTXTChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk);


void setTXTtables()
{
  segPtrTXT[TXT_TITLE] = &Segment::SetTitle;
  segPtrTXT[TXT_DATE] = &Segment::SetDate;
  segPtrTXT[TXT_LOCATION] = &Segment::SetLocation;
  segPtrTXT[TXT_EVENT] = &Segment::SetEvent;
  segPtrTXT[TXT_SESSION] = &Segment::SetSession;
  segPtrTXT[TXT_SCORING] = &Segment::SetScoring;
  segPtrTXT[TXT_TEAMS] = &Segment::SetTeams;

  segPtrTXT[TXT_WEST] = &Segment::SetWest;
  segPtrTXT[TXT_NORTH] = &Segment::SetNorth;
  segPtrTXT[TXT_EAST] = &Segment::SetEast;
  segPtrTXT[TXT_SOUTH] = &Segment::SetSouth;

  segPtrTXT[TXT_BOARD] = &Segment::SetNumber;

  boardPtrTXT[TXT_DEAL] = &Board::SetDeal;
  boardPtrTXT[TXT_DEALER] = &Board::SetDealer;
  boardPtrTXT[TXT_VULNERABLE] = &Board::SetVul;
  boardPtrTXT[TXT_AUCTION] = &Board::SetAuction;
  boardPtrTXT[TXT_CONTRACT] = &Board::SetContract;
  boardPtrTXT[TXT_PLAY] = &Board::SetPlays;

  boardPtrTXT[TXT_RESULT] = &Board::SetResult;
  boardPtrTXT[TXT_SCORE] = &Board::SetScore;
  boardPtrTXT[TXT_SCORE_IMP] = &Board::SetScoreIMP;

  TXTdashes.resize(0);
  TXTdashes.insert(0, 41, '-');

  TXTshortDashes.resize(0);
  TXTshortDashes.insert(0, 10, '-');
}


bool readTXTCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas)
{
  string line;
  bool seenDeal = false;
  while (getline(fstr, line))
  {
    lno++;
    if (line.at(0) == '%')
      continue;

    if (line.empty())
    {
      if (! seenDeal)
        continue;

      string& prevLine = canvas.back();
      string wd;
      if (ReadNextWord(prevLine, 0, wd) && (wd == "Down" || wd == "Made"))
        break;
    }

    if (line.size() > 25)
    {
      if (line.substr(0, 6) == "West")
        seenDeal = true;
      else
      {
        const string mid = line.substr(10, 10);
        if (mid == TXTshortDashes)
          break;
      }
    }

    canvas.push_back(line);
  }
  return true;
}


bool getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine)
{
  auctionLine = 0;
  string wd = "";
  while (auctionLine < canvas.size())
  {
    if (canvas[auctionLine].size() > 12 &&
        ReadNextWord(canvas[auctionLine], 0, wd))
    {
      if (wd == "West")
        break;
    }
    auctionLine++;
  }

  if (auctionLine == canvas.size())
    return false;
  else
  {
    auctionLine--;
    return true;
  }
}


bool getTXTFields(
  const vector<string>& canvas,
  const unsigned aline,
  vector<string>& chunk)
{
  if (aline > 10)
  {
    if (! ReadNextWord(canvas[1], 0, chunk[TXT_TITLE])) return false;
    if (! ReadNextWord(canvas[3], 0, chunk[TXT_DATE])) return false;
    if (! ReadNextWord(canvas[4], 0, chunk[TXT_LOCATION])) return false;
    if (! ReadNextWord(canvas[5], 0, chunk[TXT_EVENT])) return false;
    if (! ReadNextWord(canvas[6], 0, chunk[TXT_SESSION])) return false;
    chunk[TXT_SCORING] = "IMPs"; // Maybe others possible
    if (! ReadNextWord(canvas[7], 0, chunk[TXT_TEAMS])) return false;

    if (! ReadNextWord(canvas[9], 0, chunk[TXT_BOARD])) return false;
    chunk[TXT_BOARD].pop_back(); // Drop trailing point

    if (! ReadNextWord(canvas[23], 0, chunk[TXT_VULNERABLE])) return false;

    if (! getTXTDeal(canvas, 9, chunk)) return false;
  }

  if (! ReadNextWord(canvas[aline], 0, chunk[TXT_WEST])) return false;
  if (! ReadNextWord(canvas[aline], 12, chunk[TXT_NORTH])) return false;
  if (! ReadNextWord(canvas[aline], 24, chunk[TXT_EAST])) return false;
  if (! ReadNextWord(canvas[aline], 36, chunk[TXT_SOUTH])) return false;


  unsigned cline = aline+2;
  if (! getTXTAuction(canvas, cline, chunk))
    return false;

  cline++;
  chunk[TXT_CONTRACT] = canvas[cline++];

  if (canvas[cline].size() < 5)
    return false;

  string wd = canvas[cline].substr(0, 5);
  if (wd == "Trick")
  {
    cline++;
    if (! getTXTPlay(canvas, cline, chunk))
      return false;
  }
  else if (wd == "Lead:")
  {
    if (! ReadLastWord(canvas[cline], wd))
      return false;
    chunk[TXT_PLAY] = wd;
  }

  cline++;
  if (canvas[cline].size() < 5)
    return false;
  chunk[TXT_RESULT] = canvas[cline];

  // Ignore running IMP score, as we regenerate this.
  return true;
}


bool getTXTDeal(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! ReadNextSpacedWord(canvas[offset+6], 2, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+7], 2, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+8], 2, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+9], 2, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[offset+1], 12, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+2], 12, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+3], 12, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+4], 12, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[offset+6], 24, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+7], 24, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+8], 24, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+9], 24, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[offset+11], 12, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+11], 12, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+11], 12, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+11], 12, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;


  chunk[TXT_DEAL] = d.str();
  return true;
}


bool getTXTAuction(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  unsigned firstStart = 0;
  while (firstStart < 48 && canvas[offset].at(firstStart) == ' ')
    firstStart += 12;

  if (firstStart >= 48)
    return false;

  string wd;
  unsigned no = 0;
  for (unsigned l = offset; l < canvas.size(); l++)
  {
    for (unsigned beg = (l == offset ? firstStart : 0); beg < 48; beg += 12)
    {
      if (! ReadNextWord(canvas[l], beg, wd))
        break;

      if (no > 0 && no % 4 == 0)
        d << ":";

      if (wd == "Dbl")
        d << "X";
      else if (wd == "Rdbl")
        d << "R";
      else if (wd.size() == 3 && wd.at(1) == 'N' && wd.at(2) == 'T')
      {
        wd.erase(2, 1);
        d << wd;
      }
      else if (wd == "Pass")
        d << "P";
      else if (wd == "All")
      {
        d << "A";
        break;
      }
      else
        d << wd;
      no++;
    }
  }

  chunk[TXT_AUCTION] = d.str();
  return true;
}


bool getTXTPlay(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  bool done = false;
  string wd;
  for (unsigned l = offset; l < canvas.size(); l++)
  {
    for (unsigned p = 8; p < 40; p += 8)
    {
      if (! ReadNextWord(canvas[l], p, wd))
      {
        done = true;
        break;
      }
      if (wd == "10")
        d << "T";
      else if (wd.size() == 3 && wd.substr(1, 2) == "NT")
        d << wd.substr(0, 1) << "N";
      else
        d << wd;
    }
    if (done)
      break;
    d << ":";
  }

  chunk[TXT_PLAY] = d.str();
  return true;
}


bool readTXTChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readTXTCanvas(fstr, lno, canvas))
    return false;
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < TXT_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned auctionLine = 0;
  if (! getTXTCanvasOffset(canvas, auctionLine))
    return false;

  return getTXTFields(canvas, auctionLine, chunk);
}


bool readTXT(
  Group& group,
  const string& fname)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such TXT file");
    return false;
  }

  const formatType f = BRIDGE_FORMAT_TXT;

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

  vector<string> chunk(TXT_LABELS_SIZE);
  while (readTXTChunk(fstr, lno, chunk))
  {
    if (chunk[TXT_BOARD] != "" && chunk[TXT_BOARD] != lastBoard)
    {
      // New board.
      lastBoard = chunk[TXT_BOARD];
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

    for (unsigned i = 0; i < TXT_LABELS_SIZE; i++)
    {
      if (! tryTXTMethod(chunk, segment, board, i, fstr, TXTname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


bool tryTXTMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= TXT_BOARD)
  {
    if ((segment->*segPtrTXT[label])(chunk[label], BRIDGE_FORMAT_TXT))
      return true;
    else
    {
      LOG("Cannot add " + info + " line '" + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else if ((board->*boardPtrTXT[label])(chunk[label], BRIDGE_FORMAT_TXT))
    return true;
  else
  {
    LOG("Cannot add " + info + " line '" + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writeTXT(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such TXT file");
    return false;
  }

  fstr << "% TXT\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  const formatType f = BRIDGE_FORMAT_TXT;
  Canvas canvas;

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << "\n" << segment->TitleAsString(f) << "\n";
    fstr << segment->TitleAsString(f);
    fstr << segment->DateAsString(f);
    fstr << segment->LocationAsString(f);
    fstr << segment->EventAsString(f);
    fstr << segment->SessionAsString(f);
    fstr << segment->TeamsAsString(f) << "\n";

    unsigned score1 = 0, score2 = 0;
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


        if (i == 0)
        {
          const string dstr = board->DealAsString(BRIDGE_WEST, f);
          const string bstr = segment->NumberAsString(f, b);
          const string vstr = board->VulAsString(f);

          // Convert deal, auction and play from \n to vectors.
          vector<string> deal;
          ConvertMultilineToVector(dstr, deal);

          canvas.SetDimensions(15, 80);
          canvas.SetRectangle(deal, 0, 0);
          canvas.SetLine(bstr, 0, 0);
          canvas.SetLine(vstr, 14, 0);
          fstr << canvas.AsString() << "\n";

          fstr << setw(12) << board->WestAsString(f) <<
              setw(12) << board->NorthAsString(f) <<
              setw(12) << board->EastAsString(f) <<
              setw(12) << board->SouthAsString(f) << "\n";
        }

        fstr << board->AuctionAsString(f) << "\n";
        fstr << board->ContractAsString(f) << "\n";
        fstr << board->PlayAsString(f) << "\n";
        fstr << board->ResultAsString(f, false) << " -- ";
        fstr << board->ScoreAsString(f, segment->ScoringIsIMPs()) << "\n";

        if (i == 1)
        {
          int s = board->ScoreIMPAsInt();
          if (s > 0)
            score1 += s;
          else
            score2 += s;

          fstr << segment->TeamsAsString(score1, score2, f) << "\n";
          fstr << TXTdashes;
        }
      }
    }
  }

  fstr.close();
  return true;
}

