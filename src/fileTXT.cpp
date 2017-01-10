/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <sstream>
#include <regex>

#include "Group.h"
#include "Segment.h"
#include "Canvas.h"
#include "fileTXT.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


string TXTdashes;


static void getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine,
  vector<string>& chunk);

static void getTXTFields(
  vector<string>& canvas,
  const unsigned auctionLine,
  vector<string>& chunk);

static void getTXTDeal(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk);

static void getTXTAuction(
  vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk);

static void getTXTPlay(
  const vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk);


void setTXTTables()
{
  TXTdashes.resize(0);
  TXTdashes.insert(0, 41, '-');
}


static void readTXTCanvas(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& canvas)
{
  LineData lineData;
  bool seenDeal = false;
  while (buffer.next(lineData))
  {
    lno[0] = lineData.no; // Not very good...
    if (lineData.type == BRIDGE_BUFFER_EMPTY)
    {
      if (! seenDeal)
        continue;

      LineData prevData;
      if (! buffer.previous(prevData))
        THROW("Should not happen");

      string wd;
      if (readNextWord(prevData.line, 0, wd) && 
          (wd == "Down" || wd == "Made" || wd == "Passed"))
      {
        int seen = static_cast<int>
	  (count(prevData.line.begin(), prevData.line.end(), ' '));
        if (seen > 4 || (wd == "Passed" && seen >= 3))
          // Team line still to come
          continue;
        else
          return;
      }
      else
        continue;
    }
    else if (lineData.type == BRIDGE_BUFFER_COMMENT)
      continue;

    if (lineData.len > 25)
    {
      if (lineData.line.substr(0, 4) == "West")
        seenDeal = true;
      else if (lineData.type == BRIDGE_BUFFER_DASHES)
      {
        if (seenDeal)
          return;
        else
          continue;
      }
    }

    canvas.push_back(lineData.line);
  }
}


static void getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine,
  vector<string>& chunk)
{
  auctionLine = 0;
  string wd = "";
  while (auctionLine < canvas.size())
  {
    // Must distinguish from "West Dlr".
    if (canvas[auctionLine].size() > 12 &&
        canvas[auctionLine].substr(0, 6) == "West  ")
      break;
    auctionLine++;
  }

  if (auctionLine == canvas.size())
    THROW("Cannot find end of canvas");

  if (auctionLine == 0)
    // No player names present.
    return;

  // Get the names.

  unsigned aline = auctionLine-1;
  unsigned n = 0;

  // The fields can be longer than 12 characters...
  // The whole line can also be absent.

  const unsigned ll = static_cast<unsigned>(canvas[aline].length());
  if (ll > 13u && ll < 36u && canvas[aline].substr(12, 2) == "C ")
    return;

  if (readAllWordsOverlong(canvas[aline], n, n+11, 
      chunk[BRIDGE_FORMAT_WEST])) 
    n += static_cast<unsigned>
      (Max(12, chunk[BRIDGE_FORMAT_WEST].length()+1));
  else
    n += 12;

  if (readAllWordsOverlong(canvas[aline], n, n+11, 
      chunk[BRIDGE_FORMAT_NORTH])) 
    n += static_cast<unsigned>
      (Max(12, chunk[BRIDGE_FORMAT_NORTH].length()+1));
  else
    n += 12;

  if (readAllWordsOverlong(canvas[aline], n, n+11, 
      chunk[BRIDGE_FORMAT_EAST])) 
    n += static_cast<unsigned>
      (Max(12, chunk[BRIDGE_FORMAT_EAST].length()+1));
  else
    n += 12;

  (void) readAllWordsOverlong(canvas[aline], n, n+11, 
      chunk[BRIDGE_FORMAT_SOUTH]);
}


static void getTXTFields(
  vector<string>& canvas,
  const unsigned aline,
  vector<string>& chunk)
{
  unsigned bline = 0;
  if (canvas[0].size() < 19 || 
     (canvas[0].substr(12, 7) != "  North" &&
      (canvas[1].size() < 4 || canvas[1].substr(0, 4) != "West") &&
       canvas[0].substr(0, 4) != "West" &&
       canvas[0].substr(12, 5) != "North"))
  {
    // Guess the number of header lines.
    bline = 6;
    string tmp;
    while (bline != 0 && ! readNextWord(canvas[bline], 0, tmp))
      bline--;

    if (bline == 6)
    {
      chunk[BRIDGE_FORMAT_TITLE] = canvas[0];
      chunk[BRIDGE_FORMAT_DATE] = canvas[1];
      chunk[BRIDGE_FORMAT_LOCATION] = canvas[2];
      chunk[BRIDGE_FORMAT_EVENT] = canvas[3];
      chunk[BRIDGE_FORMAT_SESSION] = canvas[4];
      chunk[BRIDGE_FORMAT_SCORING] = "IMPs"; // Maybe others possible
      chunk[BRIDGE_FORMAT_TEAMS] = canvas[5];
    }
    else if (bline == 5)
    {
      // Probably.  Could do a better job here.
      chunk[BRIDGE_FORMAT_TITLE] = canvas[0];
      chunk[BRIDGE_FORMAT_DATE] = canvas[1];
      chunk[BRIDGE_FORMAT_EVENT] = canvas[2];
      chunk[BRIDGE_FORMAT_SESSION] = canvas[3];
      chunk[BRIDGE_FORMAT_SCORING] = "IMPs"; // Maybe others possible
      chunk[BRIDGE_FORMAT_TEAMS] = canvas[4];
    }
    else
      THROW("Cannot locate header");
  }

  if (aline > 11)
  {
    if (! readNextWord(canvas[bline], 0, chunk[BRIDGE_FORMAT_BOARD_NO])) 
      THROW("Cannot find board number");
    chunk[BRIDGE_FORMAT_BOARD_NO].pop_back(); // Drop trailing point

    // Attempt to read dealer.  Pavlicek only puts a dealer
    // when the auction is not given.

    (void) readNextWord(canvas[bline+13], 0, chunk[BRIDGE_FORMAT_DEALER]);

    if (! readNextWord(canvas[bline+14], 0, chunk[BRIDGE_FORMAT_VULNERABLE])) 
      THROW("Cannot find vulnerability");

    getTXTDeal(canvas, bline, chunk);
  }

  unsigned cline = aline+1;
  getTXTAuction(canvas, cline, chunk);

  if (canvas[cline].length() < 10 ||
      canvas[cline].substr(0, 10) != "Passed out")
  {
    chunk[BRIDGE_FORMAT_CONTRACT] = canvas[cline++];

    if (canvas[cline].size() < 5)
      THROW("Cannot find trick");

    string wd = canvas[cline].substr(0, 5);
    if (wd == "Trick")
    {
      cline++;
      getTXTPlay(canvas, cline, chunk);
      cline++;
    }
    else if (wd == "Lead:")
    {
      if (! readLastWord(canvas[cline], wd))
        THROW("Cannot find lead");
      if (wd.size() == 3 && wd.substr(1, 2) == "10")
        chunk[BRIDGE_FORMAT_PLAY] = wd.substr(0, 1) + "T";
      else
        chunk[BRIDGE_FORMAT_PLAY] = wd;
      cline++;
    }

    if (canvas[cline].size() < 5)
      THROW("Cannot find result");
    chunk[BRIDGE_FORMAT_RESULT] = canvas[cline];
  }

  // Ignore running IMP score, as we regenerate this.
}


static void getTXTDeal(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk)
{
  string sts, sth, std, stc;

  stringstream d;
  d << "W:";

  if (! readNextSpacedWord(canvas[offset+6], 2, sts)) sts = "";
  if (! readNextSpacedWord(canvas[offset+7], 2, sth)) sth = "";
  if (! readNextSpacedWord(canvas[offset+8], 2, std)) std = "";
  if (! readNextSpacedWord(canvas[offset+9], 2, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextSpacedWord(canvas[offset+1], 14, sts)) sts = "";
  if (! readNextSpacedWord(canvas[offset+2], 14, sth)) sth = "";
  if (! readNextSpacedWord(canvas[offset+3], 14, std)) std = "";
  if (! readNextSpacedWord(canvas[offset+4], 14, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextSpacedWord(canvas[offset+6], 26, sts)) sts = "";
  if (! readNextSpacedWord(canvas[offset+7], 26, sth)) sth = "";
  if (! readNextSpacedWord(canvas[offset+8], 26, std)) std = "";
  if (! readNextSpacedWord(canvas[offset+9], 26, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc << " ";

  if (! readNextSpacedWord(canvas[offset+11], 14, sts)) sts = "";
  if (! readNextSpacedWord(canvas[offset+12], 14, sth)) sth = "";
  if (! readNextSpacedWord(canvas[offset+13], 14, std)) std = "";
  if (! readNextSpacedWord(canvas[offset+14], 14, stc)) stc = "";
  d << sts << "." << sth <<  "." << std << "." << stc;

  // Turn -- (void) into nothing.
  regex re("--");
  chunk[BRIDGE_FORMAT_DEAL] = regex_replace(d.str(), re, string(""));
}


static void getTXTAuction(
  vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk)
{
  const unsigned l0 = static_cast<unsigned>(canvas[offset].length());
  if (l0 > 6 && l0 < 11 && canvas[offset] != "All Pass")
  {
    // No auction, e.g. "5Cx North".
    chunk[BRIDGE_FORMAT_AUCTION] = "";
    return;
  }

  stringstream d;
  d.clear();

  // The auction fields are normally 12 characters wide.
  // With long names this may run over, but hopefully not by too much.
  unsigned firstStart = trimLeading(canvas[offset]);

  if (firstStart == l0)
    THROW("Cannot fine start of auction");

  chunk[BRIDGE_FORMAT_DEALER] = PLAYER_NAMES_LONG[
    ((firstStart/12) + BRIDGE_WEST) % 4];

  string wd;
  unsigned no = 0;
  bool done = false;
  unsigned l;
  unsigned numPasses = 0;
  for (l = offset; l < canvas.size(); l++)
  {
    const unsigned ll = static_cast<unsigned>(canvas[l].length());
    if (ll >= 7 && ll <= 11 && canvas[l] != "All Pass")
    {
      // "5S South", not an actual bid.
      l--;
      break;

    }

    while (getNextWord(canvas[l], wd))
    {
      trimLeading(canvas[l]);

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
    THROW("Canvas too short for auction");

  chunk[BRIDGE_FORMAT_AUCTION] = d.str();
  offset = l+1;
}


static void getTXTPlay(
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
    if (canvas[l].length() < 3 ||
       (canvas[l].substr(1, 2) != ". " && canvas[l].substr(2, 2) != ". "))
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

      if (! readNextWord(canvas[l], p, wd))
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
    THROW("Cannot find play");
  offset = l;
  chunk[BRIDGE_FORMAT_PLAY] = d.str();
}


void readTXTChunk(
  Buffer& buffer,
  vector<unsigned>& lno,
  vector<string>& chunk,
  bool& newSegFlag)
{
  // First get all the lines of a hand.
  vector<string> canvas;
  readTXTCanvas(buffer, lno, canvas);
  
  if (canvas.size() == 0)
    return;

  // Then parse them into the chunk structure.
  unsigned auctionLine = 0;
  getTXTCanvasOffset(canvas, auctionLine, chunk);

  newSegFlag = false;
  getTXTFields(canvas, auctionLine, chunk);
}


void writeTXTSegmentLevel(
  string& st,
  Segment& segment,
  const Format format)
{
  if (segment.getExtBoardNo(0) != 1 || segment.hasCarry())
  {
    // Pavlicek bug.
    st += TXTdashes + "\n";
  }

  st += "\n" + segment.strTitle(format) + "\n";
  st += segment.strDate(format);
  st += segment.strLocation(format);
  st += segment.strEvent(format);
  st += segment.strSession(format);
  st += segment.strTeams(format) + "\n\n";
}


void writeTXTBoardLevel(
  string& st,
  Segment& segment,
  Board& board,
  WriteInfo& writeInfo,
  const Format format)
{
  Canvas canvas;
  string tmp;

  board.calculateScore();

  if (writeInfo.ino == 0)
  {
    const string dstr = board.strDeal(BRIDGE_WEST, format);
    const string bstr = segment.strNumber(writeInfo.bno, format);
    const string vstr = board.strVul(format);
    const string lstr = (board.auctionIsEmpty() ?  board.strDealer(format) : "");

    // Convert deal, auction and play from \n to vectors.
    vector<string> deal;
    str2lines(dstr, deal);

    canvas.resize(15, 80);
    canvas.setRectangle(deal, 0, 0);
    canvas.setLine(bstr, 0, 0);
    canvas.setLine(lstr, 13, 0);
    canvas.setLine(vstr, 14, 0);
    st += canvas.str() + "\n";

    st += board.strPlayers(format);
    st += board.strAuction(format) + "\n";
    st += board.strContract(format);
    st += board.strPlay(format);

    st += board.strResult(format, false) + "\n";
  }
  else
  {
    const string p = board.strPlayers(format);
    // Pavlicek bug?
    if (p == "")
      st += "\n";
    else
      st += board.strPlayers(format); // p?!

    st += board.strAuction(format) + "\n";
    st += board.strContract(format);
    st += board.strPlay(format);

    int s = board.IMPScore();
    string tWin;
    if (s > 0)
    {
      writeInfo.score2 += (s > 0 ? s : -s);
      tWin = segment.strSecondTeam(format);
    }
    else
    {
      writeInfo.score1 += (s > 0 ? s : -s);
      tWin = segment.strFirstTeam(format);
    }

    st += board.strResult(format, tWin) + "\n";
    st += 
      segment.strTeams(writeInfo.score1, writeInfo.score2, format) + "\n";
    if (writeInfo.bno != writeInfo.numBoards-1)
      st += TXTdashes + "\n\n";
  }
}

