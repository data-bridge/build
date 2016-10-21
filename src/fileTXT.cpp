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
#include "Canvas.h"
#include "fileTXT.h"
#include "parse.h"

using namespace std;


string TXTdashes;
string TXTshortDashes;


static bool readTXTCanvas(
  ifstream& fstr,
  unsigned& lno,
  vector<string>& canvas);

static bool getTXTCanvasOffset(
  const vector<string>& canvas,
  unsigned& auctionLine,
  vector<string>& chunk);

static bool getTXTFields(
  vector<string>& canvas,
  const unsigned auctionLine,
  vector<string>& chunk);

static bool getTXTDeal(
  const vector<string>& canvas,
  const unsigned offset,
  vector<string>& chunk);

static bool getTXTAuction(
  vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk);

static bool getTXTPlay(
  const vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk);


void setTXTTables()
{
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
      if (ReadNextWord(prevLine, 0, wd) && 
          (wd == "Down" || wd == "Made" || wd == "Passed"))
      {
        int seen = count(prevLine.begin(), prevLine.end(), ' ');
        if (seen > 4 || (wd == "Passed" && seen >= 3))
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
        {
          if (seenDeal)
            break;
          else
            continue;
        }
      }
    }

    canvas.push_back(line);
  }
  return true;
}


static bool getTXTCanvasOffset(
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
    return false;

  if (auctionLine == 0)
    // No player names present.
    return true;

  // Get the names.

  unsigned aline = auctionLine-1;

  unsigned l = canvas[aline].length();
  unsigned n = 0;

  if (! ReadNextWord(canvas[aline], 0, chunk[BRIDGE_FORMAT_WEST])) 
    return false;
  n += chunk[BRIDGE_FORMAT_WEST].length();
  while (n < l && canvas[aline].at(n) == ' ') n++;

  if (! ReadNextWord(canvas[aline], n, chunk[BRIDGE_FORMAT_NORTH])) 
    return false;
  n += chunk[BRIDGE_FORMAT_NORTH].length();
  while (n < l && canvas[aline].at(n) == ' ') n++;

  if (! ReadNextWord(canvas[aline], n, chunk[BRIDGE_FORMAT_EAST])) 
    return false;
  n += chunk[BRIDGE_FORMAT_EAST].length();
  while (n < l && canvas[aline].at(n) == ' ') n++;

  if (! ReadNextWord(canvas[aline], n, chunk[BRIDGE_FORMAT_SOUTH])) 
    return false;

  return true;
}


static bool getTXTFields(
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
    chunk[BRIDGE_FORMAT_TITLE] = canvas[0];
    chunk[BRIDGE_FORMAT_DATE] = canvas[1];
    chunk[BRIDGE_FORMAT_LOCATION] = canvas[2];
    chunk[BRIDGE_FORMAT_EVENT] = canvas[3];
    chunk[BRIDGE_FORMAT_SESSION] = canvas[4];
    chunk[BRIDGE_FORMAT_SCORING] = "IMPs"; // Maybe others possible
    chunk[BRIDGE_FORMAT_TEAMS] = canvas[5];
    bline = 6;
  }

  if (aline > 11)
  {
    if (! ReadNextWord(canvas[bline], 0, chunk[BRIDGE_FORMAT_BOARD_NO])) 
      return false;
    chunk[BRIDGE_FORMAT_BOARD_NO].pop_back(); // Drop trailing point

    // Attempt to read dealer.  Pavlicek only puts a dealer
    // when the auction is not given.

    (void) ReadNextWord(canvas[bline+13], 0, chunk[BRIDGE_FORMAT_DEALER]);

    if (! ReadNextWord(canvas[bline+14], 0, chunk[BRIDGE_FORMAT_VULNERABLE])) 
      return false;

    if (! getTXTDeal(canvas, bline, chunk)) 
      return false;
  }

  unsigned cline = aline+1;
  if (! getTXTAuction(canvas, cline, chunk))
    return false;

  if (canvas[cline].length() < 10 ||
      canvas[cline].substr(0, 10) != "Passed out")
  {
    chunk[BRIDGE_FORMAT_CONTRACT] = canvas[cline++];

    if (canvas[cline].size() < 5)
      return false;

    string wd = canvas[cline].substr(0, 5);
    if (wd == "Trick")
    {
      cline++;
      if (! getTXTPlay(canvas, cline, chunk))
        return false;
      cline++;
    }
    else if (wd == "Lead:")
    {
      if (! ReadLastWord(canvas[cline], wd))
        return false;
      if (wd.size() == 3 && wd.substr(1, 2) == "10")
        chunk[BRIDGE_FORMAT_PLAY] = wd.substr(0, 1) + "T";
      else
        chunk[BRIDGE_FORMAT_PLAY] = wd;
      cline++;
    }

    if (canvas[cline].size() < 5)
      return false;
    chunk[BRIDGE_FORMAT_RESULT] = canvas[cline];
  }

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
  vector<string>& canvas,
  unsigned& offset,
  vector<string>& chunk)
{
  const unsigned l0 = canvas[offset].length();
  if (l0 > 6 && l0 < 11)
  {
    // No auction, e.g. "5Cx North".
    chunk[BRIDGE_FORMAT_AUCTION] = "";
    return true;
  }

  stringstream d;
  d.clear();

  // The auction fields are normally 12 characters wide.
  // With long names this may run over, but hopefully not by too much.
  unsigned firstStart = GobbleLeadingSpace(canvas[offset]);

  if (firstStart == l0)
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
    while (GetNextWord(canvas[l], wd))
    // for (unsigned beg = (l == offset ? firstStart : 0); beg < 48; beg += 12)
    {
      // if (! ReadNextWord(canvas[l], beg, wd))
      // {
        // done = true;
        // break;
      // }
      GobbleLeadingSpace(canvas[l]);

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
  if (! getTXTCanvasOffset(canvas, auctionLine, chunk))
    return false;

  newSegFlag = false;
  return getTXTFields(canvas, auctionLine, chunk);
}


void writeTXTSegmentLevel(
  ofstream& fstr,
  Segment * segment,
  const formatType f)
{
  // if (segment->CarryExists())
  if (segment->GetExtBoardNo(0) != 1 || segment->CarryExists())
  {
    // Pavlicek bug.
      fstr << TXTdashes << "\n";
  }

  fstr << "\n" << segment->TitleAsString(f) << "\n";
  fstr << segment->DateAsString(f);
  fstr << segment->LocationAsString(f);
  fstr << segment->EventAsString(f);
  fstr << segment->SessionAsString(f);
  fstr << segment->TeamsAsString(f) << "\n\n";
}


void writeTXTBoardLevel(
  ofstream& fstr,
  Segment * segment,
  Board * board,
  writeInfoType& writeInfo,
  const formatType f)
{
  Canvas canvas;

  board->CalculateScore();

  if (writeInfo.ino == 0)
  {
    const string dstr = board->DealAsString(BRIDGE_WEST, f);
    const string bstr = segment->NumberAsString(f, writeInfo.bno);
    const string vstr = board->VulAsString(f);
    const string lstr = (board->AuctionIsEmpty() ?
      board->DealerAsString(f) : "");

    // Convert deal, auction and play from \n to vectors.
    vector<string> deal;
    ConvertMultilineToVector(dstr, deal);

    canvas.SetDimensions(15, 80);
    canvas.SetRectangle(deal, 0, 0);
    canvas.SetLine(bstr, 0, 0);
    canvas.SetLine(lstr, 13, 0);
    canvas.SetLine(vstr, 14, 0);
    fstr << canvas.AsString() << "\n";

    fstr << board->PlayersAsString(f);
    fstr << board->AuctionAsString(f) << "\n";
    fstr << board->ContractAsString(f);
    fstr << board->PlayAsString(f);

    fstr << board->ResultAsString(f, false) << "\n";
  }
  else
  {
    const string p = board->PlayersAsString(f);
    // Pavlicek bug?
    if (p == "")
      fstr << "\n";
    else
      fstr << board->PlayersAsString(f);

    fstr << board->AuctionAsString(f) << "\n";
    fstr << board->ContractAsString(f);
    fstr << board->PlayAsString(f);

    int s = board->ScoreIMPAsInt();
    string tWin;
    if (s > 0)
    {
      writeInfo.score2 += s;
      tWin = segment->SecondTeamAsString(f);
    }
    else
    {
      writeInfo.score1 += -s;
      tWin = segment->FirstTeamAsString(f);
    }

    fstr << board->ResultAsString(f, tWin) << "\n";
    fstr << 
      segment->TeamsAsString(writeInfo.score1, writeInfo.score2, f) << "\n";
    if (writeInfo.bno != writeInfo.numBoards-1)
      fstr << TXTdashes << "\n\n";
  }
}

