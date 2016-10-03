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
#include "Debug.h"

using namespace std;

extern Debug debug;


static bool tryRECMethod(
  const vector<string>& chunk,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr,
  const string& info);

static bool readRECCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getRECCanvasOffset(
  const vector<string>& canvas,
  unsigned& playLine);

static bool getRECFields(
  const vector<string>& canvas,
  const unsigned auctionLine,
  vector<string>& chunk);

static bool getRECDeal(
  const vector<string>& canvas,
  vector<string>& chunk);

static bool getRECAuction(
  const vector<string>& canvas,
  vector<string>& chunk);

static bool getRECPlay(
  const vector<string>& canvas,
  const unsigned& offset,
  vector<string>& chunk);


static bool readRECCanvas(
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
          prevLine.at(2) == ' ' && prevLine.at(3) == ' ' &&
          prevLine.at(4) != ' ')
        return true;
      else
        continue;
    }
    else if (line.at(0) == '%')
      continue;

    canvas.push_back(line);
    prevLine = line;
  }
  return (canvas.size() > 0);
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


static bool getRECFields(
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

  if (! ReadNextWord(canvas[0], 0, chunk[BRIDGE_FORMAT_SCORING])) return false;
  if (! ReadNextWord(canvas[0], 29, chunk[BRIDGE_FORMAT_DEALER])) return false;
  if (! ReadNextWord(canvas[1], 6, chunk[BRIDGE_FORMAT_BOARD_NO])) return false;
  if (! ReadNextWord(canvas[1], 29, chunk[BRIDGE_FORMAT_VULNERABLE])) return false;

  if (! ReadNextWord(canvas[3], 0, chunk[BRIDGE_FORMAT_WEST])) return false;
  if (! ReadNextWord(canvas[0], 12, chunk[BRIDGE_FORMAT_NORTH])) return false;
  if (! ReadNextWord(canvas[3], 24, chunk[BRIDGE_FORMAT_EAST])) return false;
  if (! ReadNextWord(canvas[6], 12, chunk[BRIDGE_FORMAT_SOUTH])) return false;

  if (! getRECDeal(canvas, chunk)) return false;
  if (! getRECAuction(canvas, chunk)) return false;
  if (! getRECPlay(canvas, pline, chunk)) return false;

  chunk[BRIDGE_FORMAT_RESULT] = canvas[pline-2].substr(28);

  return true;
}


static bool getRECDeal(
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
  chunk[BRIDGE_FORMAT_DEAL] = d.str();
  return true;
}


static bool getRECAuction(
  const vector<string>& canvas,
  vector<string>& chunk)
{
  stringstream d;
  d.clear();

  const unsigned offset = 13;
  unsigned firstStart = 0;
  while (firstStart < 32 && canvas[offset].at(firstStart) == ' ')
    firstStart += 9;

  if (firstStart >= 32)
    return false;

  string wd;
  unsigned no = 0;
  bool done = false;
  unsigned l;
  unsigned numPasses = 0;
  for (l = offset; l < canvas.size(); l++)
  {
    for (unsigned beg = (l == offset ? firstStart : 0); beg < 32; beg += 9)
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
  return true;
}


static bool getRECPlay(
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

  regex re(" ");
  chunk[BRIDGE_FORMAT_PLAY] = regex_replace(d.str(), re, "");
  return true;
}


bool readRECChunk(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  if (! readRECCanvas(fstr, lno, canvas))
    return false;
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  unsigned playLine = 0;
  if (! getRECCanvasOffset(canvas, playLine))
    return false;

  newSegFlag = false;
  return getRECFields(canvas, playLine, chunk);
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
  fstr << board->AuctionAsString(f) << "\n";

  fstr << board->LeadAsString(f) << "    ";
  fstr << board->ResultAsString(f, false) << "\n";
  fstr << board->ScoreAsString(f, false);
  fstr << board->ScoreIMPAsString(f, writeInfo.ino == 1) << "\n\n";

  fstr << board->PlayAsString(f);
}

