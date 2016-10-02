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
#include "Group.h"
#include "Segment.h"
#include "Debug.h"
#include "fileTXT.h"
#include "parse.h"
#include "portab.h"

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
  TXT_PLAY = 17,
  TXT_RESULT = 18,
  TXT_SCORE = 19,
  TXT_SCORE_IMP = 20,
  TXT_LABELS_SIZE = 21
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


static bool tryTXTMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

static bool readTXTCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine);

static bool getTXTFields(
  const vector<string>& canvas,
  const unsigned auctionLine,
  vector<string>& chunk);

static bool getTXTDeal(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk);

static bool getTXTAuction(
  const vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk);

static bool getTXTPlay(
  const vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk);


void setTXTTables()
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


static bool readTXTCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas)
{
  string line;
  bool seenDeal = false;
  while (getline(fstr, line))
  {
    lno++;
    if (line.empty())
    {
      if (! seenDeal)
        continue;

      string& prevLine = canvas.back();
      string wd;
      if (ReadNextWord(prevLine, 0, wd) && (wd == "Down" || wd == "Made"))
      {
        int seen = count(prevLine.begin(), prevLine.end(), ' ');
        if (seen > 4)
          // Team line still to come
          continue;
        else
          break;
      }
      else
        continue;
    }
    else if (line.at(0) == '%')
      continue;

    if (line.size() > 25)
    {
      if (line.substr(0, 4) == "West")
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


static bool getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine)
{
  auctionLine = 0;
  string wd = "";
  while (auctionLine < canvas.size())
  {
    if (canvas[auctionLine].size() > 12 &&
        canvas[auctionLine].substr(0, 4) == "West")
      break;
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


static bool getTXTFields(
  const vector<string>& canvas,
  const unsigned aline,
  vector<string>& chunk)
{
  unsigned bline = 0;
  if (canvas[0].size() < 19 || 
     (canvas[0].substr(12, 7) != "  North" &&
      canvas[1].substr(0, 4) != "West"))
  {
    chunk[BRIDGE_FORMAT_TITLE] = canvas[0];
    chunk[BRIDGE_FORMAT_DATE] = canvas[1];
    chunk[BRIDGE_FORMAT_LOCATION] = canvas[2];
    chunk[BRIDGE_FORMAT_EVENT] = canvas[3];
    chunk[BRIDGE_FORMAT_SESSION] = canvas[4];
    chunk[BRIDGE_FORMAT_SCORING] = "IMPs"; // Maybe others possible
    chunk[BRIDGE_FORMAT_TEAMS] = canvas[5];
    bline = 6;
  }

  if (aline > 10)
  {
    if (! ReadNextWord(canvas[bline], 0, chunk[BRIDGE_FORMAT_BOARD_NO])) 
      return false;
    chunk[BRIDGE_FORMAT_BOARD_NO].pop_back(); // Drop trailing point

    if (! ReadNextWord(canvas[bline+14], 0, chunk[BRIDGE_FORMAT_VULNERABLE])) 
      return false;

    if (! getTXTDeal(canvas, bline, chunk)) 
      return false;
  }

  if (! ReadNextWord(canvas[aline], 0, chunk[BRIDGE_FORMAT_WEST])) return false;
  if (! ReadNextWord(canvas[aline], 12, chunk[BRIDGE_FORMAT_NORTH])) return false;
  if (! ReadNextWord(canvas[aline], 24, chunk[BRIDGE_FORMAT_EAST])) return false;
  if (! ReadNextWord(canvas[aline], 36, chunk[BRIDGE_FORMAT_SOUTH])) return false;


  unsigned cline = aline+2;
  if (! getTXTAuction(canvas, cline, chunk))
    return false;

  chunk[BRIDGE_FORMAT_CONTRACT] = canvas[cline++];

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
    chunk[BRIDGE_FORMAT_PLAY] = wd;
  }

  cline++;
  if (canvas[cline].size() < 5)
    return false;
  chunk[BRIDGE_FORMAT_RESULT] = canvas[cline];

  // Ignore running IMP score, as we regenerate this.
  return true;
}


static bool getTXTDeal(
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

  if (! ReadNextSpacedWord(canvas[offset+1], 14, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+2], 14, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+3], 14, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+4], 14, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[offset+6], 26, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+7], 26, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+8], 26, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+9], 26, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextSpacedWord(canvas[offset+11], 14, sts)) sts = "";
  if (! ReadNextSpacedWord(canvas[offset+12], 14, sth)) sth = "";
  if (! ReadNextSpacedWord(canvas[offset+13], 14, std)) std = "";
  if (! ReadNextSpacedWord(canvas[offset+14], 14, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;


  // Turn -- (void) into nothing.
  regex re("--");
  chunk[BRIDGE_FORMAT_DEAL] = regex_replace(d.str(), re, "");
  return true;
}


static bool getTXTAuction(
  const vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  unsigned firstStart = 0;
  while (firstStart < 48 && canvas[offset].at(firstStart) == ' ')
    firstStart += 12;

  if (firstStart >= 48)
    return false;

  chunk[BRIDGE_FORMAT_DEALER] = PLAYER_NAMES_LONG[
    ((firstStart/12) + BRIDGE_WEST) % 4];

  string wd;
  unsigned no = 0;
  bool done = false;
  unsigned l;
  unsigned numPasses = 0;
  for (l = offset; l < canvas.size(); l++)
  {
    for (unsigned beg = (l == offset ? firstStart : 0); beg < 48; beg += 12)
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

  chunk[BRIDGE_FORMAT_AUCTION] = d.str();
  offset = l+1;
  return true;
}


static bool getTXTPlay(
  const vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  bool done = false;
  string wd;
  unsigned l;
  for (l = offset; l < canvas.size(); l++)
  {
    if (canvas[l].substr(1, 2) != ". " && canvas[l].substr(2, 2) != ". ")
    {
      l--;
      break;
    }

    if (l != offset)
      d << ":";

    for (unsigned p = 8; p < 36; p += 8)
    {
      if (p > 20)
        p--; // WTF?

      if (! ReadNextWord(canvas[l], p, wd))
      {
        done = true;
        break;
      }
      if (wd == "10")
        d << "T";
      else if (wd.size() == 3 && wd.substr(1, 2) == "10")
        d << wd.substr(0, 1) << "T";
      else
        d << wd;
    }
    if (done)
      break;
  }

  if (l == canvas.size())
    return false;
  offset = l;
  chunk[BRIDGE_FORMAT_PLAY] = d.str();
  return true;
}


bool readTXTChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readTXTCanvas(fstr, lno, canvas))
    return false;
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned auctionLine = 0;
  if (! getTXTCanvasOffset(canvas, auctionLine))
    return false;

  newSegFlag = false;
  return getTXTFields(canvas, auctionLine, chunk);
}


bool readTXT(
  Group& group,
  const string& fname)
{
  bool newSegFlag = true;

  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such TXT file");
    return false;
  }

  group.SetFileName(fname);

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
  while (readTXTChunk(fstr, lno, chunk, newSegFlag))
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


static bool tryTXTMethod(
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

  fstr << "% TXT " << 
    GuessOriginalLine(group.GetFileName(), group.GetCount()) << "\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  const formatType f = BRIDGE_FORMAT_TXT;
  Canvas canvas;

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    fstr << "\n" << segment->TitleAsString(f) << "\n";
    fstr << segment->DateAsString(f);
    fstr << segment->LocationAsString(f);
    fstr << segment->EventAsString(f);
    fstr << segment->SessionAsString(f);
    fstr << segment->TeamsAsString(f) << "\n\n";

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
        }

        fstr << setw(12) << left << board->WestAsString(f) <<
            setw(12) << board->NorthAsString(f) <<
            setw(12) << board->EastAsString(f) <<
            board->SouthAsString(f) << "\n";

        fstr << board->AuctionAsString(f) << "\n";
        fstr << board->ContractAsString(f);
        fstr << board->PlayAsString(f);

        if (i == 0)
          fstr << board->ResultAsString(f, false) << "\n";
        else if (i == 1)
        {
          int s = board->ScoreIMPAsInt();
          string tWin;
          if (s > 0)
          {
            score2 += static_cast<unsigned>(s);
            tWin = segment->SecondTeamAsString(f);
          }
          else
          {
            score1 += static_cast<unsigned>(-s);
            tWin = segment->FirstTeamAsString(f);
          }

          fstr << board->ResultAsString(f, tWin) << "\n";
          fstr << segment->TeamsAsString(score1, score2, f) << "\n";
          if (b != numBoards-1)
            fstr << TXTdashes << "\n\n";
        }
      }
    }
  }

  fstr.close();
  return true;
}

