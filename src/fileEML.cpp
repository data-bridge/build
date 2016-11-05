/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>
#include <fstream>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "Canvas.h"
#include "fileEML.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


static string EMLdashes, EMLequals;
static string EMLshortDashes, EMLshortEquals;


static void readEMLCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getEMLCanvasWest(
  const vector<string>& canvas,
  const unsigned pos,
  const unsigned resultLine,
  unsigned& westLine,
  unsigned& cardStart);

static void getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned& resultLine,
  bool& playIsPresent,
  unsigned& westLine);

static void getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk);

static void getEMLAuction(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk);

static void getEMLPlay(
  const vector<string>& canvas,
  const bool playIsPresent,
  const unsigned resultLine,
  const unsigned westLine,
  const unsigned cardStart,
  vector<string>& chunk);


void setEMLTables()
{
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


static void readEMLCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas)
{
  string line;
  while (getline(fstr, line))
  {
    lno++;
    if (line.empty())
    {
      // If some players aren't given, we might have an empty line.
      if (canvas.size() != 7 && canvas.size() != 13)
        continue;
    }
    else if (line.at(0) == '%')
      continue;

    if (line.size() > 40)
    {
      const string mid = line.substr(30, 10);
      if (mid == EMLshortDashes || mid == EMLshortEquals)
        break;
    }

    canvas.push_back(line);
  }

  if (canvas.size() < 17)
    THROW("Canvas too short");
}


static bool getEMLCanvasWest(
  const vector<string>& canvas,
  const unsigned pos,
  const unsigned resultLine,
  unsigned& westLine,
  unsigned& cardStart)
{
  westLine = resultLine;
  cardStart = 0;
  string wd = "";
  while (westLine < canvas.size())
  {
    if (canvas[westLine].size() >= pos+1 &&
        readNextWord(canvas[westLine], pos, wd) && 
        wd == "W")
    {
      cardStart = pos+3;
      break;
    }
    westLine++;
  }

  return (cardStart > 0);
}


static void getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned& resultLine,
  unsigned& westLine,
  bool& playIsPresent,
  unsigned& cardStart)
{
  resultLine = 5;
  string wd = "";
  while (resultLine < canvas.size())
  {
    if (readNextWord(canvas[resultLine], 42, wd) && wd == "Result:")
      break;
    resultLine++;
  }

  if (wd != "Result:")
    THROW("Cannot find result");

  if (getEMLCanvasWest(canvas, 42, resultLine, westLine, cardStart))
    playIsPresent = true;
  else if (getEMLCanvasWest(canvas, 39, resultLine, westLine, cardStart))
    playIsPresent = true;
  else
    playIsPresent = false;
}


static void getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk)
{
  if (! readNextWord(canvas[0], 0, chunk[BRIDGE_FORMAT_SCORING]))
    THROW("Cannot find scoring");

  if (! readAllWords(canvas[1], 16, 23, chunk[BRIDGE_FORMAT_NORTH]))
    chunk[BRIDGE_FORMAT_NORTH] = "";
  if (! readAllWords(canvas[7], 4, 11, chunk[BRIDGE_FORMAT_WEST]))
    chunk[BRIDGE_FORMAT_WEST] = "";
  if (! readAllWords(canvas[7], 27, 34, chunk[BRIDGE_FORMAT_EAST]))
    chunk[BRIDGE_FORMAT_EAST] = "";
  if (! readAllWords(canvas[13], 16, 23, chunk[BRIDGE_FORMAT_SOUTH]))
    chunk[BRIDGE_FORMAT_SOUTH] = "";

  if (! readNextWord(canvas[0], 54, chunk[BRIDGE_FORMAT_BOARD_NO]))
    THROW("Cannot find board number");
  if (! readNextWord(canvas[1], 5, chunk[BRIDGE_FORMAT_DEALER]))
    THROW("Cannot find dealer");
  if (! readNextWord(canvas[2], 5, chunk[BRIDGE_FORMAT_VULNERABLE]))
    THROW("Cannot find vulnerability");

  if (! readNextWord(canvas[resultLine], 50, chunk[BRIDGE_FORMAT_RESULT]))
    THROW("Cannot find result");

  if (! readNextWord(canvas[resultLine+1], 49, chunk[BRIDGE_FORMAT_SCORE]))
    THROW("Cannot find score");
  chunk[BRIDGE_FORMAT_SCORE].pop_back(); // Drop trailing comma

  if (canvas[resultLine+1].back() != ':')
  {
    if (! readLastWord(canvas[resultLine+1], chunk[BRIDGE_FORMAT_SCORE_IMP]))
      THROW("Cannot find IMP result");
  }
}


static void getEMLDeal(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! readNextWord(canvas[8], 6, sts)) sts = "";
  if (! readNextWord(canvas[9], 6, sth)) sth = "";
  if (! readNextWord(canvas[10], 6, std)) std = "";
  if (! readNextWord(canvas[11], 6, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextWord(canvas[2], 18, sts)) sts = "";
  if (! readNextWord(canvas[3], 18, sth)) sth = "";
  if (! readNextWord(canvas[4], 18, std)) std = "";
  if (! readNextWord(canvas[5], 18, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextWord(canvas[8], 29, sts)) sts = "";
  if (! readNextWord(canvas[9], 29, sth)) sth = "";
  if (! readNextWord(canvas[10], 29, std)) std = "";
  if (! readNextWord(canvas[11], 29, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextWord(canvas[14], 18, sts)) sts = "";
  if (! readNextWord(canvas[15], 18, sth)) sth = "";
  if (! readNextWord(canvas[16], 18, std)) std = "";
  if (! readNextWord(canvas[17], 18, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;

  chunk[BRIDGE_FORMAT_DEAL] = d.str();
}


static void getEMLAuction(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  unsigned firstStart = 42;
  if (canvas[5].size() < firstStart)
    return;

  while (firstStart < 75 && canvas[5].at(firstStart) == ' ')
    firstStart += 9;

  if (firstStart >= 75)
    THROW("Cannot locate auction");

  string wd;
  unsigned no = 0;
  for (unsigned l = 5; l < resultLine-1; l++)
  {
    for (unsigned beg = (l == 5 ? firstStart : 42); beg < 75; beg += 9)
    {
      if (! readNextWord(canvas[l], beg, wd))
      {
        if (l == resultLine-2 || l == resultLine-3)
          break;
        else
          THROW("Early end of auction");
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
      else if (wd == "(all" || wd == "Passed")
      {
        d << "A";
        break;
      }
      else
        d << wd;
      no++;
    }
  }

  chunk[BRIDGE_FORMAT_AUCTION] = d.str();
}


static void getEMLPlay(
  const vector<string>& canvas,
  const bool playIsPresent,
  const unsigned resultLine,
  const unsigned westLine,
  const unsigned cardStart,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  string p1, p2, p3, p4;

  string opld;
  if (! readNextWord(canvas[resultLine-1], 56, opld))
  {
    if (playIsPresent)
      THROW("Cannot locate play");
    else
      return;
  }

  if (! playIsPresent)
  {
    chunk[BRIDGE_FORMAT_PLAY] = opld;
    return;
  }

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
            readNextWord(canvas[westLine+h], cardStart-1, cardStart, wd) &&
            wd == opld)
        {
          found = true;
          break;
        }
      }

      if (! found)
        THROW("Cannot locate opening lead");
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
        THROW("Cannot find the dash for the opening lead");
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
        if (! readNextWord(canvas[l], pos0-1, pos0, wd))
          THROW("Cannot find next play");
        d << wd;
        
      }
    }
    if (pos0 < la-1)
      d << ":";

    pos0 += 3;
  }
      
  chunk[BRIDGE_FORMAT_PLAY] = d.str();
}


void readEMLChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  newSegFlag = false;

  // First get all the lines of a hand.
  vector<string> canvas;
  readEMLCanvas(fstr, lno, canvas);
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned openingLine = 0;
  unsigned westLine = 0;
  unsigned cardStart = 0;
  bool playIsPresent = false;
  getEMLCanvasOffset(canvas, openingLine, 
      westLine, playIsPresent, cardStart);

  getEMLSimpleFields(canvas, openingLine, chunk);

  // Synthesize an RBN-style deal.
  getEMLDeal(canvas, chunk);

  // Synthesize an RBN-style string of bids.
  getEMLAuction(canvas, openingLine, chunk);

  // Synthesize an RBN-style string of plays.
  getEMLPlay(canvas, playIsPresent, openingLine, 
      westLine, cardStart, chunk);
}


void writeEMLBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  Canvas canvas;

  board.calculateScore();

  // Convert deal, auction and play from \n to vectors.
  vector<string> deal, auction, play;
  str2lines(board.strDeal(BRIDGE_WEST, format), deal);
  str2lines(board.strAuction(format), auction);
  str2lines(board.strPlay(format), play);

  // Height of auction determines dimensions.
  // It seems we leave out the play if that makes the canvas too large.
  const unsigned as = auction.size();
  const unsigned a = Max(4, as);
  const unsigned alstart = (a <= 6 ? 14 : a+8);
  bool const playFlag = (play[0].size() > 2 && alstart < 19 ? true : false);
  const unsigned clen = (playFlag ? alstart + 5 : Max(18, as+6));
  const unsigned acstart = (play[0].length() > 38 ? 39u : 42u);

  canvas.resize(clen, 80);

  canvas.setRectangle(deal, 0, 4);
  canvas.setRectangle(auction, 2, 42);
  if (playFlag)
    canvas.setRectangle(play, alstart, acstart);

  canvas.setLine(segment.strScoring(format), 0, 0);
  canvas.setLine(board.strPlayer(BRIDGE_WEST, format), 7, 4);
  canvas.setLine(board.strPlayer(BRIDGE_NORTH, format), 1, 16);
  canvas.setLine(board.strPlayer(BRIDGE_EAST, format), 7, 27);
  canvas.setLine(board.strPlayer(BRIDGE_SOUTH, format), 13, 16);
  canvas.setLine(board.strPlayer(BRIDGE_WEST, format), 3, 42);
  canvas.setLine(board.strPlayer(BRIDGE_NORTH, format), 3, 51);
  canvas.setLine(board.strPlayer(BRIDGE_EAST, format), 3, 60);
  canvas.setLine(board.strPlayer(BRIDGE_SOUTH, format), 3, 69);
  canvas.setLine(segment.strNumber(writeInfo.bno, format), 0, 42);

  canvas.setLine(board.strDealer(format), 1, 0);
  canvas.setLine(board.strVul(format), 2, 0);
  const string l = board.strLead(format);
  unsigned p;
  if (l == "Opening Lead:")
    p = a+3;
  else
  {
    canvas.setLine(l, a+3, 42);
    p = a+4;
  }
  canvas.setLine(board.strResult(format, false), p, 42);
  canvas.setLine(board.strScore(format, segment.scoringIsIMPs()), p+1, 42);

  st += canvas.str() + "\n";

  if (writeInfo.ino < writeInfo.numInst-1)
    st += EMLdashes + "\n\n";
  else if (writeInfo.bno < writeInfo.numBoards-1)
    st += EMLequals + "\n\n";
}

