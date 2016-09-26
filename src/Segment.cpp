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
#include "debug.h"

extern Debug debug;

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
  seg.date.Reset();
  seg.location.Reset();
  seg.event = ""; 
  seg.session.Reset(); 
  seg.scoring.Reset();
  seg.teams.Reset();
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

  board->SetLINheader(LINdata[intNo]);
}


void Segment::TransferPlayers(
  const unsigned intNo,
  const unsigned instNo)
{
  if (LINcount == 0)
    return;

  Board * board = Segment::AcquireBoard(intNo);
  if (board == nullptr)
    return;

  if (! board->SetInstance(instNo))
    return;

  for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
  {
    if (LINdata[intNo].players[instNo][p] != "")
      board->SetPlayer(LINdata[intNo].players[instNo][p],
        static_cast<playerType>(p));
  }
}


bool Segment::SetTitleLIN(const string t)
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

  int seen = count(t.begin(), t.end(), ',');
  if (seen != 8)
  {
    LOG("LIN vg needs exactly 8 commas.");
    return false;
  }

  vector<string> v(9);
  v.clear();
  tokenize(t, v, ",");

  // Try to pick out the RBN-generated line.
  bool eventFlag = true;
  regex re("^(.+)\\s+(\\w+)$");
  smatch match;
  if (regex_search(v[0], match, re) && 
      match.size() >= 2 &&
      seg.session.IsRBNPart(match.str(2)))
  {
    if (! Segment::SetTitle(match.str(1), BRIDGE_FORMAT_RBN))
      return false;

    // Make a synthetic RBN-like session line (a bit wasteful).
    stringstream s;
    s << match.str(2) << ":" << v[1];
    if (! Segment::SetSession(s.str(), BRIDGE_FORMAT_RBN))
      return false;
    seg.event = "";
    eventFlag = false;
  }

  // See whether the title line contains extra information.
  seen = count(t.begin(), t.end(), '%');
  if (seen == 3)
  {
    vector<string> vv(4);
    vv.clear();
    tokenize(v[0], vv, "%");
    if (! Segment::SetTitle(vv[0], BRIDGE_FORMAT_RBN))
      return false;

    if (! Segment::SetDate(vv[1], BRIDGE_FORMAT_RBN))
      return false;

    if (! Segment::SetLocation(vv[2], BRIDGE_FORMAT_RBN))
      return false;

    if (! Segment::SetSession(vv[3], BRIDGE_FORMAT_RBN))
      return false;

    if (! Segment::SetEvent(v[1], BRIDGE_FORMAT_RBN))
      return false;
  }
  else if (eventFlag)
  {
    if (! Segment::SetTitle(v[0], BRIDGE_FORMAT_RBN))
      return false;

    if (! Segment::SetEvent(v[1], BRIDGE_FORMAT_RBN))
      return false;
  }


  if (v[2] == "P" && v[1] != "IMPs")
    Segment::SetScoring("P", BRIDGE_FORMAT_LIN);
  else
    Segment::SetScoring("I", BRIDGE_FORMAT_LIN);
    
  if (v[3] == "")
    bInmin = 0;
  else if (! StringToUnsigned(v[3], bInmin))
  {
    LOG("Not a board number");
    return false;
  }

  if (v[4] == "")
    bInmax = 0;
  else if (! StringToUnsigned(v[4], bInmax))
  {
    LOG("Not a board number");
    return false;
  }

  // Synthesize an RBN-like team line (a bit wasteful).
  stringstream s;
  s << v[5] << ":" << v[7];
  if (v[6] != "" || v[8] != "")
  {
    s << ":" << (v[6] == "" ? 0 : v[6]);
    s << ":" << (v[8] == "" ? 0 : v[8]);
  }

  if (! Segment::SetTeams(s.str(), BRIDGE_FORMAT_LIN))
    return false;

  return true;
}


unsigned Segment::GetLength() const
{
  return len;
}


bool Segment::SetTitle(
  const string& t,
  const formatType f)
{
  // In LIN this includes the entire vg line.
  
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::SetTitleLIN(t);

    case BRIDGE_FORMAT_PBN:
      seg.title = t;
      return true;

    case BRIDGE_FORMAT_RBN:
      seg.title = t;
      return true;

    case BRIDGE_FORMAT_TXT:
      seg.title = t;
      return true;

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


bool Segment::SetDate(
  const string& t,
  const formatType f)
{
  return seg.date.Set(t, f);
}


bool Segment::SetLocation(
  const string& t,
  const formatType f)
{
  return seg.location.Set(t, f);
}


bool Segment::SetEvent(
  const string& t,
  const formatType f)
{
  UNUSED(f);
  seg.event = t;
  return true;
}


bool Segment::SetSession(
  const string& t,
  const formatType f)
{
  return seg.session.Set(t, f);
}


bool Segment::SetScoring(
  const string& t,
  const formatType f)
{
  return seg.scoring.Set(t, f);
}


bool Segment::SetTeams(
  const string& s,
  const formatType f)
{
  return seg.teams.Set(s, f); 
}


bool Segment::SetFirstTeam(
  const string& s,
  const formatType f)
{
  return seg.teams.SetFirst(s, f); 
}


bool Segment::SetSecondTeam(
  const string& s,
  const formatType f)
{
  return seg.teams.SetSecond(s, f); 
}


bool Segment::SetPlayers(
  const string& s,
  const formatType f)
{
  return activeBoard->SetPlayers(s, f);
}


bool Segment::SetWest(
  const string& s,
  const formatType f)
{
  UNUSED(f);
  return activeBoard->SetPlayer(s, BRIDGE_WEST);
}


bool Segment::SetNorth(
  const string& s,
  const formatType f)
{
  UNUSED(f);
  return activeBoard->SetPlayer(s, BRIDGE_NORTH);
}


bool Segment::SetEast(
  const string& s,
  const formatType f)
{
  UNUSED(f);
  return activeBoard->SetPlayer(s, BRIDGE_EAST);
}


bool Segment::SetSouth(
  const string& s,
  const formatType f)
{
  UNUSED(f);
  return activeBoard->SetPlayer(s, BRIDGE_SOUTH);
}


bool Segment::SetResultsList(
  const string& s,
  const formatType f)
{
  if (f != BRIDGE_FORMAT_LIN)
    return false;
  
  size_t c = countDelimiters(s, ",");
  if (c > 100)
  {
    LOG("Too many fields");
    return false;
  }

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(s, tokens, ",");

  // There may or may not be a trailing comma
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
  return true;
}


bool Segment::SetPlayersList(
  const string& s,
  const formatType f)
{
  if (f != BRIDGE_FORMAT_LIN)
    return false;

  size_t c = countDelimiters(s, ",");
  if (c != 4*LINcount)
  {
    LOG("Wrong number of fields");
    return false;
  }

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(s, tokens, ",");

  for (size_t b = 0; b < c; b += 8)
  {
    for (unsigned d = 0; d < BRIDGE_PLAYERS; d++)
    {
      LINdata[b >> 3].players[0][(d+2) % 4] = tokens[b+d];
      LINdata[b >> 3].players[1][(d+2) % 4] = tokens[b+d+4];
    }
  }
  return true;
}


bool Segment::SetPlayersHeader(
  const string& s,
  const formatType f)
{
  if (f != BRIDGE_FORMAT_LIN)
    return false;

  size_t c = countDelimiters(s, ",");
  if (c != 7)
  {
    LOG("Bad number of fields");
    return false;
  }

  vector<string> tokens(8);
  tokens.clear();
  tokenize(s, tokens, ",");

  for (unsigned i = 0; i < BRIDGE_PLAYERS; i++)
  {
    LINdata[0].players[0][(i+2) % 4] = tokens[i];
    LINdata[0].players[1][(i+2) % 4] = tokens[i+4];
  }
  return true;
}


bool Segment::SetScoresList(
  const string& s,
  const formatType f)
{
  if (f != BRIDGE_FORMAT_LIN)
    return false;

  size_t c = countDelimiters(s, ",");
  if (c != 2*LINcount)
  {
    LOG("Wrong number of fields");
    return false;
  }

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(s, tokens, ",");

  for (size_t b = 0, d = 0; b < c; b += 2, d++)
  {
    LINdata[d].mp[0] = tokens[b];
    LINdata[d].mp[1] = tokens[b+1];
  }
  return true;
}


bool Segment::SetBoardsList(
  const string& s,
  const formatType f)
{
  if (f != BRIDGE_FORMAT_LIN)
    return false;
  
  size_t c = countDelimiters(s, ",");
  if (c > 100)
  {
    LOG("Too many fields");
    return false;
  }

  vector<string> tokens(c+1);
  tokens.clear();
  tokenize(s, tokens, ",");

  if (c != LINcount)
  {
    LOG("Odd number of boards");
    return false;
  }

  for (unsigned i = 0; i < c; i++)
  {
    LINdata[i].no = tokens[i];
  }

  return true;
}


void Segment::CopyPlayers()
{
  if (len <= 1)
    return;

  unsigned inst = activeBoard->GetInstance();
  if (! boards[len-2].board.PlayersAreSet(inst))
    return;
  else
    activeBoard->CopyPlayers(boards[len-2].board, inst);
}


bool Segment::SetRoom(
  const string& s,
  const formatType f)
{
  unsigned inst = activeBoard->GetInstance();
  return activeBoard->SetRoom(s, inst, f);
}


bool Segment::SetNumber(
  const string& s,
  const formatType f)
{
  string t = s;
  if (f == BRIDGE_FORMAT_LIN)
  {
    // Drop the open/closed indicator.
    t = t.substr(1);

    Segment::SetRoom(s.substr(0, 1), f);
  }

  unsigned extNo;
  if (! StringToUnsigned(t, extNo))
  {
    LOG("Board number is not numerical");
    return false;
  }

  if (extNo < bmin)
    bmin = extNo;
  if (extNo > bmax)
    bmax = extNo;

  boards[activeNo].extNo = extNo;
  return true;
}


bool Segment::ScoringIsIMPs() const
{
  return seg.scoring.ScoringIsIMPs();
}


bool Segment::operator == (const Segment& s2) const
{
  if (seg.title != s2.seg.title)
  {
    LOG("Different titles");
    return false;
  }
  else if (seg.date != s2.seg.date)
    return false;
  else if (seg.location != s2.seg.location)
  {
    LOG("Different locations");
    return false;
  }
  else if (seg.event != s2.seg.event)
  {
    LOG("Different events");
    return false;
  }
  else if (seg.session != s2.seg.session)
  {
    LOG("Different sessions");
    return false;
  }
  else if (seg.scoring != s2.seg.scoring)
  {
    LOG("Different scoring");
    return false;
  }
  else if (seg.teams != s2.seg.teams)
    return false;
  else
    return true;
}


bool Segment::operator != (const Segment& s2) const
{
  return ! (* this == s2);
}


string Segment::TitleAsLINCommon() const
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

  s << seg.teams.AsString(BRIDGE_FORMAT_LIN) << "|";
  return s.str();
}


string Segment::TitleAsLIN() const
{
  // BBO hands played at own table (not tournaments).
  stringstream s;
  s << seg.title << "%" <<
      seg.date.AsString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.location.AsString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.session.AsString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.event << "%" <<
      seg.scoring.AsString(BRIDGE_FORMAT_LIN) << 
      ",IMPs,P,";
  s << Segment::TitleAsLINCommon();
  return s.str() + "\n";
}


string Segment::TitleAsLIN_RP() const
{
  // BBO hands from Pavlicek.
  stringstream s;
  s << "vg|" << seg.title << 
      seg.session.AsString(BRIDGE_FORMAT_LIN_RP) << "," <<
      seg.scoring.AsString(BRIDGE_FORMAT_LIN) << ",";
  s << Segment::TitleAsLINCommon() << "pf|y|\n";
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
      seg.date.AsString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.location.AsString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.session.AsString(BRIDGE_FORMAT_LIN) << "%" <<
      seg.event << "%" <<
      seg.scoring.AsString(BRIDGE_FORMAT_LIN) << 
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
      if (seg.title == "")
        return "";
      return Segment::TitleAsLIN();

    case BRIDGE_FORMAT_LIN_RP:
      return Segment::TitleAsLIN_RP();

    case BRIDGE_FORMAT_LIN_VG:
      if (seg.title == "")
        return "";
      return Segment::TitleAsLIN_VG();

    case BRIDGE_FORMAT_LIN_TRN:
      if (seg.title == "")
        return "";
      return Segment::TitleAsLIN_TRN();

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
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::DateAsString(const formatType f) const
{
  return seg.date.AsString(f);
}


string Segment::LocationAsString(const formatType f) const
{
  return seg.location.AsString(f);
}


string Segment::EventAsString(const formatType f) const
{
  stringstream st;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      if (seg.event == "")
        return "";
      LOG("No LIN event format");
      return "";

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
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::SessionAsString(
  const formatType f) const
{
  return seg.session.AsString(f);
}


string Segment::ScoringAsString(
  const formatType f) const
{
  return seg.scoring.AsString(f);
}


string Segment::TeamsAsString(
  const formatType f) const
{
  return seg.teams.AsString(f);
}


string Segment::TeamsAsString(
  const unsigned score1,
  const unsigned score2,
  const formatType f) const
{
  return seg.teams.AsString(f, score1, score2);
}


string Segment::FirstTeamAsString(
  const formatType f) const
{
  return seg.teams.FirstAsString(f);
}


string Segment::SecondTeamAsString(
  const formatType f) const
{
  return seg.teams.SecondAsString(f);
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
      st << activeBoard->RoomAsString(extNo, f);
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
      if (! seg.scoring.ScoringIsIMPs())
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
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::ContractsAsString(const formatType f) 
{
  stringstream s;
  string st;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      s << "rs|";
      for (auto &p: boards)
      {
        for (unsigned i = 0; i < p.board.GetLength(); i++)
        {
          p.board.SetInstance(i);
          s << p.board.ContractAsString(f) << ",";
        }
      }

      st = s.str();
      if (f == BRIDGE_FORMAT_LIN_RP || f == BRIDGE_FORMAT_LIN_VG)
        st.pop_back(); // Remove trailing comma
      return st + "|\n";

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::PlayersAsString(const formatType f) 
{
  string s1, s2;
  Board * board;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      s1 = "pw|";
      for (auto &p: boards)
      {
        for (unsigned i = 0; i < p.board.GetLength(); i++)
        {
          p.board.SetInstance(i);
          string st = p.board.PlayersAsString(f);
          s1 += st.substr(3);
        }
      }
      return s1 + "\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      board = Segment::GetBoard(0);
      if (board == nullptr || board->GetLength() != 2)
        return "";

      board->SetInstance(0);
      s1 = board->PlayersAsString(f);
      board->SetInstance(1);
      s2 = board->PlayersAsString(f);

      return "pn|" + s1 + "," + s2 + "|pg||\n";

    default:
      LOG("Invalid format " + STR(f));
      return false;
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
      return s;

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "";

    default:
      LOG("Invalid format " + STR(f));
      return false;
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
      return st + "|\n";

    case BRIDGE_FORMAT_LIN_RP:
    case BRIDGE_FORMAT_LIN_VG:
    case BRIDGE_FORMAT_LIN_TRN:
      return "";

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}

