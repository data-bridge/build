/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>
#include <string>
#include <regex>

#include "Segment.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


#define BIG_BOARD 99999


Segment::Segment()
{
  Segment::reset();
}


Segment::~Segment()
{
}


void Segment::reset()
{
  len = 0;
  boards.clear();
  LINcount = 0;
  LINdata.clear();

  bmin = BIG_BOARD;
  bmax = 0;

  title = ""; 
  date.reset();
  location.reset();
  event = ""; 
  session.reset(); 
  scoring.reset();
  teams.reset();
}


Board * Segment::getBoard(const unsigned intNo) 
{
  for (auto &p: boards)
  {
    if (p.no == intNo)
    {
      activeBoard = &p.board;
      return &p.board;
    }
  }
  return nullptr;
}


Board * Segment::acquireBoard(const unsigned intNo)
{
  for (auto &p: boards)
  {
    if (p.no == intNo)
      return &p.board;
  }

  // Make a new board
  boards.resize(len+1);
  boards[len].no = intNo;
  boards[len].extNo = 0;
  len++;
  activeBoard = &boards[len-1].board;
  activeNo = intNo;

  if (activeBoard == nullptr)
    THROW("Could not make board: " + STR(intNo));

  return activeBoard;
}


void Segment::setBoard(const unsigned intNo)
{
  for (auto &p: boards)
  {
    if (p.no == intNo)
    {
      activeBoard = &p.board;
      return;
    }
  }
  THROW("Bad internal board number: " + STR(intNo));
}


unsigned Segment::getExtBoardNo(const unsigned intNo) const
{
  for (auto &p: boards)
  {
    if (p.no == intNo)
      return p.extNo;
  }
  THROW("Bad internal board number: " + STR(intNo));
}


unsigned Segment::getActiveExtBoardNo() const
{
  return Segment::getExtBoardNo(activeNo);
}


void Segment::loadFromHeader(
  const unsigned intNo,
  const unsigned instNo)
{
  if (LINcount == 0)
    return;

  Board * board = Segment::acquireBoard(intNo);
  if (board == nullptr)
    THROW("Acquired null board");

  board->setInstance(instNo);

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    if (LINdata[intNo].players[instNo][p] != "")
      board->setPlayer(LINdata[intNo].players[instNo][p],
        static_cast<Player>(p));
  }

  if (instNo == 0)
    board->setLINheader(LINdata[intNo]);
}


void Segment::setTitleLIN(const string& t)
{
  // We figure out which of the LIN formats is used.
  // We cater to several uses of the first three fields:
  //
  //           own table   own MP    own tourney   Vugraph    RBN-generated
  // 0         BBO         #9651 ... #9651 ...     41st...    T + S(1)
  // 1         IMPs        BBO       BBO           DOSB-...   S(2)
  // 2         P           P         I             I          I
  // scoring   I           P         I             I          I

  int seen = std::count(t.begin(), t.end(), ',');
  if (seen != 8)
    THROW("LIN vg needs exactly 8 commas.");

  vector<string> v(9);
  v.clear();
  tokenize(t, v, ",");

  // Pick out the RBN-generated line.
  regex re("^(.+)\\s+(\\w+)$");
  smatch match;
  if (regex_search(v[0], match, re) && 
      match.size() >= 2 &&
      session.isStage(match.str(2)) &&
      session.isSegmentLike(v[1]))
  {
    Segment::setTitle(match.str(1), BRIDGE_FORMAT_RBN);

    // Make a synthetic RBN-like session line (a bit wasteful).
    stringstream s;
    s << match.str(2) << ":" << v[1];
    Segment::setSession(s.str(), BRIDGE_FORMAT_RBN);
    event = "";
  }
  else if (session.isRoundOfLike(v[1]))
  {
    Segment::setTitle(v[0], BRIDGE_FORMAT_RBN);
    Segment::setSession(v[1], BRIDGE_FORMAT_RBN);
    event = "";
  }
  else
  {
    // Simple case.
    Segment::setTitle(v[0], BRIDGE_FORMAT_RBN);
    Segment::setEvent(v[1], BRIDGE_FORMAT_RBN);
  }

  // Scoring (2).
  if (v[2] == "P" && v[1] != "IMPs")
    Segment::setScoring("P", BRIDGE_FORMAT_LIN);
  else
    Segment::setScoring("I", BRIDGE_FORMAT_LIN);
    
  // Board numbers (3-4).
  if (v[3] == "")
    bInmin = 0;
  else if (! str2upos(v[3], bInmin))
    THROW("Not a board number");

  if (v[4] == "")
    bInmax = 0;
  else if (! str2upos(v[4], bInmax))
    THROW("Not a board number");

  // Teams (5-8): Synthesize an RBN-like team line (a bit wasteful).
  stringstream s;
  s << v[5] << ":" << v[7];
  if (v[6] != "" || v[8] != "")
  {
    s << ":" << (v[6] == "" ? 0 : v[6]);
    s << ":" << (v[8] == "" ? 0 : v[8]);
  }
  Segment::setTeams(s.str(), BRIDGE_FORMAT_LIN);
}


unsigned Segment::size() const
{
  return len;
}


unsigned Segment::count() const
{
  unsigned cnt = 0;
  for (auto &p: boards)
    cnt += p.board.count();

  return cnt;
}


void Segment::setTitle(
  const string& text,
  const Format format)
{
  // In LIN this includes the entire vg line.
  
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      Segment::setTitleLIN(text);
      break;

    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_TXT:
      title = text;
      break;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Segment::setDate(
  const string& text,
  const Format format)
{
  date.set(text, format);
}


void Segment::setLocation(
  const string& text,
  const Format format)
{
  location.set(text, format);
}


void Segment::setEvent(
  const string& text,
  const Format format)
{
  UNUSED(format);
  event = text;
}


void Segment::setSession(
  const string& text,
  const Format format)
{
  session.set(text, format);
}


void Segment::setScoring(
  const string& text,
  const Format format)
{
  scoring.set(text, format);
}


void Segment::setTeams(
  const string& text,
  const Format format)
{
  teams.set(text, format); 
}


void Segment::setFirstTeam(
  const string& text,
  const Format format)
{
  teams.setFirst(text, format); 
}


void Segment::setSecondTeam(
  const string& text,
  const Format format)
{
  teams.setSecond(text, format); 
}


void Segment::setPlayers(
  const string& text,
  const Format format)
{
  activeBoard->setPlayers(text, format);
}


void Segment::setWest(
  const string& text,
  const Format format)
{
  UNUSED(format);
  activeBoard->setPlayer(text, BRIDGE_WEST);
}


void Segment::setNorth(
  const string& text,
  const Format format)
{
  UNUSED(format);
  activeBoard->setPlayer(text, BRIDGE_NORTH);
}


void Segment::setEast(
  const string& text,
  const Format format)
{
  UNUSED(format);
  activeBoard->setPlayer(text, BRIDGE_EAST);
}


void Segment::setSouth(
  const string& text,
  const Format format)
{
  UNUSED(format);
  activeBoard->setPlayer(text, BRIDGE_SOUTH);
}


void Segment::setResultsList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));
  
  size_t c = countDelimiters(text, ",");
  if (c == 0 || c > 100)
    THROW("Bad number of fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  // There may or may not be a trailing comma
  if (c % 2 == 0)
    c--;

  if (c+1 > 2*LINcount)
  {
    LINcount = (c+1)/2;
    LINdata.resize(LINcount);
  }

  for (size_t b = 0, d = 0; b < c+1; b += 2, d++)
  {
    LINdata[d].contract[0] = tokens[b];
    LINdata[d].contract[1] = tokens[b+1];
  }
}


void Segment::setPlayersList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  if (c == 7)
  {
    // Assume a single set of 8 players repeating.
    vector<string> tokens(c+1);
    tokens.clear();
    tokenize(text, tokens, ",");

    for (size_t b = 0; b < LINcount; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b].players[0][(d+2) % 4] = tokens[d];
        LINdata[b].players[1][(d+2) % 4] = tokens[d+4];
      }
    }
  }
  else
  {
    // Assume all players are given.
    if (c+1 != 4*LINcount)
      THROW("Wrong number of fields");

    vector<string> tokens(c+1);
    tokens.clear();
    tokenize(text, tokens, ",");

    for (size_t b = 0; b < c; b += 8)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b >> 3].players[0][(d+2) % 4] = tokens[b+d];
        LINdata[b >> 3].players[1][(d+2) % 4] = tokens[b+d+4];
      }
    }
  }
}


void Segment::loadSpecificsFromHeader(const string& room)
{
  if (LINcount == 0)
    return;

  if (activeBoard == nullptr)
    THROW("activeBoard == nullptr");

  int r = (room != "" && room.substr(0, 1) == "c" ? 1 : 0);

  if (activeNo == 0 &&
      activeBoard->getInstance() == 0 &&
      r == 1)
  {
    // Teams from LIN header were in "wrong" order for our internal
    // format, which is consistent with RBN:  The first team is NS
    // in the first room.  In LIN, the first team is NS in Open.
    teams.swap();
  }

  const Format format = BRIDGE_FORMAT_LIN;
  activeBoard->setContract(LINdata[activeNo].contract[r], format);

  string st = "";
  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
  {
    st += LINdata[activeNo].players[r][(i+2) % 4];
    if (i < 3)
      st += ",";
  }

  if (st != ",,,")
    activeBoard->setPlayers(st, format);
}



void Segment::setPlayersHeader(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  if (c != 7)
    THROW("Bad number of fields");

  vector<string> tokens(8);
  tokens.clear();
  tokenize(text, tokens, ",");

  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
  {
    LINdata[0].players[0][(i+2) % 4] = tokens[i];
    LINdata[0].players[1][(i+2) % 4] = tokens[i+4];
  }
}


void Segment::setScoresList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  if (c+1 != 2*LINcount)
    THROW("Wrong number of fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  for (size_t b = 0, d = 0; b < c; b += 2, d++)
  {
    LINdata[d].mp[0] = tokens[b];
    LINdata[d].mp[1] = tokens[b+1];
  }
}


void Segment::setBoardsList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN)
    THROW("Invalid format: " + STR(format));
  
  size_t c = countDelimiters(text, ",");
  if (c > 100)
    THROW("Too many fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c+1 != LINcount)
    THROW("Odd number of boards");

  for (unsigned i = 0; i < c; i++)
    LINdata[i].no = tokens[i];
}


void Segment::copyPlayers()
{
  if (len <= 1)
    return;

  // Also copies room.
  activeBoard->copyPlayers(boards[len-2].board);
}


void Segment::setRoom(
  const string& text,
  const Format format)
{
  activeBoard->setRoom(text, format);
}


void Segment::setNumber(
  const string& text,
  const Format format)
{
  string t = text;
  if (format == BRIDGE_FORMAT_LIN)
  {
    // Drop the open/closed indicator.
    t = t.substr(1);
    Segment::setRoom(text.substr(0, 1), format);
  }

  unsigned extNo;
  if (! str2upos(t, extNo))
    THROW("Board number is not numerical");

  if (extNo < bmin)
    bmin = extNo;
  if (extNo > bmax)
    bmax = extNo;

  boards[activeNo].extNo = extNo;
}


bool Segment::scoringIsIMPs() const
{
  return scoring.isIMPs();
}


bool Segment::hasCarry() const
{
  return teams.hasCarry();
}


bool Segment::operator == (const Segment& segment2) const
{
  if (title != segment2.title)
    DIFF("Different titles");
  else if (date != segment2.date)
    DIFF("Different dates");
  else if (location != segment2.location)
    DIFF("Different locations");
  else if (event != segment2.event)
    DIFF("Different events");
  else if (session != segment2.session)
    DIFF("Different sessions");
  else if (scoring != segment2.scoring)
    DIFF("Different scoring");
  else if (teams != segment2.teams)
    DIFF("Different teams");
  else
    return true;
}


bool Segment::operator != (const Segment& segment2) const
{
  return ! (* this == segment2);
}


string Segment::strTitleLINCore(const bool swapFlag) const
{
  stringstream ss;
  if (bmin == 0)
    ss << ",";
  else
    ss << bmin << ",";

  if (bmax == 0)
    ss << ",";
  else
    ss << bmax << ",";

  ss << teams.str(BRIDGE_FORMAT_LIN, swapFlag) << "|";
  return ss.str();
}


string Segment::strTitleLIN() const
{
  return "vg|" + title + "," +
      event + "," +
      scoring.str(BRIDGE_FORMAT_LIN) + "," +
      Segment::strTitleLINCore() + "\n";
}


string Segment::strTitleLIN_RP() const
{
  bool swapFlag = false;
  activeBoard->setInstance(0);
  if (activeBoard->room() == BRIDGE_ROOM_CLOSED)
    swapFlag = true;

  return "vg|" + title +
      session.str(BRIDGE_FORMAT_LIN_RP) + "," +
      scoring.str(BRIDGE_FORMAT_LIN) + "," +
      Segment::strTitleLINCore(swapFlag) + "pf|y|\n";
}


string Segment::strTitle(const Format format) const
{
  // In LIN this is the entire vg field.

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return Segment::strTitleLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Segment::strTitleLIN_RP();

    case BRIDGE_FORMAT_PBN:
      if (title == "")
        return "";
      else
      return "[Description \"" + title + "\"]\n";

    case BRIDGE_FORMAT_RBN:
      return "T " + title + "\n";

    case BRIDGE_FORMAT_RBX:
      return "T{" + title + "}";

    case BRIDGE_FORMAT_TXT:
      return title + "\n";

    default:
      THROW("Invalid format " + STR(format));
  }
}


string Segment::strDate(const Format format) const
{
  return date.str(format);
}


string Segment::strLocation(const Format format) const
{
  return location.str(format);
}


string Segment::strEvent(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      if (event == "")
        return "";
      THROW("No LIN event format");

    case BRIDGE_FORMAT_PBN:
      if (event == "")
        return "";
      return "[Event \"" + event + "\"]\n";

    case BRIDGE_FORMAT_RBN:
      return "E " + event + "\n";

    case BRIDGE_FORMAT_RBX:
      return "E{" + event + "}";

    case BRIDGE_FORMAT_TXT:
      return event + "\n";

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Segment::strSession(const Format format) const
{
  return session.str(format);
}


string Segment::strScoring(const Format format) const
{
  return scoring.str(format);
}


string Segment::strTeams(const Format format) const
{
  return teams.str(format);
}


string Segment::strTeams(
  const int score1,
  const int score2,
  const Format format) const
{
  return teams.str(score1, score2, format);
}


string Segment::strFirstTeam(const Format format) const
{
  return teams.strFirst(format);
}


string Segment::strSecondTeam(const Format format) const
{
  return teams.strSecond(format);
}


string Segment::strNumber(
  const unsigned intNo,
  const Format format) const
{
  unsigned extNo = Segment::getExtBoardNo(intNo);

  stringstream ss;
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      ss << "ah|Board " << extNo << "|";
      return ss.str();

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
      ss << activeBoard->strRoom(extNo, format);
      return ss.str();

    case BRIDGE_FORMAT_PBN:
      ss << "[Board \"" << extNo << "\"]\n";
      return ss.str();

    case BRIDGE_FORMAT_RBN:
      ss << "B " << extNo << "\n";
      return ss.str();

    case BRIDGE_FORMAT_RBX:
      ss << "B{" << extNo << "}";
      return ss.str();

    case BRIDGE_FORMAT_EML:
      if (! scoring.isIMPs())
        return "";
      ss << "Teams Board " << extNo;
      return ss.str();

    case BRIDGE_FORMAT_TXT:
      ss << extNo << ".";
      return ss.str();

    case BRIDGE_FORMAT_REC:
      ss << "Board " << extNo;
      return ss.str();

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Segment::strContracts(const Format format) 
{
  string st = "rs|";

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      for (auto &p: boards)
      {
        const string contractFromHeader = 
          (LINcount == 0 ? "" : LINdata[p.no].contract[1]);
        st += p.board.strContracts(contractFromHeader, format);
      }

      if (format == BRIDGE_FORMAT_LIN ||
          format == BRIDGE_FORMAT_LIN_RP || 
          format == BRIDGE_FORMAT_LIN_VG)
        st.pop_back(); // Remove trailing comma
      return st + "|\n";

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Segment::strPlayers(const Format format) 
{
  Board * board;
  Board * refBoard = nullptr;
  string st;

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      st = "pw|";
      for (auto &p: boards)
      {
        st += p.board.strPlayers(format, refBoard);
        refBoard = &p.board;
      }
      st.pop_back();
      return st + "|\n";

    case BRIDGE_FORMAT_LIN_VG:
      board = Segment::getBoard(0);
      return board->strPlayers(format);

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_TRN:
      board = Segment::getBoard(0);
      return board->strPlayers(format);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Segment::strScores(const Format format) const
{
  string st;

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      st = "mp|";
      for (auto &p: boards)
        st += p.board.strGivenScore(format);
      st.pop_back(); // Remove trailing comma
      return st + "|\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "";

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Segment::strBoards(const Format format) const
{
  stringstream ss;
  string st;

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      ss << "bn|";
      for (auto &p: boards)
        ss << p.extNo << ",";

      st = ss.str();
      st.pop_back(); // Remove trailing comma
      return st + "|\npg||\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "";

    default:
      THROW("Invalid format: " + STR(format));
  }
}

