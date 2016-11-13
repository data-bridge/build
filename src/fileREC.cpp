/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <sstream>
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
  Buffer& buffer,
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
  Buffer& buffer,
  unsigned& lno,
  vector<string>& canvas)
{
  string line;
  LineData lineData;
  while (buffer.next(lineData))
  {
    lno++;
    if (lineData.type == BRIDGE_BUFFER_EMPTY)
    {
      int i = buffer.peek();
      if (i == EOF || i == 0x49) // I
        return;
      else
        continue;
    }
    else if (lineData.type == BRIDGE_BUFFER_COMMENT)
      continue;

    canvas.push_back(lineData.line);
  }
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
    if (! readNextWord(line, st, wd))
      continue;

    unsigned u;
    if (str2upos(wd, u))
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

  if (! readNextWord(canvas[0], 0, chunk[BRIDGE_FORMAT_SCORING])) 
    THROW("Couldn't read format: '" + chunk[BRIDGE_FORMAT_SCORING] + "'");

  if (! readNextWord(canvas[0], 29, chunk[BRIDGE_FORMAT_DEALER])) 
    THROW("Couldn't read dealer: '" + chunk[BRIDGE_FORMAT_DEALER] + "'");

  if (! readNextWord(canvas[1], 6, chunk[BRIDGE_FORMAT_BOARD_NO])) 
    THROW("Couldn't read board: '" + chunk[BRIDGE_FORMAT_BOARD_NO] + "'");

  if (! readNextWord(canvas[1], 29, chunk[BRIDGE_FORMAT_VULNERABLE])) 
    THROW("Couldn't read vul: '" + chunk[BRIDGE_FORMAT_VULNERABLE] + "'");

  if (! readAllWords(canvas[3], 0, 11, chunk[BRIDGE_FORMAT_WEST])) 
    THROW("Couldn't read West: '" + chunk[BRIDGE_FORMAT_WEST] + "'");
  // Let's hope West doesn't sit West :-)
  if (chunk[BRIDGE_FORMAT_WEST] == "West")
    chunk[BRIDGE_FORMAT_WEST] = "";

  if (! readAllWords(canvas[0], 12, 23, chunk[BRIDGE_FORMAT_NORTH])) 
    THROW("Couldn't read North: '" + chunk[BRIDGE_FORMAT_NORTH] + "'");
  if (chunk[BRIDGE_FORMAT_NORTH] == "North")
    chunk[BRIDGE_FORMAT_NORTH] = "";

  if (! readAllWords(canvas[3], 24, 35, chunk[BRIDGE_FORMAT_EAST])) 
    THROW("Couldn't read East: '" + chunk[BRIDGE_FORMAT_EAST] + "'");
  if (chunk[BRIDGE_FORMAT_EAST] == "East")
    chunk[BRIDGE_FORMAT_EAST] = "";

  if (! readAllWords(canvas[6], 12, 23, chunk[BRIDGE_FORMAT_SOUTH])) 
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

  if (! readNextWord(canvas[4], 2, 11, sts)) sts = "";
  if (! readNextWord(canvas[5], 2, 11, sth)) sth = "";
  if (! readNextWord(canvas[6], 2, 11, std)) std = "";
  if (! readNextWord(canvas[7], 2, 11, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextWord(canvas[1], 14, 23, sts)) sts = "";
  if (! readNextWord(canvas[2], 14, 23, sth)) sth = "";
  if (! readNextWord(canvas[3], 14, 23, std)) std = "";
  if (! readNextWord(canvas[4], 14, 23, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextWord(canvas[4], 26, sts)) sts = "";
  if (! readNextWord(canvas[5], 26, sth)) sth = "";
  if (! readNextWord(canvas[6], 26, std)) std = "";
  if (! readNextWord(canvas[7], 26, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextWord(canvas[7], 14, 23, sts)) sts = "";
  if (! readNextWord(canvas[8], 14, 23, sth)) sth = "";
  if (! readNextWord(canvas[9], 14, 23, std)) std = "";
  if (! readNextWord(canvas[10], 14, 23, stc)) stc = "";
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
      if (! readNextWord(canvas[l], beg, wd) || wd == "Opening")
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
  chunk[BRIDGE_FORMAT_PLAY] = regex_replace(d.str(), re, string(""));
}


void readRECChunk(
  Buffer& buffer,
  unsigned& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  readRECCanvas(buffer, lno, canvas);
  
  // Then parse them into the chunk structure.
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    chunk[i] = "";

  if (canvas.size() == 0)
    return;

  unsigned playLine = 0;
  newSegFlag = false;

  bool playExists = getRECCanvasOffset(canvas, playLine);
  getRECFields(canvas, playLine, playExists, chunk);
}



void writeRECBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  Canvas canvas;

  board.calculateScore();

  const string dstr = board.strDeal(BRIDGE_WEST, format);
  const string sstr = segment.strScoring(format);
  const string estr = board.strDealer(format);
  const string bstr = segment.strNumber(writeInfo.bno, format);
  const string vstr = board.strVul(format);

  const string west = board.strPlayer(BRIDGE_WEST, format);
  const string north = board.strPlayer(BRIDGE_NORTH, format);
  const string east = board.strPlayer(BRIDGE_EAST, format);
  const string south = board.strPlayer(BRIDGE_SOUTH, format);

  // Convert deal, auction and play from \n to vectors.
  vector<string> deal;
  str2lines(dstr, deal);

  canvas.resize(11, 80);
  canvas.setRectangle(deal, 0, 0);

  canvas.setLine(sstr, 0, 0);
  canvas.setLine(estr, 0, 24);
  canvas.setLine(bstr, 1, 0);
  canvas.setLine(vstr, 1, 24);

  canvas.setLine(north, 0, 12);
  canvas.setLine(west, 3, 0);
  canvas.setLine(east, 3, 24);
  canvas.setLine(south, 6, 12);

  st += canvas.str(true) + "\n";

  st += board.strPlayers(format) + "\n";
  st += board.strAuction(format);

  st += board.strLead(format) + "    ";
  st += board.strResult(format, false) + "\n";
  st += board.strScore(format, false);
  st += board.strScoreIMP(format, writeInfo.ino == 1) + "\n\n";

  st += board.strPlay(format);
}

