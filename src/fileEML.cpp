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

#include "fileEML.h"
#include "parse.h"
#include "portab.h"
#include "debug.h"

using namespace std;

extern Debug debug;


const unsigned EMLlineLength = 80;
const unsigned EMLlineCount = 30;

enum EMLlabel
{
  EML_SCORING = 0,
  EML_WEST = 1,
  EML_NORTH = 2,
  EML_EAST = 3,
  EML_SOUTH = 4,
  EML_BOARD = 5,
  EML_ROOM = 6,
  EML_DEAL = 7,
  EML_DEALER = 8,
  EML_VULNERABLE = 9,
  EML_AUCTION = 10,
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
  "Room",
  "Deal",
  "Dealer",
  "Vulnerable",
  "Auction",
  "Play",
  "Result",
  "Score",
  "ScoreIMP"
};

string EMLdashes, EMLequals;


typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtrEML[EML_LABELS_SIZE];
BoardPtr boardPtrEML[EML_LABELS_SIZE];


void setEMLtables()
{
  segPtrEML[EML_SCORING] = &Segment::SetScoring;
  segPtrEML[EML_WEST] = &Segment::SetWest;
  segPtrEML[EML_NORTH] = &Segment::SetNorth;
  segPtrEML[EML_EAST] = &Segment::SetEast;
  segPtrEML[EML_SOUTH] = &Segment::SetSouth;

  segPtrEML[EML_BOARD] = &Segment::SetNumber;
  segPtrEML[EML_ROOM] = &Segment::SetRoom;

  boardPtrEML[EML_DEAL] = &Board::SetDeal;
  boardPtrEML[EML_DEALER] = &Board::SetDealer;
  boardPtrEML[EML_VULNERABLE] = &Board::SetVul;

  boardPtrEML[EML_RESULT] = &Board::SetResult;
  boardPtrEML[EML_SCORE] = &Board::SetScore;
  boardPtrEML[EML_SCORE_IMP] = &Board::SetScoreIMP;

  EMLdashes.insert(0, 12, ' ');
  EMLdashes.insert(13, 44, '-');

  EMLequals.insert(0, 80, '=');
}


bool tryEMLMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

bool readEMLCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

bool getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned openingLine);

bool getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk);

bool getEMLAuction(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk);

bool getEMLPlay(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk);

bool readEMLChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk);


bool readEMLCanvas(
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

    const string mid = line.substr(40, 10);
    if (mid == EMLdashes || mid == EMLequals)
      break;

    canvas.push_back(line);
  }
  return (canvas.size() >= 20);
}


bool getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned openingLine)
{
  openingLine = 5;
  string wd = "";
  while (openingLine < canvas.size())
  {
    if (ReadNextWord(canvas[openingLine], 42, wd) && wd == "Opening")
      break;
    openingLine++;
  }

  return (wd == "Opening");
}


bool getEMLSimpleFields(
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

  if (canvas[openingLine+2].at(canvas.size()-1) != ':')
  {
    if (! ReadLastWord(canvas[openingLine+2], chunk[EML_SCORE_IMP]))
      return false;
  }
  return true;
}


bool getEMLDeal(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! ReadNextWord(canvas[8], 6, sts)) return false;
  if (! ReadNextWord(canvas[9], 6, sth)) return false;
  if (! ReadNextWord(canvas[10], 6, std)) return false;
  if (! ReadNextWord(canvas[11], 6, stc)) return false;
  d << sts << "." << sth <<  "." << std << "." << stc << ":";

  if (! ReadNextWord(canvas[2], 18, sts)) return false;
  if (! ReadNextWord(canvas[3], 18, sth)) return false;
  if (! ReadNextWord(canvas[4], 18, std)) return false;
  if (! ReadNextWord(canvas[5], 18, stc)) return false;

  d << sts << "." << sth <<  "." << std << "." << stc << ":";
  if (! ReadNextWord(canvas[8], 29, sts)) return false;
  if (! ReadNextWord(canvas[9], 29, sth)) return false;
  if (! ReadNextWord(canvas[10], 29, std)) return false;
  if (! ReadNextWord(canvas[11], 29, stc)) return false;
  d << sts << "." << sth <<  "." << std << "." << stc << ":";

  chunk[EML_DEAL] = d.str();
  return true;
}


bool getEMLAuction(
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
      d << wd;
      no++;
    }
  }

  chunk[EML_AUCTION] = d.str();
  return true;
}


bool getEMLPlay(
  const vector<string>& canvas,
  const unsigned openingLine,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  string p1, p2, p3, p4;
  string opld;
  if (! ReadNextWord(canvas[openingLine], 56, opld))
    return false;

  string wd;
  unsigned pos0 = 45;
  unsigned la = canvas[openingLine+6].size();
  while (pos0 < la)
  {
    unsigned h;
    bool found = false;
    if (pos0 == la)
    {
      // Find the opening lead
      for (h = 0; h < 4; h++)
      {
        if (ReadNextWord(canvas[openingLine+h+7], 44, wd) && wd == opld)
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
        if (canvas[openingLine+h+7].at(pos0-2) == '-')
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
      unsigned l = openingLine+7 + ((h+p) % 4);
      if (canvas[l].at(pos0-1) == ' ')
      {
        // May be done
        if (canvas[l].at(pos0) == ' ')
          return true;

        d << canvas[l].at(pos0);
      }
      else
      {
        if (! ReadNextWord(canvas[l], pos0-1, wd))
          return false;
        d << wd;
        
      }
    }
    if (pos0 < la-1)
      d << ":";

    pos0 += 3;
  }
      
  chunk[EML_PLAY] = d.str();
}


bool readEMLChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readEMLCanvas(fstr, lno, canvas))
    return false;
  

  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < EML_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned openingLine;
  if (! getEMLCanvasOffset(canvas, openingLine))
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
  if (! getEMLPlay(canvas, openingLine, chunk))
    return false;

  return true;;
}


bool readEML(
  Group& group,
  const string& fname)
{
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
  while (readEMLChunk(fstr, lno, chunk))
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


bool tryEMLMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info)
{
  if (chunk[label] == "")
    return true;
  else if (label <= EML_ROOM)
  {
    if ((segment->*segPtrEML[label])(chunk[label], BRIDGE_FORMAT_EML))
      return true;
    else
    {
      LOG("Cannot add " + info + " line " + chunk[label] + "'");
      fstr.close();
      return false;
    }
  }
  else if ((board->*boardPtrEML[label])(chunk[label], BRIDGE_FORMAT_EML))
    return true;
  else
  {
    LOG("Cannot add " + info + " line " + chunk[label] + "'");
    fstr.close();
    return false;
  }
}


unsigned writeEMLCanvasHeight(
  Board * board)
{
  // unsigned eff = board->GetEffectiveAuctionLength();
  // playerType p = board->GetDealer();
  unsigned eff = 0, p = 0;
  UNUSED(board);

  return eff + ((p + 4 - BRIDGE_WEST) % 4);
}


void writeEMLCanvasFixed(
  vector<string>& canvas,
  const unsigned openingLine)
{
  canvas[0].replace(16, 5, "north");
  canvas[6].replace(4, 4, "west");
  canvas[6].replace(27, 4, "east");
  canvas[12].replace(16, 5, "south");

  canvas[2].replace(42, 4, "west");
  canvas[2].replace(51, 5, "north");
  canvas[2].replace(60, 4, "east");
  canvas[2].replace(69, 5, "south");

  canvas[openingLine].replace(42, 14, "Opening Lead:");
  canvas[openingLine+1].replace(42, 7, "Result:");
  canvas[openingLine+2].replace(42, 5, "Score:");

  canvas[openingLine+7].replace(42, 1, "W");
  canvas[openingLine+8].replace(42, 1, "N");
  canvas[openingLine+9].replace(42, 1, "E");
  canvas[openingLine+10].replace(42, 1, "S");
}


void writeEMLCanvasEasy(
  vector<string>& canvas,
  Segment * segment,
  Board * board)
{
  // scoring
  // dealer
  // vul
  // west .. south, in two places
  // Teams, Board x
  // result
  // score
  // possibly IMPs
  UNUSED(canvas);
  UNUSED(segment);
  UNUSED(board);
  assert(false);
}


void writeEMLCanvasDeal(
  vector<string>& canvas,
  Board * board)
{
  UNUSED(canvas);
  UNUSED(board);
  assert(false);
}


void writeEMLCanvasAuction(
  vector<string>& canvas,
  Board * board)
{
  UNUSED(canvas);
  UNUSED(board);
  assert(false);
}


void writeEMLCanvasPlay(
  vector<string>& canvas,
  Board * board)
{
  // need player leading to each trick
  // opening lead (first card played)
  // get a whole play structure out
  UNUSED(canvas);
  UNUSED(board);
  assert(false);
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

  Segment * segment = group.GetSegment(0);
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

      // Figure out actual number of lines in the canvas,
      // based on the length of the auction and the dealer.
      const unsigned clen = writeEMLCanvasHeight(board);
      vector<string> canvas(clen);
      for (unsigned j = 0; j < clen; j++)
        canvas[j].assign(EMLlineLength, ' ');

      board->CalculateScore();

      // Fill out the fixed fields directly.
      writeEMLCanvasFixed(canvas, clen-11);

      // Fill out the easy fields.
      writeEMLCanvasEasy(canvas, segment, board);

      writeEMLCanvasDeal(canvas, board);

      writeEMLCanvasAuction(canvas, board);

      writeEMLCanvasPlay(canvas, board);

      // Print the canvas, dropping trailing spaces.
      for (unsigned j = 0; j < clen; j++)
      {
        int p = EMLlineLength-1;
        while (p >= 0 && canvas[j].at(static_cast<unsigned>(p)) == ' ')
          p--;
        if (p < 0)
          fstr << "\n";
        else
          fstr << canvas[j].substr(0, static_cast<unsigned>(p+1)) << "\n";
      }

      if (i < numInstances-1)
        fstr << EMLdashes << "\n\n";
      else if (b < numBoards-1)
        fstr << EMLequals << "\n\n";
    }
  }

  fstr.close();
  return true;
}

