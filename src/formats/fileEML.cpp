/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <sstream>
#pragma warning(pop)

#include "fileEML.h"

#include "Canvas.h"
#include "WriteInfo.h"

#include "../records/Segment.h"
#include "../records/Board.h"
#include "../files/Buffer.h"
#include "../files/Chunk.h"
#include "../util/parse.h"
#include "../handling/Bexcept.h"

using namespace std;


static string EMLdashes, EMLequals;


void setEMLTables()
{
  EMLdashes.resize(0);
  EMLdashes.insert(0, 12, ' ');
  EMLdashes.insert(12, 43, '-');

  EMLequals.resize(0);
  EMLequals.insert(0, 79, '=');
}


static void readEMLCanvas(
  Buffer& buffer,
  vector<string>& canvas,
  Chunk& chunk)
{
  LineData lineData;
  while (buffer.next(lineData))
  {
    chunk.set(BRIDGE_FORMAT_TITLE, "", lineData.no); // Not very accurate...
    if (lineData.type == BRIDGE_BUFFER_EMPTY)
    {
      // If some players aren't given, we might have an empty line.
      if (canvas.size() != 7 && canvas.size() != 13)
        continue;
    }
    else if (lineData.type == BRIDGE_BUFFER_COMMENT)
      continue;

    if (lineData.type == BRIDGE_BUFFER_DASHES)
      break;

    canvas.push_back(lineData.line);
  }
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
  Chunk& chunk)
{
  string st;
  if (! readNextWord(canvas[0], 0, st))
    THROW("Cannot find scoring");
  chunk.set(BRIDGE_FORMAT_SCORING, st);

  if (! readAllWords(canvas[1], 16, 23, st))
    chunk.set(BRIDGE_FORMAT_NORTH, "");
  else
    chunk.set(BRIDGE_FORMAT_NORTH, st);
    
  if (! readAllWords(canvas[7], 4, 11, st))
    chunk.set(BRIDGE_FORMAT_WEST, "");
  else
    chunk.set(BRIDGE_FORMAT_WEST, st);

  if (! readAllWords(canvas[7], 27, 34, st))
    chunk.set(BRIDGE_FORMAT_EAST, "");
  else
    chunk.set(BRIDGE_FORMAT_EAST, st);

  if (! readAllWords(canvas[13], 16, 23, st))
    chunk.set(BRIDGE_FORMAT_SOUTH, "");
  else
    chunk.set(BRIDGE_FORMAT_SOUTH, st);

  if (! readNextWord(canvas[0], 54, st))
    THROW("Cannot find board number");
  chunk.set(BRIDGE_FORMAT_BOARD_NO, st);

  if (! readNextWord(canvas[1], 5, st))
    THROW("Cannot find dealer");
  chunk.set(BRIDGE_FORMAT_DEALER, st);

  if (! readNextWord(canvas[2], 5, st))
    THROW("Cannot find vulnerability");
  chunk.set(BRIDGE_FORMAT_VULNERABLE, st);

  if (! readNextWord(canvas[resultLine], 50, st))
    THROW("Cannot find result");
  chunk.set(BRIDGE_FORMAT_RESULT, st);

  if (! readNextWord(canvas[resultLine+1], 49, st))
    THROW("Cannot find score");
  st.pop_back(); // Drop trailing comma
  chunk.set(BRIDGE_FORMAT_SCORE, st);

  if (canvas[resultLine+1].back() != ':')
  {
    if (! readLastWord(canvas[resultLine+1], st))
      THROW("Cannot find IMP result");
    chunk.set(BRIDGE_FORMAT_SCORE_IMP, st);
  }

  if (resultLine+2 < canvas.size() &&
      canvas[resultLine+2].length() >= 52 &&
      canvas[resultLine+2].substr(42, 8) == "Contract")
  {
    // This is not standard EML, but we need the contract in
    // case there's no auction.
    if (! readLastWord(canvas[resultLine+2], st))
      THROW("Cannot find contract");
    chunk.set(BRIDGE_FORMAT_CONTRACT, st);
  }
}


static void getEMLDeal(
  const vector<string>& canvas,
  Chunk& chunk)
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

  chunk.set(BRIDGE_FORMAT_DEAL, d.str());
}


static void getEMLAuction(
  const vector<string>& canvas,
  const unsigned resultLine,
  Chunk& chunk)
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

  chunk.set(BRIDGE_FORMAT_AUCTION, d.str());
}


static void getEMLPlay(
  const vector<string>& canvas,
  const bool playIsPresent,
  const unsigned resultLine,
  const unsigned westLine,
  const unsigned cardStart,
  Chunk& chunk)
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
    chunk.set(BRIDGE_FORMAT_PLAY, opld);
    return;
  }

  string wd;
  unsigned pos0 = cardStart;
  unsigned la = static_cast<unsigned>(canvas[westLine-1].size());
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
      
  chunk.set(BRIDGE_FORMAT_PLAY, d.str());
}


void readEMLChunk(
  Buffer& buffer,
  Chunk& chunk,
  bool& newSegFlag)
{
  newSegFlag = false;

  // First get all the lines of a hand.
  vector<string> canvas;
  readEMLCanvas(buffer, canvas, chunk);
  
  if (canvas.size() == 0)
    return;
  else if (canvas.size() < 17)
    THROW("Canvas too short");

  // Then parse them into the chunk structure.
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
  const Segment& segment,
  const Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  const Instance& instance = board.getInstance(writeInfo.instNo);

  if (board.skipped(writeInfo.instNo))
    return;

  // Convert deal, auction and play from \n to vectors.
  vector<string> deal, auction, play;
  str2lines(board.strDeal(BRIDGE_WEST, format), deal);
  str2lines(instance.strAuction(format), auction);
  str2lines(instance.strPlay(format), play);

  // Height of auction determines dimensions.
  // It seems we leave out the play if that makes the canvas too large.
  const unsigned as = static_cast<unsigned>(auction.size());
  const unsigned a = max(4u, as);
  const unsigned alstart = (a <= 6 ? 14 : a+8);
  bool const playFlag = (play[0].size() > 2 && alstart < 19 ? true : false);
  const unsigned clen = (playFlag ? alstart + 5 : max(18u, as+6));
  const unsigned acstart = (play[0].length() > 38 ? 39u : 42u);

  Canvas canvas;
  canvas.resize(clen, 80);

  canvas.setRectangle(deal, 0, 4);
  canvas.setRectangle(auction, 2, 42);
  if (playFlag)
    canvas.setRectangle(play, alstart, acstart);

  canvas.setLine(segment.strScoring(format), 0, 0);
  canvas.setLine(instance.strPlayer(BRIDGE_WEST, format), 7, 4);
  canvas.setLine(instance.strPlayer(BRIDGE_NORTH, format), 1, 16);
  canvas.setLine(instance.strPlayer(BRIDGE_EAST, format), 7, 27);
  canvas.setLine(instance.strPlayer(BRIDGE_SOUTH, format), 13, 16);
  canvas.setLine(instance.strPlayer(BRIDGE_WEST, format), 3, 42);
  canvas.setLine(instance.strPlayer(BRIDGE_NORTH, format), 3, 51);
  canvas.setLine(instance.strPlayer(BRIDGE_EAST, format), 3, 60);
  canvas.setLine(instance.strPlayer(BRIDGE_SOUTH, format), 3, 69);
  canvas.setLine(segment.strNumber(writeInfo.bno, format), 0, 42);

  canvas.setLine(instance.strDealer(format), 1, 0);
  canvas.setLine(instance.strVul(format), 2, 0);
  const string l = instance.strLead(format);
  unsigned p;
  if (l == "Opening Lead:")
    p = a+3;
  else
  {
    canvas.setLine(l, a+3, 42);
    p = a+4;
  }
  canvas.setLine(instance.strResult(format), p, 42);

  string ss;
  if (segment.scoringIsIMPs() && writeInfo.ino == 1)
    ss = board.strScore(writeInfo.instNo, format);
  else
    ss = instance.strScore(format);
  canvas.setLine(ss, p+1, 42);

  if (auction.size() == 3 && auction[2] == "")
  {
    // Not standard EML: Give the contract.
    canvas.setLine("Contract: " + instance.strContract(format), p+2, 42);
  }

  st += canvas.str() + "\n";

  if (writeInfo.ino < writeInfo.numInst-1 && writeInfo.numInstActive > 1)
    st += EMLdashes + "\n\n";
  else if (! writeInfo.last)
    st += EMLequals + "\n\n";
}

