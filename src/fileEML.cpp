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

#include "Board.h"
#include "Canvas.h"
#include "Debug.h"
#include "Group.h"
#include "Segment.h"
#include "fileEML.h"
#include "bconst.h"
#include "parse.h"
#include "portab.h"

using namespace std;

extern Debug debug;


const unsigned EMLlineLength = 80;

enum EMLlabel
{
  EML_SCORING = 0,
  EML_WEST = 1,
  EML_NORTH = 2,
  EML_EAST = 3,
  EML_SOUTH = 4,
  EML_BOARD = 5,
  EML_DEAL = 6,
  EML_DEALER = 7,
  EML_VULNERABLE = 8,
  EML_AUCTION = 9,
  EML_LEAD = 10,
  EML_PLAY = 11,
  EML_RESULT = 12,
  EML_SCORE = 13,
  EML_SCORE_IMP = 14,
  EML_LABELS_SIZE = 15
};

const string EMLname[] =
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
  "Lead",
  "Play",
  "Result",
  "Score",
  "ScoreIMP"
};

string EMLdashes, EMLequals;
string EMLshortDashes, EMLshortEquals;


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrEML[EML_LABELS_SIZE];
BoardPtr boardPtrEML[EML_LABELS_SIZE];


static bool tryEMLMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

static bool readEMLCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getEMLCanvasWest(
  const vector<string>& canvas,
  const unsigned pos,
  const unsigned openingLine,
  unsigned& westLine,
  unsigned& cardStart);

static bool getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned& openingLine,
  unsigned& westLine);

static bool getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk);

static bool getEMLAuction(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk);

static bool getEMLPlay(
  const vector<string>& canvas,
  const unsigned openingLine,
  const unsigned westLine,
  const unsigned cardStart,
  vector<string>& chunk);


void setEMLtables()
{
  segPtrEML[EML_SCORING] = &Segment::SetScoring;
  segPtrEML[EML_WEST] = &Segment::SetWest;
  segPtrEML[EML_NORTH] = &Segment::SetNorth;
  segPtrEML[EML_EAST] = &Segment::SetEast;
  segPtrEML[EML_SOUTH] = &Segment::SetSouth;

  segPtrEML[EML_BOARD] = &Segment::SetNumber;

  boardPtrEML[EML_DEAL] = &Board::SetDeal;
  boardPtrEML[EML_DEALER] = &Board::SetDealer;
  boardPtrEML[EML_VULNERABLE] = &Board::SetVul;
  boardPtrEML[EML_AUCTION] = &Board::SetAuction;
  boardPtrEML[EML_PLAY] = &Board::SetPlays;

  boardPtrEML[EML_RESULT] = &Board::SetResult;
  boardPtrEML[EML_SCORE] = &Board::SetScore;
  boardPtrEML[EML_SCORE_IMP] = &Board::SetScoreIMP;

  EMLdashes.resize(0);
  EMLdashes.insert(0, 12, ' ');
  EMLdashes.insert(12, 43, '-');

  EMLequals.resize(0);
  EMLequals.insert(0, 79, '=');

  EMLshortDashes.resize(0);
  EMLshortDashes.insert(0, 10, '-');

  EMLshortEquals.resize(0);
  EMLshortEquals.insert(0, 10, '=');
}


static bool readEMLCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas)
{
  string line;
  while (getline(fstr, line))
  {
    lno++;
    if (line.empty() || line.at(0) == '%')
      continue;

    if (line.size() > 40)
    {
      const string mid = line.substr(30, 10);
      if (mid == EMLshortDashes || mid == EMLshortEquals)
        break;
    }

    canvas.push_back(line);
  }
  return (canvas.size() >= 18);
}


static bool getEMLCanvasWest(
  const vector<string>& canvas,
  const unsigned pos,
  const unsigned openingLine,
  unsigned& westLine,
  unsigned& cardStart)
{
  westLine = openingLine;
  cardStart = 0;
  string wd = "";
  while (westLine < canvas.size())
  {
    if (canvas[westLine].size() >= pos+1 &&
        ReadNextWord(canvas[westLine], pos, wd) && 
        wd == "W")
    {
      cardStart = pos+3;
      break;
    }
    westLine++;
  }

  return (cardStart > 0);
}


static bool getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned& openingLine,
  unsigned& westLine,
  unsigned& cardStart)
{
  openingLine = 5;
  string wd = "";
  while (openingLine < canvas.size())
  {
    if (ReadNextWord(canvas[openingLine], 42, wd) && wd == "Opening")
      break;
    openingLine++;
  }

  if (wd != "Opening")
    return false;

  if (getEMLCanvasWest(canvas, 42, openingLine, westLine, cardStart))
    return true;
  else if (getEMLCanvasWest(canvas, 39, openingLine, westLine, cardStart))
    return true;
  else
    return false;
}


static bool getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk)
{
  if (! ReadNextWord(canvas[0], 0, chunk[EML_SCORING]))
    return false;

  if (! ReadNextWord(canvas[1], 16, chunk[EML_NORTH]))
    return false;
  if (! ReadNextWord(canvas[7], 4, chunk[EML_WEST]))
    return false;
  if (! ReadNextWord(canvas[7], 27, chunk[EML_EAST]))
    return false;
  if (! ReadNextWord(canvas[13], 16, chunk[EML_SOUTH]))
    return false;

  if (! ReadNextWord(canvas[0], 54, chunk[EML_BOARD]))
    return false;
  if (! ReadNextWord(canvas[1], 5, chunk[EML_DEALER]))
    return false;
  if (! ReadNextWord(canvas[2], 5, chunk[EML_VULNERABLE]))
    return false;

  if (! ReadNextWord(canvas[openingLine+1], 50, chunk[EML_RESULT]))
    return false;

  if (! ReadNextWord(canvas[openingLine+2], 49, chunk[EML_SCORE]))
    return false;
  chunk[EML_SCORE].pop_back(); // Drop trailing comma

  if (canvas[openingLine+2].back() != ':')
  {
    if (! ReadLastWord(canvas[openingLine+2], chunk[EML_SCORE_IMP]))
      return false;
  }
  return true;
}


static bool getEMLDeal(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! ReadNextWord(canvas[8], 6, sts)) sts = "";
  if (! ReadNextWord(canvas[9], 6, sth)) sth = "";
  if (! ReadNextWord(canvas[10], 6, std)) std = "";
  if (! ReadNextWord(canvas[11], 6, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextWord(canvas[2], 18, sts)) sts = "";
  if (! ReadNextWord(canvas[3], 18, sth)) sth = "";
  if (! ReadNextWord(canvas[4], 18, std)) std = "";
  if (! ReadNextWord(canvas[5], 18, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextWord(canvas[8], 29, sts)) sts = "";
  if (! ReadNextWord(canvas[9], 29, sth)) sth = "";
  if (! ReadNextWord(canvas[10], 29, std)) std = "";
  if (! ReadNextWord(canvas[11], 29, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextWord(canvas[14], 18, sts)) sts = "";
  if (! ReadNextWord(canvas[15], 18, sth)) sth = "";
  if (! ReadNextWord(canvas[16], 18, std)) std = "";
  if (! ReadNextWord(canvas[17], 18, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;


  chunk[EML_DEAL] = d.str();
  return true;
}


static bool getEMLAuction(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  unsigned firstStart = 42;
  while (firstStart < 75 && canvas[5].at(firstStart) == ' ')
    firstStart += 9;

  if (firstStart >= 75)
    return false;

  string wd;
  unsigned no = 0;
  for (unsigned l = 5; l < openingLine-1; l++)
  {
    for (unsigned beg = (l == 5 ? firstStart : 42); beg < 75; beg += 9)
    {
      if (! ReadNextWord(canvas[l], beg, wd))
      {
        if (l == openingLine-1)
          break;
        else
          return false;
      }
      if (no > 0 && no % 4 == 0)
        d << ":";

      if (wd == "XX")
        d << "R";
      else if (wd.size() == 3 && wd.at(1) == 'N' && wd.at(2) == 'T')
      {
        wd.erase(2, 1);
        d << wd;
      }
      else if (wd == "pass")
        d << "P";
      else if (wd == "(all")
      {
        d << "A";
        break;
      }
      else
        d << wd;
      no++;
    }
  }

  chunk[EML_AUCTION] = d.str();
  return true;
}


static bool getEMLPlay(
  const vector<string>& canvas,
  const unsigned openingLine,
  const unsigned westLine,
  const unsigned cardStart,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  string p1, p2, p3, p4;
  string opld;
  if (! ReadNextWord(canvas[openingLine], 56, opld))
    return false;

  string wd;
  unsigned pos0 = cardStart;
  unsigned la = canvas[westLine-1].size();
  while (pos0 < la)
  {
    unsigned h;
    bool found = false;
    if (pos0 == cardStart)
    {
      // Find the opening lead
      for (h = 0; h < 4; h++)
      {
        if (canvas[westLine+h].size() >= cardStart+1 &&
            ReadNextWord(canvas[westLine+h], cardStart-1, cardStart, wd) &&
            wd == opld)
        {
          found = true;
          break;
        }
      }

      if (! found)
        return false;
    }
    else
    {
      // Find the dash
      for (h = 0; h < 4; h++)
      {
        if (canvas[westLine+h].size() >= pos0-1 &&
            canvas[westLine+h].at(pos0-2) == '-')
        {
          found = true;
          break;
        }
      }

      if (! found)
        return false;
    }

    for (unsigned p = 0; p < 4; p++)
    {
      unsigned l = westLine + ((h+p) % 4);

      // Done?
      if (pos0 > canvas[l].size())
        break;

      if (canvas[l].at(pos0-1) == ' ')
        d << canvas[l].at(pos0);
      else
      {
        if (! ReadNextWord(canvas[l], pos0-1, pos0, wd))
          return false;
        d << wd;
        
      }
    }
    if (pos0 < la-1)
      d << ":";

    pos0 += 3;
  }
      
  chunk[EML_PLAY] = d.str();
  return true;
}


bool readEMLChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  newSegFlag = true;

  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readEMLCanvas(fstr, lno, canvas))
    return false;
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < EML_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned openingLine = 0;
  unsigned westLine = 0;
  unsigned cardStart = 0;
  if (! getEMLCanvasOffset(canvas, openingLine, westLine, cardStart))
    return false;

  if (! getEMLSimpleFields(canvas, openingLine, chunk))
    return false;

  // Synthesize an RBN-style deal.
  if (! getEMLDeal(canvas, chunk))
    return false;

  // Synthesize an RBN-style string of bids.
  if (! getEMLAuction(canvas, openingLine, chunk))
    return false;

  // Synthesize an RBN-style string of plays.
  if (! getEMLPlay(canvas, openingLine, westLine, cardStart, chunk))
    return false;

  return true;
}


bool readEML(
  Group& group,
  const string& fname)
{
  bool newSegFlag = true;

  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such EML file");
    return false;
  }

  const formatType f = BRIDGE_FORMAT_EML;

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

  vector<string> chunk(EML_LABELS_SIZE);
  while (readEMLChunk(fstr, lno, chunk, newSegFlag))
  {
    if (chunk[EML_BOARD] != "" && chunk[EML_BOARD] != lastBoard)
    {
      // New board.
      lastBoard = chunk[EML_BOARD];
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

    for (unsigned i = 0; i < EML_LABELS_SIZE; i++)
    {
      if (! tryEMLMethod(chunk, segment, board, i, fstr, EMLname[i]))
        return false;
    }

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


static bool tryEMLMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= EML_BOARD)
  {
    if ((segment->*segPtrEML[label])(chunk[label], BRIDGE_FORMAT_EML))
      return true;
    else
    {
      LOG("Cannot add " + info + " line '" + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else if ((board->*boardPtrEML[label])(chunk[label], BRIDGE_FORMAT_EML))
    return true;
  else
  {
    LOG("Cannot add " + info + " line '" + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


bool writeEML(
  Group& group,
  const string& fname)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such EML file");
    return false;
  }

  fstr << "% EML\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";

  const formatType f = BRIDGE_FORMAT_EML;
  string chunk[EML_LABELS_SIZE];
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

        chunk[EML_SCORING] = segment->ScoringAsString(f);
        chunk[EML_BOARD] = segment->NumberAsString(f, b);

        chunk[EML_WEST] = board->WestAsString(f);
        chunk[EML_NORTH] = board->NorthAsString(f);
        chunk[EML_EAST] = board->EastAsString(f);
        chunk[EML_SOUTH] = board->SouthAsString(f);

        chunk[EML_DEAL] = board->DealAsString(BRIDGE_WEST, f);
        chunk[EML_DEALER] = board->DealerAsString(f);
        chunk[EML_VULNERABLE] = board->VulAsString(f);
        chunk[EML_AUCTION] = board->AuctionAsString(f);
        chunk[EML_LEAD] = board->LeadAsString(f);
        chunk[EML_PLAY] = board->PlayAsString(f);
        chunk[EML_RESULT] = board->ResultAsString(f, false);
        chunk[EML_SCORE] = board->ScoreAsString(f, 
          segment->ScoringIsIMPs());

        // Convert deal, auction and play from \n to vectors.
        vector<string> deal, auction, play;
        ConvertMultilineToVector(chunk[EML_DEAL], deal);
        ConvertMultilineToVector(chunk[EML_AUCTION], auction);
        ConvertMultilineToVector(chunk[EML_PLAY], play);

        // Height of auction determines dimensions.
        const unsigned a = auction.size();
        const unsigned alstart = (a <= 6 ? 14 : a+8);
        const unsigned clen = alstart + 5;
        const unsigned acstart = (play[0].length() > 38 ? 39u : 42u);

        canvas.SetDimensions(clen, 80);

        canvas.SetRectangle(deal, 0, 4);
        canvas.SetRectangle(auction, 2, 42);
        canvas.SetRectangle(play, alstart, acstart);

        canvas.SetLine(chunk[EML_SCORING], 0, 0);
        canvas.SetLine(chunk[EML_WEST], 7, 4);
        canvas.SetLine(chunk[EML_NORTH], 1, 16);
        canvas.SetLine(chunk[EML_EAST], 7, 27);
        canvas.SetLine(chunk[EML_SOUTH], 13, 16);
        canvas.SetLine(chunk[EML_WEST], 3, 42);
        canvas.SetLine(chunk[EML_NORTH], 3, 51);
        canvas.SetLine(chunk[EML_EAST], 3, 60);
        canvas.SetLine(chunk[EML_SOUTH], 3, 69);
        canvas.SetLine(chunk[EML_BOARD], 0, 42);

        canvas.SetLine(chunk[EML_DEALER], 1, 0);
        canvas.SetLine(chunk[EML_VULNERABLE], 2, 0);
        canvas.SetLine(chunk[EML_LEAD], a+3, 42);
        canvas.SetLine(chunk[EML_RESULT], a+4, 42);
        canvas.SetLine(chunk[EML_SCORE], a+5, 42);

        fstr << canvas.AsString() << "\n";

        if (i < numInstances-1)
          fstr << EMLdashes << "\n\n";
        else if (b < numBoards-1)
          fstr << EMLequals << "\n\n";
      }
    }
  }

  fstr.close();
  return true;
}

