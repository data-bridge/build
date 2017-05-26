/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <regex>

#include "Segment.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"



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
  LINPlayersListFlag = false;

  bmin = BIGNUM;
  bmax = 0;

  title = ""; 
  date.reset();
  location.reset();
  event = ""; 
  session.reset(); 
  scoring.reset();
  teams.reset();
  flagCOCO = false;
}


Board * Segment::getBoard(const unsigned intNo) 
{
  for (auto &p: boards)
  {
    if (p.no == intNo)
    {
      activeBoard = &p.board;
      activeNo = intNo;
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


unsigned Segment::getIntBoardNo(const unsigned extNo) const
{
  for (auto &p: boards)
  {
    if (p.extNo == extNo)
      return p.no;
  }
  return BIGNUM;
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


void Segment::setCOCO()
{
  flagCOCO = true;
}


bool Segment::getCOCO() const
{
  return flagCOCO;
}


void Segment::setTitleLIN(
  const string& t,
  const Format format)
{
  // We figure out which of the LIN formats is used.
  // We cater to several uses of the first three fields:
  //
  //           own table   own MP    own tourney   Vugraph    RBN-generated
  // 0         BBO         #9651 ... #9651 ...     41st...    T + S(1)
  // 1         IMPs        BBO       BBO           DOSB-...   S(2)
  // 2         P           P         I             I          I
  // scoring   I           P         I             I          I

  int seen = static_cast<int>(std::count(t.begin(), t.end(), ','));
  if (seen != 8)
    THROW("LIN vg needs exactly 8 commas.");

  vector<string> v(9);
  v.clear();
  tokenize(t, v, ",");

  // Pick out the RBN-generated line.
  regex re("^(.+)\\s+(\\w+)$");
  smatch match;
  if (format == BRIDGE_FORMAT_LIN_VG)
  {
    // Simple case.
    Segment::setTitle(v[0], BRIDGE_FORMAT_RBN);
    Segment::setEvent(v[1], BRIDGE_FORMAT_RBN);
  }
  else if (regex_search(v[0], match, re) && 
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

  // Scoring (2).  This isn't really matchpoints, could be IMP Pairs.
  if (v[2] == "P")
    Segment::setScoring("P", BRIDGE_FORMAT_LIN);
  else if (v[2] == "B" && v[1] != "IMPs")
    Segment::setScoring("B", BRIDGE_FORMAT_LIN);
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
    s << ":" << (v[6] == "" ? "0" : v[6]);
    s << ":" << (v[8] == "" ? "0" : v[8]);
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


unsigned Segment::countBoards() const
{
  unsigned cnt = 0;
  for (auto &p: boards)
  {
    if (! p.board.skippedAll())
    cnt++;
  }

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
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      Segment::setTitleLIN(text, format);
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
  if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX ||
      format == BRIDGE_FORMAT_TXT)
  {
    // TODO: Probably more formats.
    if (flagCOCO)
      teams.swap();
  }
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

bool Segment::isShortPass(const string& st) const
{
  return (st.length() == 1 && (st == "P" || st == "p"));
}


void Segment::setResultsList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN &&
      format != BRIDGE_FORMAT_LIN_RP &&
      format != BRIDGE_FORMAT_LIN_VG &&
      format != BRIDGE_FORMAT_LIN_TRN)
    THROW("Invalid format: " + STR(format));
  
  size_t c = countDelimiters(text, ",");
  if (c == 0 || c > 256)
    THROW("Bad number of fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c % 2 == 0)
  {
    if (tokens[c] == "")
      c--;
    else
    {
      // Some old lin files lack a single trailing comma.
      c++;
      tokens.push_back("");
    }
  }

  if (c+1 > 2*LINcount)
  {
    LINcount = static_cast<unsigned>((c+1)/2);
    LINdata.resize(LINcount);
  }

  for (size_t b = 0, d = 0; b < c+1; b += 2, d++)
  {
    if (Segment::isShortPass(tokens[b]))
      LINdata[d].contract[0] = "PASS";
    else
      LINdata[d].contract[0] = tokens[b];

    if (Segment::isShortPass(tokens[b+1]))
      LINdata[d].contract[1] = "PASS";
    else
      LINdata[d].contract[1] = tokens[b+1];
  }
}


void Segment::setPlayersList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN && 
      format != BRIDGE_FORMAT_LIN_VG &&
      format != BRIDGE_FORMAT_LIN_TRN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c == 7)
  {
    // Assume a single set of 8 players repeating.

    for (size_t b = 0; b < LINcount; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b].players[0][(d+2) % 4] = tokens[d];
        LINdata[b].players[1][(d+2) % 4] = tokens[d+4];
      }
    }
  }
  else if (c == 3)
  {
    // Assume a single set of 4 players repeating.
    //
    for (size_t b = 0; b < LINcount; b++)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
        LINdata[b].players[0][(d+2) % 4] = tokens[d];
    }
  }
  else if (format == BRIDGE_FORMAT_LIN_VG ||
      format == BRIDGE_FORMAT_LIN_RP)
  {
    if (format ==  BRIDGE_FORMAT_LIN_VG &&
        scoring.str(BRIDGE_FORMAT_LIN) == "P")
    {
      // Errors in some early LIN_VG files.
      if (c+2 == 8*LINcount)
      {
        tokens.push_back("");
        c++;
      }

      // Guess whether players repeat in blocks of 4 or 8.
      if (c+1 == 8*LINcount)
      {
        for (size_t b = 0; b < c; b += 8)
        {
          for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
          {
            LINdata[b >> 3].players[0][(d+2) % 4] = tokens[b+d];
            LINdata[b >> 3].players[1][(d+2) % 4] = tokens[b+d+4];
          }
        }
        LINPlayersListFlag = true;
      }
      else
      {
        if (c+5 == 4*LINcount || c+9 == 4*LINcount)
        {
          const unsigned cnt = 4*LINcount-c-1;
          for (unsigned i = 0; i < cnt; i++)
            tokens.push_back("");
          c += cnt;
        }
        else if (LINcount > 0 && c+1 > 4*LINcount)
        {
          Segment::checkPlayersTrailing(4*LINcount, c, tokens);
          c = 4*LINcount-1;
        }

        if (c+1 != 4*LINcount && (c != 4*LINcount || tokens[c] != ""))
          THROW("Wrong number of fields: " + STR(c) + " vs. " + 
            " 4*LINcount " + STR(4*LINcount));

        for (size_t b = 0; b < c; b += 4)
          for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
            LINdata[b >> 2].players[0][(d+2) % 4] = tokens[b+d];
        LINPlayersListFlag = true;
      }
    }
    else
    {
      if (c+1 != 8*LINcount)
        THROW("Wrong number of fields: " + 
          STR(c+1) + " vs. "  + STR(8*LINcount));
    
      for (size_t b = 0; b < c; b += 8)
      {
        for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
        {
          LINdata[b >> 3].players[0][(d+2) % 4] = 
            Segment::getEffectivePlayer(b, d, tokens);
          LINdata[b >> 3].players[1][(d+2) % 4] = 
            Segment::getEffectivePlayer(b, d+4, tokens);
        }
      }
      LINPlayersListFlag = true;
    }
  }
  else
  {
    if (c+1 != 4*LINcount)
      THROW("Wrong number of fields: " + STR(c) + " vs. " + 
        " 4*LINcount " + STR(4*LINcount));

    for (size_t b = 0; b < c; b += 4)
    {
      for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
      {
        LINdata[b >> 2].players[0][(d+2) % 4] = tokens[b+d];
      }
    }
    LINPlayersListFlag = true;
  }
}


string Segment::getEffectivePlayer(
  const unsigned start,
  const unsigned offset,
  const vector<string>& tokens) const
{
  // start must be a multiple of 8.
  // Search backwards for the first non-empty entry.

  for (unsigned e = 0; e <= start; e += 8)
  {
    if (tokens[(start-e)+offset] != "")
      return tokens[(start-e)+offset];
  }
  return "";
}
  

unsigned Segment::getLINActiveNo() const
{
  const unsigned eno = boards[activeNo].extNo;
  if (eno < bInmin || eno > bInmax)
    THROW("Board number out of range of LIN header");
  return(eno - bInmin);
}


unsigned Segment::getLINIntNo(const unsigned extNo) const
{
  if (extNo < bInmin || extNo > bInmax)
    THROW("Board number out of range of LIN header");
  return(extNo - bInmin);
}


void Segment::loadSpecificsFromHeader(
  const string& room,
  const Format format)
{
  if (LINcount == 0)
    return;

  if (activeBoard == nullptr)
    THROW("activeBoard == nullptr");

  int r = (room != "" && room.substr(0, 1) == "c" ? 1 : 0);

  const unsigned instNo = activeBoard->getInstance();

  if (activeNo == 0 && instNo == 0 && r == 1)
  {
    // Teams from LIN header were in "wrong" order for our internal
    // format, which is consistent with RBN:  The first team is NS
    // in the first room.  In LIN, the first team is NS in Open.
    teams.swap();
  }

  unsigned linTableNo;
  if (format == BRIDGE_FORMAT_LIN_RP)
  {
    // Consecutively without gaps.
    linTableNo = activeNo;
  }
  else
  {
    // At the right place, perhaps with gaps.
    linTableNo = Segment::getLINActiveNo();
  }

  if (LINdata[linTableNo].contract[r] != "")
    activeBoard->setContract(
      LINdata[linTableNo].contract[r], BRIDGE_FORMAT_LIN);

  string st = "";
  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
  {
    st += LINdata[activeNo].players[r][(i+2) % 4];
    if (i < 3)
      st += ",";
  }

  if (st != ",,,")
    activeBoard->setPlayers(st, BRIDGE_FORMAT_LIN, false);

  if (instNo == 0)
    activeBoard->setLINheader(LINdata[linTableNo]);
}



void Segment::setPlayersHeader(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN &&
      format != BRIDGE_FORMAT_LIN_RP &&
      format != BRIDGE_FORMAT_LIN_VG &&
      format != BRIDGE_FORMAT_LIN_TRN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c < 3)
      THROW("Bad number of fields");

  if (c > 7)
  {
    Segment::checkPlayersTrailing(8, c, tokens);
    c = 7;
  }

  if (c == 7)
  {
    for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
    {
      LINdata[0].players[0][(i+2) % 4] = tokens[i];
      LINdata[0].players[1][(i+2) % 4] = tokens[i+4];
    }
    return;
  }

  if (format ==  BRIDGE_FORMAT_LIN_VG &&
      scoring.str(BRIDGE_FORMAT_LIN) == "P")
  {
    if (c > 3)
      Segment::checkPlayersTrailing(4, c, tokens);

    for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
      LINdata[0].players[0][(i+2) % 4] = tokens[i];
  }
  else if (c < 7)
    THROW("Bad number of fields");
}


void Segment::checkPlayersTrailing(
  const unsigned first,
  const unsigned lastIncl,
  const vector<string>& tokens) const
{
  for (unsigned i = first; i <= lastIncl; i++)
    if (tokens[i] != "" &&
        tokens[i] != PLAYER_NAMES_LONG[PLAYER_LIN_TO_DDS[i % 4]])
      THROW("Bad number of fields: " + STR(first) + " vs. " + 
                " lastIncl " + STR(lastIncl));
}


void Segment::setScoresList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN &&
      format != BRIDGE_FORMAT_LIN_RP &&
      format != BRIDGE_FORMAT_LIN_VG &&
      format != BRIDGE_FORMAT_LIN_TRN)
    THROW("Invalid format: " + STR(format));

  size_t c = countDelimiters(text, ",");
  if (format ==  BRIDGE_FORMAT_LIN_VG &&
      scoring.str(BRIDGE_FORMAT_LIN) == "P")
  {
  }
  else
  {
    vector<string> tokens(c+1);
    tokens.clear();
    tokenize(text, tokens, ",");

    if (c+1 != 2*LINcount && (c != 2*LINcount || tokens[c] != ""))
      THROW("Wrong number of fields: " + STR(c+1));

    for (size_t b = 0, d = 0; b < c; b += 2, d++)
    {
      if (tokens[b] == "")
      {
        if (tokens[b+1] == "")
          LINdata[d].mp[0] = "";
        else
          LINdata[d].mp[0] = "-" + tokens[b+1];
      }
      else
        LINdata[d].mp[0] = tokens[b];
    }
  }
}


void Segment::setBoardsList(
  const string& text,
  const Format format)
{
  if (format != BRIDGE_FORMAT_LIN &&
      format != BRIDGE_FORMAT_LIN_RP &&
      format != BRIDGE_FORMAT_LIN_VG &&
      format != BRIDGE_FORMAT_LIN_TRN)
    THROW("Invalid format: " + STR(format));
  
  size_t c = countDelimiters(text, ",");
  if (c > 100)
    THROW("Too many fields");

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(text, tokens, ",");

  if (c == LINcount && tokens[c] == "")
    c--;

  if (c+1 != LINcount)
    THROW("Odd number of boards");

  for (unsigned i = 0; i <= c; i++)
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
  if (format == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_LIN_RP ||
      format == BRIDGE_FORMAT_LIN_VG ||
      format == BRIDGE_FORMAT_LIN_TRN)
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


string Segment::contractFromHeader() const
{
  if (LINcount == 0)
    return "";

  const unsigned linTableNo = Segment::getLINActiveNo();
  const unsigned instNo = boards[activeNo].board.getInstance();
  return LINdata[linTableNo].contract[instNo];
}


bool Segment::setContractInHeader(const string& text)
{
  if (LINcount == 0)
    return false;

  const unsigned linTableNo = Segment::getLINActiveNo();
  const unsigned instNo = boards[activeNo].board.getInstance();
  LINdata[linTableNo].contract[instNo] = text;
  return true;
}


unsigned Segment::getContractSeqNo() const
{
  return 2*(boards[activeNo].extNo - bInmin) + 
    boards[activeNo].board.getInstance() + 1;
}


bool Segment::scoringIsIMPs() const
{
  return scoring.isIMPs();
}


bool Segment::hasCarry() const
{
  return teams.hasCarry();
}


void Segment::getCarry(
  unsigned& score1,
  unsigned& score2) const
{
  teams.getCarry(score1, score2);
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
  else if (len != segment2.len)
    DIFF("Different board numbers");

  for (unsigned b = 0; b < len; b++)
  {
    const BoardPair& bp1 = boards[b];
    const BoardPair& bp2 = segment2.boards[b];
    if (bp1.no != bp2.no)
      DIFF("Different internal number of board");
    else if (bp1.extNo != bp2.extNo)
      DIFF("Different external number of board");
    else if (bp1.board != bp2.board)
      DIFF("Different boards"); // Board == will already throw, though
  }

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
  // bool swapFlag = false;
  // activeBoard->setInstance(0);
  // if (activeBoard->room() == BRIDGE_ROOM_CLOSED)
    // swapFlag = true;

  return "vg|" + title +
      session.str(BRIDGE_FORMAT_LIN_RP) + "," +
      scoring.str(BRIDGE_FORMAT_LIN) + "," +
      Segment::strTitleLINCore() + "pf|y|\n";
      // Segment::strTitleLINCore(swapFlag) + "pf|y|\n";
}


string Segment::strTitleLIN_VG() const
{
  if (event == "")
    return "vg|" + title + "," +
        session.str(BRIDGE_FORMAT_LIN_VG) + "," +
        scoring.str(BRIDGE_FORMAT_LIN) + "," +
        Segment::strTitleLINCore() + "\n";
  else
    return Segment::strTitleLIN();
}


string Segment::strTitle(const Format format) const
{
  // In LIN this is the entire vg field.

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      return Segment::strTitleLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Segment::strTitleLIN_RP();

    case BRIDGE_FORMAT_LIN_VG:
      return Segment::strTitleLIN_VG();

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

    case BRIDGE_FORMAT_PAR:
      return title;

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
      else
        return "[Event \"" + event + "\"]\n";

    case BRIDGE_FORMAT_RBN:
      return "E " + event + "\n";

    case BRIDGE_FORMAT_RBX:
      return "E{" + event + "}";

    case BRIDGE_FORMAT_TXT:
      return event + "\n";

    case BRIDGE_FORMAT_PAR:
      return event;

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
  if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
    // Maybe more formats.
    return teams.str(format, flagCOCO);
  else
    return teams.str(format);
}


string Segment::strTeams(
  const int score1,
  const int score2,
  const Format format,
  const bool swapFlag) const
{
  return teams.str(score1, score2, format, swapFlag);
}


string Segment::strFirstTeam(
  const Format format,
  const bool swapFlag) const
{
  return teams.strFirst(format, swapFlag);
}


string Segment::strSecondTeam(
  const Format format,
  const bool swapFlag) const
{
  return teams.strSecond(format, swapFlag);
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
      if (format == BRIDGE_FORMAT_LIN_TRN || LINcount == 0)
        ss << "ah|Board " << extNo << "|";
      else
        ss << "ah|Board " << LINdata[intNo].no << "|";
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


string Segment::strContractsCore(const Format format) 
{
  string st = "";

  if (format == BRIDGE_FORMAT_PAR)
  {
    if (LINcount == 0)
      return "";

    for (unsigned b = 0; b < LINcount; b++)
      st += LINdata[b].contract[0] + "," + LINdata[b].contract[1] + ",";
    st.pop_back(); // Remove trailing comma
    return st;
  }
  else if (LINcount == 0)
  {
    // This is not quite the same as below.
    // Say we have PBN 980, 982, 983, ...
    // The skipped board should not (currently) lead to a LIN gap.
    for (auto &p: boards)
      st += p.board.strContracts("", format);
  }
  else
  {
    for (unsigned b = bInmin; b <= bInmax; b++)
    {
      const unsigned bint = Segment::getLINIntNo(b);
      const Board * bptr = Segment::getBoard(bint);
      if (bptr == nullptr)
      {
        st += LINdata[bint].contract[0] + "," + 
              LINdata[bint].contract[1] + ",";
      }
      else
      {
        st += bptr->strContract(0, format) + "," +
              bptr->strContract(1, format) + ",";
      }
    }
  }

  st.pop_back(); // Remove trailing comma
  return st;

}


string Segment::strContracts(const Format format) 
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "rs|" + Segment::strContractsCore(format) + "|\n";

    case BRIDGE_FORMAT_PAR:
      return Segment::strContractsCore(BRIDGE_FORMAT_PAR);

    default:
      THROW("Invalid format: " + STR(format));
  }
}


string Segment::strPlayersLIN()
{
  Board * refBoard = nullptr;
  string st = "pw|";
  const bool isIMPs = scoring.isIMPs();
  if (LINcount == 0)
  {
    for (auto &p: boards)
    {
      // This is not quite the same as below.
      // Say we have PBN 980, 982, 983, ...
      // The skipped board should not (currently) lead to a LIN gap.
      st += p.board.strPlayers(BRIDGE_FORMAT_LIN, isIMPs, refBoard);
      refBoard = &p.board;
    }
  }
  else
  {
    for (unsigned b = bInmin; b <= bInmax; b++)
    {
      const unsigned bint = Segment::getLINIntNo(b);
      Board * bptr = Segment::getBoard(bint);
      if (bptr)
      {
        st += bptr->strPlayers(BRIDGE_FORMAT_LIN, isIMPs, refBoard);
        refBoard = bptr;
      }
      else if (isIMPs)
        st += ",,,,,,,,";
      else
        st += ",,,,";
    }
  }

  st.pop_back();
  return st + "|\n";
}


bool Segment::playersAreUnique()
{
  Board * refBoard = nullptr;
  string pprev, pnew = "";
  for (auto &p: boards)
  {
    pprev = pnew;
    pnew = p.board.strPlayers(BRIDGE_FORMAT_LIN_VG, true, refBoard);
    if (refBoard != nullptr && pprev != pnew)
      return false;
  }
  return true;
}


string Segment::strPlayers(const Format format) 
{
  Board * board;

  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::strPlayersLIN();

    case BRIDGE_FORMAT_LIN_VG:
      if (LINPlayersListFlag ||
          ((scoring.str(BRIDGE_FORMAT_LIN) == "P" ||
          scoring.str(BRIDGE_FORMAT_LIN) == "B") &&
          ! Segment::playersAreUnique()))
      {
        return Segment::strPlayersLIN();
      }
      else
      {
        board = Segment::getBoard(0);
        return board->strPlayers(format);
      }

    case BRIDGE_FORMAT_LIN_RP:
      board = Segment::getBoard(0);
      return board->strPlayers(format);

    case BRIDGE_FORMAT_LIN_TRN:
      board = Segment::getBoard(0);
      return board->strPlayers(BRIDGE_FORMAT_LIN_VG);

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
      if (LINcount == 0)
      {
        for (auto &p: boards)
          ss << p.extNo << ",";
      }
      else
      {
        for (auto &p: boards)
          ss << LINdata[p.no].no << ",";
      }

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


string Segment::strIMPSheetHeader() const
{
  stringstream ss;
  const string divider = "  |  ";
  const string dashes(72, '-');
  ss << setw(4) << right << "Bd." << "  " <<
    setw(8) << left << "Contr." <<
    setw(7) << right << "NS" <<
    setw(7) << "EW" <<
    divider <<
    setw(8) << left << "Contr." <<
    setw(7) << right << "NS" <<
    setw(7) << "EW" <<
    divider <<
    setw(6) << "Home" <<
    setw(6) << "Visit" << "\n";
  ss << dashes << "\n";
  return ss.str();
}


string Segment::strIMPSheetFooter(
  const unsigned score1,
  const unsigned score2) const
{
  stringstream ss;
  const string dashes(72, '-');
  ss << dashes << "\n";
  ss << setw(66) << right << score1 <<
    setw(6) << score2 << "\n\n";
  return ss.str();  
}

