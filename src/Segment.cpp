/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <string>
#include <regex>
#include "Segment.h"
#include "portab.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


#define BIG_BOARD 99999


Segment::Segment()
{
  Segment::Reset();
}


Segment::~Segment()
{
}


void Segment::Reset()
{
  len = 0;
  boards.clear();
  LINcount = 0;
  LINdata.clear();

  bmin = BIG_BOARD;
  bmax = 0;

  seg.title = ""; 
  seg.date.reset();
  seg.location.reset();
  seg.event = ""; 
  seg.session.reset(); 
  seg.scoring.reset();
  seg.teams.reset();
}


Board * Segment::GetBoard(const unsigned no) 
{
  for (auto &p: boards)
  {
    if (p.no == no)
    {
      activeBoard = &p.board;
      return &p.board;
    }
  }
  return nullptr;
}


unsigned Segment::GetExtBoardNo(const unsigned no) const
{
  for (auto &p: boards)
  {
    if (p.no == no)
      return p.extNo;
  }
  return 0;
}


unsigned Segment::GetActiveExtBoardNo() const
{
  return Segment::GetExtBoardNo(activeNo);
}


Board * Segment::AcquireBoard(const unsigned intNo)
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

  return activeBoard;
}


void Segment::TransferHeader(const unsigned intNo)
{
  if (LINcount == 0)
    return;

  Board * board = Segment::AcquireBoard(intNo);
  if (board == nullptr)
    return;

  board->setLINheader(LINdata[intNo]);
}


void Segment::TransferHeader(
  const unsigned intNo,
  const unsigned instNo)
{
  if (LINcount == 0)
    return;

  Board * board = Segment::AcquireBoard(intNo);
  if (board == nullptr)
    return;

  board->setInstance(instNo);

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    if (LINdata[intNo].players[instNo][p] != "")
      board->setPlayer(LINdata[intNo].players[instNo][p],
        static_cast<playerType>(p));
  }

  if (instNo == 0)
    board->setLINheader(LINdata[intNo]);
}


void Segment::SetTitleLIN(const string& t)
{
  // We figure out which of the LIN formats is used.
  //
  // We cater to several uses of the first three fields:
  //
  //           own table   own MP    own tourney   Vugraph    RBN-generated
  // 0         BBO         #9651 ... #9651 ...     41st...    T + S(1)
  // 1         IMPs        BBO       BBO           DOSB-...   S(2)
  // 2         P           P         I             I          I
  // scoring   I           P         I             I          I
  //
  // Field #0 may have the form text%date%location%session
  // when we generate it.  Otherwise we lose RBN information.
  //
  // Fields 3 and 4 are board ranges (ignored and re-generated).
  // Fields 5 through 8 are team tags.

  int seen = std::count(t.begin(), t.end(), ',');
  if (seen != 8)
    THROW("LIN vg needs exactly 8 commas.");

  vector<string> v(9);
  v.clear();
  tokenize(t, v, ",");

  // Try to pick out the RBN-generated line.
  bool eventFlag = true;
  regex re1("^(.+)\\s+(\\w+)$");
  regex re2("^Segment\\s+\\d+$");
  regex re3("^Round\\s+\\d+$");
  regex re4("^R(\\d+)$");
  regex re5("^R(\\d+)[ABCD]$");
  regex re6("^R(\\d+) [ABCD]$");
  regex re7("^[QS][ABCD]$");
  regex re8("^Segment\\s+\\d+ \\(overtime\\)$");
  // This has become a mess.  It should be tested in Session.cpp
  smatch match1, match2;
  if (regex_search(v[0], match1, re1) && 
      match1.size() >= 2 &&
      seg.session.isRBNPart(match1.str(2)) &&
      (regex_search(v[1], match2, re2) ||
       regex_search(v[1], match2, re3) ||
       regex_search(v[1], match2, re8) ||
       v[1] == "Overtime"))
  {
    Segment::setTitle(match1.str(1), BRIDGE_FORMAT_RBN);

    // Make a synthetic RBN-like session line (a bit wasteful).
    stringstream s;
    s << match1.str(2) << ":" << v[1];
    Segment::setSession(s.str(), BRIDGE_FORMAT_RBN);
    seg.event = "";
    eventFlag = false;
  }
  else if (regex_search(v[1], match2, re4) ||
           regex_search(v[1], match2, re5) ||
           regex_search(v[1], match2, re6) ||
           regex_search(v[1], match2, re7))
  {
    Segment::setTitle(v[0], BRIDGE_FORMAT_RBN);
    Segment::setSession(v[1], BRIDGE_FORMAT_RBN);
    seg.event = "";
    eventFlag = false;
  }

  // See whether the title line contains extra information.
  seen = std::count(t.begin(), t.end(), '%');
  if (seen == 3)
  {
    vector<string> vv(4);
    vv.clear();
    tokenize(v[0], vv, "%");

    Segment::setTitle(vv[0], BRIDGE_FORMAT_RBN);
    Segment::setDate(vv[1], BRIDGE_FORMAT_RBN);
    Segment::setLocation(vv[2], BRIDGE_FORMAT_RBN);
    Segment::setSession(vv[3], BRIDGE_FORMAT_RBN);
    Segment::setEvent(v[1], BRIDGE_FORMAT_RBN);
  }
  else if (eventFlag)
  {
    Segment::setTitle(v[0], BRIDGE_FORMAT_RBN);
    Segment::setEvent(v[1], BRIDGE_FORMAT_RBN);
  }


  if (v[2] == "P" && v[1] != "IMPs")
    Segment::setScoring("P", BRIDGE_FORMAT_LIN);
  else
    Segment::setScoring("I", BRIDGE_FORMAT_LIN);
    
  if (v[3] == "")
    bInmin = 0;
  else if (! str2upos(v[3], bInmin))
    THROW("Not a board number");

  if (v[4] == "")
    bInmax = 0;
  else if (! StringToNonzeroUnsigned(v[4], bInmax))
    THROW("Not a board number");

  // Synthesize an RBN-like team line (a bit wasteful).
  stringstream s;
  s << v[5] << ":" << v[7];
  if (v[6] != "" || v[8] != "")
  {
    s << ":" << (v[6] == "" ? 0 : v[6]);
    s << ":" << (v[8] == "" ? 0 : v[8]);
  }

  Segment::setTeams(s.str(), BRIDGE_FORMAT_LIN);
}


unsigned Segment::GetLength() const
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
      Segment::SetTitleLIN(text);
      break;

    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
    case BRIDGE_FORMAT_TXT:
      seg.title = text;
      break;

    default:
      THROW("Invalid format: " + STR(format));
  }
}


void Segment::setDate(
  const string& text,
  const Format format)
{
  seg.date.set(text, format);
}


void Segment::setLocation(
  const string& text,
  const Format format)
{
  seg.location.set(text, format);
}


void Segment::setEvent(
  const string& text,
  const Format format)
{
  UNUSED(format);
  seg.event = text;
}


void Segment::setSession(
  const string& text,
  const Format format)
{
  seg.session.set(text, format);
}


void Segment::setScoring(
  const string& text,
  const Format format)
{
  seg.scoring.set(text, format);
}


void Segment::setTeams(
  const string& text,
  const Format format)
{
  seg.teams.set(text, format); 
}


void Segment::setFirstTeam(
  const string& text,
  const Format format)
{
  seg.teams.setFirst(text, format); 
}


void Segment::setSecondTeam(
  const string& text,
  const Format format)
{
  seg.teams.setSecond(text, format); 
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


void Segment::SetFromHeader(const string& room)
{
  if (LINcount == 0)
    return;

  if (activeBoard == nullptr)
    return;

  int r = (room != "" && room.substr(0, 1) == "c" ? 1 : 0);
  const formatType f = BRIDGE_FORMAT_LIN;

  activeBoard->setContract(LINdata[activeNo].contract[r], f);

  string s = "";
  // unsigned r2 = activeBoard->GetInstance();
  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
  {
    s += LINdata[activeNo].players[r][(i+2) % 4];
    if (i < 3)
      s += ",";
  }

  if (activeNo == 0 &&
      activeBoard->getInstance() == 0 &&
      r == 1)
  {
    // Teams from LIN header were in "wrong" order for our internal
    // format, which is consistent with RBN:  The first team is NS
    // in the first room.  In LIN, the first team is NS in Open.
    seg.teams.swap();
  }

  if (s != ",,,")
    activeBoard->setPlayers(s, f);
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
  {
    LINdata[i].no = tokens[i];
  }
}


void Segment::CopyPlayers()
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


bool Segment::ScoringIsIMPs() const
{
  return seg.scoring.isIMPs();
}


bool Segment::CarryExists() const
{
  return seg.teams.hasCarry();
}


bool Segment::operator == (const Segment& s2) const
{
  if (seg.title != s2.seg.title)
    DIFF("Different titles");
  else if (seg.date != s2.seg.date)
    DIFF("Different dates");
  else if (seg.location != s2.seg.location)
    DIFF("Different locations");
  else if (seg.event != s2.seg.event)
    DIFF("Different events");
  else if (seg.session != s2.seg.session)
    DIFF("Different sessions");
  else if (seg.scoring != s2.seg.scoring)
    DIFF("Different scoring");
  else if (seg.teams != s2.seg.teams)
    DIFF("Different teams");
  else
    return true;
}


bool Segment::operator != (const Segment& s2) const
{
  return ! (* this == s2);
}


string Segment::TitleAsLINCommon(const bool swapFlag) const
{
  stringstream s;
  if (bmin == 0)
    s << ",";
  else
    s << bmin << ",";

  if (bmax == 0)
    s << ",";
  else
    s << bmax << ",";

  s << seg.teams.str(BRIDGE_FORMAT_LIN, swapFlag) << "|";
  return s.str();
}


string Segment::TitleAsLIN() const
{
  stringstream s;
  s << "vg|" << seg.title << "," << 
      seg.event << "," <<
      seg.scoring.str(BRIDGE_FORMAT_LIN) << ",";
  s << Segment::TitleAsLINCommon() << "\n";
  return s.str();
}


string Segment::TitleAsLIN_EXT() const
{
  // BBO hands played at own table (not tournaments).
  stringstream s;
  s << seg.title << "%" <<
      seg.date.str(BRIDGE_FORMAT_LIN) << "%" <<
      seg.location.str(BRIDGE_FORMAT_LIN) << "%" <<
      seg.session.asString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.event << "%" <<
      seg.scoring.str(BRIDGE_FORMAT_LIN) << 
      ",IMPs,P,";
  s << Segment::TitleAsLINCommon();
  return s.str() + "\n";
}


string Segment::TitleAsLIN_RP() const
{
  // BBO hands from Pavlicek.

  bool swapFlag = false;
  activeBoard->setInstance(0);
  if (activeBoard->room() == BRIDGE_ROOM_CLOSED)
    swapFlag = true;

  stringstream s;
  s << "vg|" << seg.title << 
      seg.session.asString(BRIDGE_FORMAT_LIN_RP) << "," <<
      seg.scoring.str(BRIDGE_FORMAT_LIN) << ",";
  s << Segment::TitleAsLINCommon(swapFlag) << "pf|y|\n";
  return s.str();
}


string Segment::TitleAsLIN_VG() const
{
  // BBO hands from Vugraph.
  stringstream s;
  s << "vg|" << seg.title << ",I,I,";

  s << Segment::TitleAsLINCommon();
  return s.str() + "\n";
}


string Segment::TitleAsLIN_TRN() const
{
  // BBO hands played in own tournaments.
  stringstream s;
  s << seg.title << "%" <<
      seg.date.str(BRIDGE_FORMAT_LIN) << "%" <<
      seg.location.str(BRIDGE_FORMAT_LIN) << "%" <<
      seg.session.asString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.event << "%" <<
      seg.scoring.str(BRIDGE_FORMAT_LIN) << 
      ",IMPs,P,";
  s << Segment::TitleAsLINCommon();
  return s.str() + "\n";
}


string Segment::TitleAsString(const formatType f) const
{
  // In LIN this is the entire vg field.

  stringstream st;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
    case BRIDGE_FORMAT_LIN_VG:
      return Segment::TitleAsLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Segment::TitleAsLIN_RP();

    case BRIDGE_FORMAT_PBN:
      if (seg.title == "")
        return "";
      st << "[Description \"" << seg.title << "\"]\n";
      return st.str();

    case BRIDGE_FORMAT_RBN:
      st << "T " << seg.title << "\n";
      return st.str();

    case BRIDGE_FORMAT_RBX:
      st << "T{" << seg.title << "}";
      return st.str();

    case BRIDGE_FORMAT_TXT:
      return seg.title + "\n";

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Segment::DateAsString(const formatType f) const
{
  return seg.date.str(f);
}


string Segment::LocationAsString(const formatType f) const
{
  return seg.location.str(f);
}


string Segment::EventAsString(const formatType f) const
{
  stringstream st;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      if (seg.event == "")
        return "";
      THROW("No LIN event format");

    case BRIDGE_FORMAT_PBN:
      if (seg.event == "")
        return "";
      st << "[Event \"" + seg.event + "\"]\n";
      return st.str();

    case BRIDGE_FORMAT_RBN:
      st << "E " << seg.event << "\n";
      return st.str();

    case BRIDGE_FORMAT_RBX:
      st << "E{" << seg.event << "}";
      return st.str();

    case BRIDGE_FORMAT_TXT:
      return seg.event + "\n";

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Segment::SessionAsString(
  const formatType f) const
{
  return seg.session.asString(f);
}


string Segment::ScoringAsString(
  const formatType f) const
{
  return seg.scoring.str(f);
}


string Segment::TeamsAsString(
  const formatType f) const
{
  return seg.teams.str(f);
}


string Segment::TeamsAsString(
  const int score1,
  const int score2,
  const formatType f) const
{
  return seg.teams.str(f, score1, score2);
}


string Segment::FirstTeamAsString(
  const formatType f) const
{
  return seg.teams.strFirst(f);
}


string Segment::SecondTeamAsString(
  const formatType f) const
{
  return seg.teams.strSecond(f);
}


string Segment::NumberAsString(
  const formatType f,
  const unsigned intNo) const
{
  unsigned extNo = Segment::GetExtBoardNo(intNo);
  if (extNo == 0)
    return "";

  stringstream st;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      st << activeBoard->strRoom(extNo, f);
      return st.str();

    case BRIDGE_FORMAT_PBN:
      st << "[Board \"" << extNo << "\"]\n";
      return st.str();

    case BRIDGE_FORMAT_RBN:
      st << "B " << extNo << "\n";
      return st.str();

    case BRIDGE_FORMAT_RBX:
      st << "B{" << extNo << "}";
      return st.str();

    case BRIDGE_FORMAT_EML:
      if (! seg.scoring.isIMPs())
        return "";
      st << "Teams Board " << extNo;
      return st.str();

    case BRIDGE_FORMAT_TXT:
      st << extNo << ".";
      return st.str();

    case BRIDGE_FORMAT_REC:
      st << "Board " << extNo;
      return st.str();

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Segment::NumberAsBoardString(
  const formatType f,
  const unsigned intNo) const
{
  if (f != BRIDGE_FORMAT_LIN && f != BRIDGE_FORMAT_LIN_TRN)
    return "";

  unsigned extNo = Segment::GetExtBoardNo(intNo);
  if (extNo == 0)
    return "";
  
  stringstream st;
  st << "ah|Board " << extNo << "|";
  return st.str();
}


string Segment::ContractsAsLIN(const formatType f)
{
  stringstream s;
  s << "rs|";
  for (auto &p: boards)
  {
    const unsigned l = p.board.count();
    p.board.setInstance(0);
    if (l == 2 && 
        p.board.room() == BRIDGE_ROOM_CLOSED)
        // activeBoard->GetRoom() == BRIDGE_ROOM_CLOSED)
    {
      p.board.setInstance(1);
      s << p.board.strContract(f) << ",";
      p.board.setInstance(0);
      s << p.board.strContract(f) << ",";
    }
    else
    {
      for (unsigned i = 0; i < l; i++)
      {
        p.board.setInstance(i);
        s << p.board.strContract(f) << ",";
      }
    }

    if (l == 1)
    {
      if (LINcount == 0)
        s << ",";
      else
        // Occurs in Vugraph pairs tournaments.
        s << LINdata[p.no].contract[1] << ",";
    }
  }

  string st = s.str();
  if (f == BRIDGE_FORMAT_LIN ||
      f == BRIDGE_FORMAT_LIN_RP || 
      f == BRIDGE_FORMAT_LIN_VG)
    st.pop_back(); // Remove trailing comma
  return st + "|\n";
}


string Segment::ContractsAsString(const formatType f) 
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return Segment::ContractsAsLIN(f);

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Segment::PlayersAsString(const formatType f) 
{
  string s1, s2;
  Board * board;
  Board * refBoard = nullptr;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      s1 = "pw|";
      for (auto &p: boards)
      {
        for (unsigned i = 0; i < p.board.count(); i++)
        {
          p.board.setInstance(i);
          s1 += p.board.PlayersAsDeltaString(refBoard, f);
          refBoard = &p.board;
        }
      }
      s1.pop_back();
      return s1 + "|\n";

    case BRIDGE_FORMAT_LIN_VG:
      board = Segment::GetBoard(0);
      if (board == nullptr)
        return "";

      if (board->count() == 1)
      {
        board->setInstance(0);
        s1 = board->PlayersAsString(f);
        return "pn|" + s1 + ",South,West,North,East|pg||\n";
      }

      board->setInstance(0);
      s1 = board->PlayersAsString(f);
      board->setInstance(1);
      s2 = board->PlayersAsString(f);

      if (f == BRIDGE_FORMAT_LIN_TRN)
      {
        s1.pop_back(); // Trailing |, leading pn|,,,,
        return s1 + "," + s2.substr(7) + "\npg||\n";
      }
      else
        return "pn|" + s1 + "," + s2 + "|pg||\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_TRN:
      board = Segment::GetBoard(0);
      if (board == nullptr || board->count() != 2)
        return "";

      // if (activeBoard->GetRoom() == BRIDGE_ROOM_CLOSED)
      board->setInstance(0);
      if (board->room() == BRIDGE_ROOM_CLOSED)
      {
        board->setInstance(1);
        s1 = board->PlayersAsString(f);
        board->setInstance(0);
        s2 = board->PlayersAsString(f);
      }
      else
      {
        board->setInstance(0);
        s1 = board->PlayersAsString(f);
        board->setInstance(1);
        s2 = board->PlayersAsString(f);
      }

      if (f == BRIDGE_FORMAT_LIN_TRN)
      {
        s1.pop_back(); // Trailing |, leading pn|,,,,
        return s1 + "," + s2.substr(7) + "\npg||\n";
      }
      else
        return "pn|" + s1 + "," + s2 + "|pg||\n\n";

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Segment::ScoresAsString(const formatType f) const
{
  string s;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      s = "mp|";
      for (auto &p: boards)
        s += p.board.GivenScoreAsString(f);
      s.pop_back(); // Remove trailing comma
      return s + "|\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "";

    default:
      THROW("Invalid format " + STR(f));
  }
}


string Segment::BoardsAsString(const formatType f) const
{
  stringstream s;
  string st;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      s << "bn|";
      for (auto &p: boards)
        s << p.extNo << ",";

      st = s.str();
      st.pop_back(); // Remove trailing comma
      return st + "|\npg||\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "";

    default:
      THROW("Invalid format " + STR(f));
  }
}

