/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <regex>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "Canvas.h"
#include "fileREC.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


static bool tryRECMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

static void readRECCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getRECCanvasOffset(
  const vector<string>& canvas,
  unsigned& playLine);

static void getRECFields(
  const vector<string>& canvas,
  const unsigned playLine,
  const bool playExists,
  vector<string>& chunk);

static void getRECDeal(
  const vector<string>& canvas,
  vector<string>& chunk);

static void getRECAuction(
  const vector<string>& canvas,
  vector<string>& chunk);

static void getRECPlay(
  const vector<string>& canvas,
  const unsigned& offset,
  vector<string>& chunk);


static void readRECCanvas(
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
      // if (prevLine.size() >= 8 && prevLine.at(1) != ' ' && 
          // prevLine.at(2) == ' ' && prevLine.at(3) == ' ' &&
          // prevLine.at(4) != ' ')
      int i = fstr.peek();
      if (i == EOF || i == 0x49) // I
        return;
      else
        continue;
    }
    else if (line.at(0) == '%')
      continue;

    canvas.push_back(line);
    prevLine = line;
  }

  if (canvas.size() == 0)
    THROW("Didn't read any canvas");
}


static bool getRECCanvasOffset(
  const vector<string>& canvas,
  unsigned& playLine)
{
  string wd;
  for (playLine = 12; playLine < canvas.size(); playLine++)
  {
    const string& line = canvas[playLine];
    if (line.size() < 8 || line.at(3) != ' ' || line.at(4) == ' ')
      continue;

    const unsigned st = (line.at(0) == ' ' ? 1u : 0u);
    if (! ReadNextWord(line, st, wd))
      continue;

    unsigned u;
    if (StringToNonzeroUnsigned(wd, u))
      break;
  }

  return (playLine != canvas.size());
}


static void getRECFields(
  const vector<string>& canvas,
  const unsigned playLine,
  const bool playExists,
  vector<string>& chunk)
{
  if (canvas[0].size() < 30 || 
      canvas[1].size() < 30 ||
      canvas[3].size() < 26 ||
      canvas[6].size() < 18)
  {
    THROW("Some deal lines are too short");
  }

  if (! ReadNextWord(canvas[0], 0, chunk[BRIDGE_FORMAT_SCORING])) 
    THROW("Couldn't read format: '" + chunk[BRIDGE_FORMAT_SCORING] + "'");

  if (! ReadNextWord(canvas[0], 29, chunk[BRIDGE_FORMAT_DEALER])) 
    THROW("Couldn't read dealer: '" + chunk[BRIDGE_FORMAT_DEALER] + "'");

  if (! ReadNextWord(canvas[1], 6, chunk[BRIDGE_FORMAT_BOARD_NO])) 
    THROW("Couldn't read board: '" + chunk[BRIDGE_FORMAT_BOARD_NO] + "'");

  if (! ReadNextWord(canvas[1], 29, chunk[BRIDGE_FORMAT_VULNERABLE])) 
    THROW("Couldn't read vul: '" + chunk[BRIDGE_FORMAT_VULNERABLE] + "'");

  if (! ReadAllWords(canvas[3], 0, 11, chunk[BRIDGE_FORMAT_WEST])) 
    THROW("Couldn't read West: '" + chunk[BRIDGE_FORMAT_WEST] + "'");
  // Let's hope West doesn't sit West :-)
  if (chunk[BRIDGE_FORMAT_WEST] == "West")
    chunk[BRIDGE_FORMAT_WEST] = "";

  if (! ReadAllWords(canvas[0], 12, 23, chunk[BRIDGE_FORMAT_NORTH])) 
    THROW("Couldn't read North: '" + chunk[BRIDGE_FORMAT_NORTH] + "'");
  if (chunk[BRIDGE_FORMAT_NORTH] == "North")
    chunk[BRIDGE_FORMAT_NORTH] = "";

  if (! ReadAllWords(canvas[3], 24, 35, chunk[BRIDGE_FORMAT_EAST])) 
    THROW("Couldn't read East: '" + chunk[BRIDGE_FORMAT_EAST] + "'");
  if (chunk[BRIDGE_FORMAT_EAST] == "East")
    chunk[BRIDGE_FORMAT_EAST] = "";

  if (! ReadAllWords(canvas[6], 12, 23, chunk[BRIDGE_FORMAT_SOUTH])) 
    THROW("Couldn't read South: '" + chunk[BRIDGE_FORMAT_SOUTH] + "'");
  if (chunk[BRIDGE_FORMAT_SOUTH] == "South")
    chunk[BRIDGE_FORMAT_SOUTH] = "";

  getRECDeal(canvas, chunk);
  getRECAuction(canvas, chunk);

  if (playExists)
  {
    getRECPlay(canvas, playLine, chunk);

    if (canvas[playLine-2].size() < 30)
      THROW("The score line is too short");

    chunk[BRIDGE_FORMAT_RESULT] = canvas[playLine-2].substr(28);
  }
  else
  {
    chunk[BRIDGE_FORMAT_RESULT] = canvas[canvas.size()-2].substr(28);

    // Pavlicek bug.
    if (chunk[BRIDGE_FORMAT_RESULT] == "Won 32")
      chunk[BRIDGE_FORMAT_RESULT] = "";
  }

}


static void getRECDeal(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! ReadNextWord(canvas[4], 2, 11, sts)) sts = "";
  if (! ReadNextWord(canvas[5], 2, 11, sth)) sth = "";
  if (! ReadNextWord(canvas[6], 2, 11, std)) std = "";
  if (! ReadNextWord(canvas[7], 2, 11, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextWord(canvas[1], 14, 23, sts)) sts = "";
  if (! ReadNextWord(canvas[2], 14, 23, sth)) sth = "";
  if (! ReadNextWord(canvas[3], 14, 23, std)) std = "";
  if (! ReadNextWord(canvas[4], 14, 23, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextWord(canvas[4], 26, sts)) sts = "";
  if (! ReadNextWord(canvas[5], 26, sth)) sth = "";
  if (! ReadNextWord(canvas[6], 26, std)) std = "";
  if (! ReadNextWord(canvas[7], 26, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! ReadNextWord(canvas[7], 14, 23, sts)) sts = "";
  if (! ReadNextWord(canvas[8], 14, 23, sth)) sth = "";
  if (! ReadNextWord(canvas[9], 14, 23, std)) std = "";
  if (! ReadNextWord(canvas[10], 14, 23, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;

  // Void is shown as empty in REC.
  chunk[BRIDGE_FORMAT_DEAL] = d.str();
}


static void getRECAuction(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  if (canvas[13].length() > 7 && canvas[13].substr(0, 7) == "Opening")
  {
    chunk[BRIDGE_FORMAT_AUCTION] = "";
    return;
  }
  
  stringstream d;
  d.clear();

  const unsigned offset = 13;
  unsigned firstStart = 0;
  while (firstStart < 32 && canvas[offset].at(firstStart) == ' ')
    firstStart += 9;

  if (firstStart >= 32)
    THROW("Auction doesn't start where expected: '" + canvas[offset] + "'");

  string wd;
  unsigned no = 0;
  bool done = false;
  unsigned l;
  unsigned numPasses = 0;
  for (l = offset; l < canvas.size(); l++)
  {
    for (unsigned beg = (l == offset ? firstStart : 0); beg < 32; beg += 9)
    {
      if (! ReadNextWord(canvas[l], beg, wd) || wd == "Opening")
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

      if (numPasses >= 3 && no >= 4)
      {
        done = true;
        break;
      }
    }
    if (done)
      break;
  }

  if (l == canvas.size())
    THROW("Ran out of canvas");

  chunk[BRIDGE_FORMAT_AUCTION] = d.str();
}


static void getRECPlay(
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
      THROW("Too many commas in play line: " + line + "'");

    vector<string> words(4);
    words.clear();
    tokenize(line, words, ",");

    if (words[0].size() != 13)
      THROW("Odd length of first play in line: '" + line + "'");

    if (l != offset)
      d << ":";

    d << words[0].substr(11);
    for (unsigned i = 1; i < words.size(); i++)
      d << words[i];
  }

  regex re(" ");
  chunk[BRIDGE_FORMAT_PLAY] = regex_replace(d.str(), re, "");
}


bool readRECChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  readRECCanvas(fstr, lno, canvas);
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned playLine = 0;
  newSegFlag = false;

  bool playExists = getRECCanvasOffset(canvas, playLine);
  getRECFields(canvas, playLine, playExists, chunk);
  return true;
}


void writeRECBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const formatType f)
{
  Canvas canvas;

  board->CalculateScore();

  const string dstr = board->DealAsString(BRIDGE_WEST, f);
  const string sstr = segment->ScoringAsString(f);
  const string estr = board->DealerAsString(f);
  const string bstr = segment->NumberAsString(f, writeInfo.bno);
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

  fstr << canvas.AsString(true) << "\n";

  fstr << board->PlayersAsString(f) << "\n";
  fstr << board->AuctionAsString(f);

  fstr << board->LeadAsString(f) << "    ";
  fstr << board->ResultAsString(f, false) << "\n";
  fstr << board->ScoreAsString(f, false);
  fstr << board->ScoreIMPAsString(f, writeInfo.ino == 1) << "\n\n";

  fstr << board->PlayAsString(f);
}

