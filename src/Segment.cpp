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

  bmin = BIGNUM;
  bmax = 0;

  title = ""; 
  date.reset();
  location.reset();
  event = ""; 
  session.reset(); 
  scoring.reset();
  teams.reset();
  headerLIN.reset();
  flagCOCO = false;
}


Board const * Segment::getBoard(const unsigned extNo) const
{
  for (auto &p: boards)
    if (p.extNo == extNo)
      return &p.board;

  return nullptr;
}


Board * Segment::acquireBoard(const unsigned extNo)
{
  for (auto &p: boards)
    if (p.extNo == extNo)
      return &p.board;

  // Make a new board
  BoardPair * bpPrev = (len > 0 ? &boards.back() : nullptr);
  boards.emplace_back(BoardPair());
  BoardPair& bp = boards.back();
  Board& board = bp.board;

  bp.intNo = len;
  bp.extNo = extNo;
  len++;

  if (extNo < bmin)
    bmin = extNo;
  if (extNo > bmax)
    bmax = extNo;

  if (headerLIN.isSet())
  {
    // Make enough room.
    board.acquireInstance(1);

    const unsigned linTableNo = Segment::getLINActiveNo(len-1);
    board.setLINheader(headerLIN.getEntry(linTableNo));
  }
  else if (len > 1)
  {
    // Copy players in order to have something.
    const unsigned instCount = bpPrev->board.countAll();
    if (instCount == 0)
      THROW("Empty predecessor board");

    // Make enough room.
    board.acquireInstance(instCount-1);
    board.copyPlayers(bpPrev->board);
  }

  return &bp.board;
}


unsigned Segment::getIntBoardNo(const unsigned extNo) const
{
  for (auto &p: boards)
    if (p.extNo == extNo)
      return p.intNo;

  THROW("Bad external board number: " + STR(extNo));
}


unsigned Segment::getExtBoardNo(const unsigned intNo) const
{
  for (auto &p: boards)
    if (p.intNo == intNo)
      return p.extNo;

  THROW("Bad internal board number: " + STR(intNo));
}


unsigned Segment::getLINActiveNo(const unsigned intNo) const
{
  const unsigned eno = Segment::getExtBoardNo(intNo);
  if (eno < bmin || eno > bmax)
    THROW("Board number out of range of LIN header");
  return(eno - bmin);
}


void Segment::setCOCO(const Format format)
{
  flagCOCO = true;
  if (format == BRIDGE_FORMAT_PBN ||
      format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
    teams.swap();
}


bool Segment::getCOCO() const
{
  return flagCOCO;
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
    bmin = BIGNUM;
  else if (! str2upos(v[3], bmin))
    THROW("Not a board number");

  if (v[4] == "")
    bmax = 0;
  else if (! str2upos(v[4], bmax))
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

  if (format == BRIDGE_FORMAT_TXT && flagCOCO)
    teams.swap();
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


void Segment::setResultsList(
  const string& text,
  const Format format)
{
  headerLIN.setResultsList(text, format);
}


void Segment::setPlayersList(
  const string& text,
  const Format format)
{
  headerLIN.setPlayersList(text, scoring.str(BRIDGE_FORMAT_LIN), format);
}


void Segment::setPlayersHeader(
  const string& text,
  const Format format)
{
  headerLIN.setPlayersHeader(text, scoring.str(BRIDGE_FORMAT_LIN), format);
}


void Segment::setScoresList(
  const string& text,
  const Format format)
{
  headerLIN.setScoresList(text, scoring.str(BRIDGE_FORMAT_LIN), format);
}


void Segment::setBoardsList(
  const string& text,
  const Format format)
{
  headerLIN.setBoardsList(text, format);
}


void Segment::setRoom(
  const string& text,
  const Format format)
{
  // We have already selected the room, so don't need this.
  UNUSED(text);
  UNUSED(format);
}


void Segment::setNumber(
  const string& text,
  const Format format)
{
  // We have already selected the room, so don't need this.
  UNUSED(text);
  UNUSED(format);
}


unsigned Segment::firstBoardNumber() const
{
  if (len == 0)
    return BIGNUM;
  else if (bmin == BIGNUM)
    return boards.front().extNo;
  else
    return bmin;
}


unsigned Segment::lastRealBoardNumber() const
{
  if (len == 0)
    return BIGNUM;
  else
    return boards.back().extNo;
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

  for (auto it1 = boards.cbegin(), it2 = segment2.boards.cbegin();
    it1 != boards.cend() && it2 != segment2.boards.cend();
    it1++, it2++)
  {
    if (it1->intNo != it2->intNo)
      DIFF("Different internal number of board");
    else if (it1->extNo != it2->extNo)
      DIFF("Different external number of board");
    else if (it1->board != it2->board)
      DIFF("Different boards"); // Board == will already throw, though
  }

  return true;
}


bool Segment::operator != (const Segment& segment2) const
{
  return ! (* this == segment2);
}


string Segment::strTitleLINCore() const
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

  ss << teams.str(BRIDGE_FORMAT_LIN) << "|";
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
  return "vg|" + title +
      session.str(BRIDGE_FORMAT_LIN_RP) + "," +
      scoring.str(BRIDGE_FORMAT_LIN) + "," +
      Segment::strTitleLINCore() + "pf|y|\n";
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
      format == BRIDGE_FORMAT_RBX ||
      format == BRIDGE_FORMAT_TXT)
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
  const unsigned extNo,
  const Format format) const
{
  unsigned intNo;

  stringstream ss;
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_TRN:
      if (format == BRIDGE_FORMAT_LIN_TRN || ! headerLIN.isSet())
        ss << "ah|Board " << extNo << "|";
      else
      {
        intNo = Segment::getIntBoardNo(extNo);
        ss << "ah|Board " << headerLIN.strBoard(intNo) << "|";
      }
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


string Segment::strContractsCore(const Format format) const
{
  string st = "";

  if (format == BRIDGE_FORMAT_PAR)
    return headerLIN.strContractsList();
  else
  {
    for (unsigned b = bmin; b <= bmax; b++)
    {
      Board const * bptr = Segment::getBoard(b);
      if (bptr == nullptr)
        st += headerLIN.strContracts(b-bmin);
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


string Segment::strContracts(const Format format) const
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


string Segment::strPlayersLIN() const
{
  Board const * refBoard = nullptr;
  string st = "pw|";
  const bool isIMPs = scoring.isIMPs();
  if (! headerLIN.isSet())
  {
    for (auto &p: boards)
    {
      // This is not quite the same as below.
      // Say we have PBN 980, 982, 983, ...
      // The skipped board should not (currently) lead to a LIN gap.
      st += p.board.strPlayersBoard(BRIDGE_FORMAT_LIN, isIMPs, refBoard);
      refBoard = &p.board;
    }
  }
  else
  {
    string sprev;
    for (unsigned b = bmin; b <= bmax; b++)
    {
      Board const * bptr = Segment::getBoard(b);
      if (bptr == nullptr)
      {
        const unsigned no = (isIMPs ? 2u : 1u);
        string sm = headerLIN.strPlayers(b-bmin, no);

        if (sm == sprev)
        {
          sm = "";
          for (unsigned i = 0; i < no; i++)
            sm += ",,,,";
        }

        st += sm;
      }
      else
      {
        sprev = bptr->strPlayersBoard(BRIDGE_FORMAT_LIN, isIMPs, refBoard);
        st += sprev;
        refBoard = bptr;
      }
    }
  }

  st.pop_back();
  return st + "|\n";
}


string Segment::strPlayers(const Format format) const
{
  switch (format)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::strPlayersLIN();

    case BRIDGE_FORMAT_LIN_VG:
      if (headerLIN.hasPlayerList())
        return Segment::strPlayersLIN();
      else
        return boards.front().board.strPlayersBoard(format);

    case BRIDGE_FORMAT_LIN_RP:
      return boards.front().board.strPlayersBoard(format);

    case BRIDGE_FORMAT_LIN_TRN:
      return boards.front().board.strPlayersBoard(BRIDGE_FORMAT_LIN_VG);

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
      if (! headerLIN.isSet())
      {
        for (auto &p: boards)
          ss << p.extNo << ",";
      }
      else
      {
        for (auto &p: boards)
          ss << headerLIN.strBoard(p.extNo - bmin) << ",";
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
  ss << setw(66) << right << score1 << setw(6) << score2 << "\n\n";
  return ss.str();  
}

