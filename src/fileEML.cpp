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

using namespace std;


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

string EMLdashes, EMLequals;
string EMLshortDashes, EMLshortEquals;


static bool readEMLCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getEMLCanvasWest(
  const vector<string>& canvas,
  const unsigned pos,
  const unsigned resultLine,
  unsigned& westLine,
  unsigned& cardStart);

static bool getEMLCanvasOffset(
  const vector<string>& canvas,
  unsigned& resultLine,
  bool& playIsPresent,
  unsigned& westLine);

static bool getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk);

static bool getEMLAuction(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk);

static bool getEMLPlay(
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


static bool readEMLCanvas(
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
  return (canvas.size() >= 17);
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
  unsigned& resultLine,
  unsigned& westLine,
  bool& playIsPresent,
  unsigned& cardStart)
{
  resultLine = 5;
  string wd = "";
  while (resultLine < canvas.size())
  {
    if (ReadNextWord(canvas[resultLine], 42, wd) && wd == "Result:")
      break;
    resultLine++;
  }

  if (wd != "Result:")
    return false;

  if (getEMLCanvasWest(canvas, 42, resultLine, westLine, cardStart))
    playIsPresent = true;
  else if (getEMLCanvasWest(canvas, 39, resultLine, westLine, cardStart))
    playIsPresent = true;
  else
    playIsPresent = false;
  return true;
}


static bool getEMLSimpleFields(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk)
{
  if (! ReadNextWord(canvas[0], 0, chunk[BRIDGE_FORMAT_SCORING]))
    return false;

  if (! ReadAllWords(canvas[1], 16, 23, chunk[BRIDGE_FORMAT_NORTH]))
    chunk[BRIDGE_FORMAT_NORTH] = "";
  if (! ReadAllWords(canvas[7], 4, 11, chunk[BRIDGE_FORMAT_WEST]))
    chunk[BRIDGE_FORMAT_WEST] = "";
  if (! ReadAllWords(canvas[7], 27, 34, chunk[BRIDGE_FORMAT_EAST]))
    chunk[BRIDGE_FORMAT_EAST] = "";
  if (! ReadAllWords(canvas[13], 16, 23, chunk[BRIDGE_FORMAT_SOUTH]))
    chunk[BRIDGE_FORMAT_SOUTH] = "";

  if (! ReadNextWord(canvas[0], 54, chunk[BRIDGE_FORMAT_BOARD_NO]))
    return false;
  if (! ReadNextWord(canvas[1], 5, chunk[BRIDGE_FORMAT_DEALER]))
    return false;
  if (! ReadNextWord(canvas[2], 5, chunk[BRIDGE_FORMAT_VULNERABLE]))
    return false;

  if (! ReadNextWord(canvas[resultLine], 50, chunk[BRIDGE_FORMAT_RESULT]))
    return false;

  if (! ReadNextWord(canvas[resultLine+1], 49, chunk[BRIDGE_FORMAT_SCORE]))
    return false;
  chunk[BRIDGE_FORMAT_SCORE].pop_back(); // Drop trailing comma

  if (canvas[resultLine+1].back() != ':')
  {
    if (! ReadLastWord(canvas[resultLine+1], chunk[BRIDGE_FORMAT_SCORE_IMP]))
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


  chunk[BRIDGE_FORMAT_DEAL] = d.str();
  return true;
}


static bool getEMLAuction(
  const vector<string>& canvas,
  const unsigned resultLine,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  unsigned firstStart = 42;
  if (canvas[5].size() < firstStart)
    return true;

  while (firstStart < 75 && canvas[5].at(firstStart) == ' ')
    firstStart += 9;

  if (firstStart >= 75)
    return false;

  string wd;
  unsigned no = 0;
  for (unsigned l = 5; l < resultLine-1; l++)
  {
    for (unsigned beg = (l == 5 ? firstStart : 42); beg < 75; beg += 9)
    {
      if (! ReadNextWord(canvas[l], beg, wd))
      {
        if (l == resultLine-2 || l == resultLine-3)
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
  return true;
}


static bool getEMLPlay(
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
  if (! ReadNextWord(canvas[resultLine-1], 56, opld))
  {
    if (playIsPresent)
      return false;
    else
      return true;
  }

  if (! playIsPresent)
  {
    chunk[BRIDGE_FORMAT_PLAY] = opld;
    return true;
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
      
  chunk[BRIDGE_FORMAT_PLAY] = d.str();
  return true;
}


bool readEMLChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  newSegFlag = false;

  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readEMLCanvas(fstr, lno, canvas))
    return false;
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned openingLine = 0;
  unsigned westLine = 0;
  unsigned cardStart = 0;
  bool playIsPresent = false;
  if (! getEMLCanvasOffset(canvas, openingLine, 
      westLine, playIsPresent, cardStart))
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
  if (! getEMLPlay(canvas, playIsPresent, openingLine, 
      westLine, cardStart, chunk))
    return false;

  return true;
}


void writeEMLBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const Format format)
{
  string chunk[EML_LABELS_SIZE];
  Canvas canvas;

  board->calculateScore();

  chunk[EML_SCORING] = segment->ScoringAsString(format);
  chunk[EML_BOARD] = segment->NumberAsString(format, writeInfo.bno);

  chunk[EML_WEST] = board->strPlayer(BRIDGE_WEST, format);
  chunk[EML_NORTH] = board->strPlayer(BRIDGE_NORTH, format);
  chunk[EML_EAST] = board->strPlayer(BRIDGE_EAST, format);
  chunk[EML_SOUTH] = board->strPlayer(BRIDGE_SOUTH, format);

  chunk[EML_DEAL] = board->strDeal(BRIDGE_WEST, format);
  chunk[EML_DEALER] = board->strDealer(format);
  chunk[EML_VULNERABLE] = board->strVul(format);
  chunk[EML_AUCTION] = board->strAuction(format);
  chunk[EML_LEAD] = board->LeadAsString(format);
  chunk[EML_PLAY] = board->PlayAsString(format);
  chunk[EML_RESULT] = board->ResultAsString(format, false);
  chunk[EML_SCORE] = board->ScoreAsString(format, segment->ScoringIsIMPs());

  // Convert deal, auction and play from \n to vectors.
  vector<string> deal, auction, play;
  ConvertMultilineToVector(chunk[EML_DEAL], deal);
  ConvertMultilineToVector(chunk[EML_AUCTION], auction);
  ConvertMultilineToVector(chunk[EML_PLAY], play);

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

  canvas.setLine(chunk[EML_SCORING], 0, 0);
  canvas.setLine(chunk[EML_WEST], 7, 4);
  canvas.setLine(chunk[EML_NORTH], 1, 16);
  canvas.setLine(chunk[EML_EAST], 7, 27);
  canvas.setLine(chunk[EML_SOUTH], 13, 16);
  canvas.setLine(chunk[EML_WEST], 3, 42);
  canvas.setLine(chunk[EML_NORTH], 3, 51);
  canvas.setLine(chunk[EML_EAST], 3, 60);
  canvas.setLine(chunk[EML_SOUTH], 3, 69);
  canvas.setLine(chunk[EML_BOARD], 0, 42);

  canvas.setLine(chunk[EML_DEALER], 1, 0);
  canvas.setLine(chunk[EML_VULNERABLE], 2, 0);
  if (chunk[EML_LEAD] == "Opening Lead:")
  {
    canvas.setLine(chunk[EML_RESULT], a+3, 42);
    canvas.setLine(chunk[EML_SCORE], a+4, 42);
  }
  else
  {
    canvas.setLine(chunk[EML_LEAD], a+3, 42);
    canvas.setLine(chunk[EML_RESULT], a+4, 42);
    canvas.setLine(chunk[EML_SCORE], a+5, 42);
  }

  fstr << canvas.str() << "\n";

  if (writeInfo.ino < writeInfo.numInst-1)
    fstr << EMLdashes << "\n\n";
  else if (writeInfo.bno < writeInfo.numBoards-1)
    fstr << EMLequals << "\n\n";
}

