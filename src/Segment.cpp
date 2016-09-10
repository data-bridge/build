/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


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

  bmin = BIG_BOARD;
  bmax = 0;

  firstStringFlag = true;

  seg.title = ""; 
  seg.date.Reset();
  seg.location.Reset();
  seg.event = ""; 
  seg.session.Reset(); 
  seg.scoring.Reset();
  seg.teams.Reset();
}


bool Segment::MakeBoard(const unsigned no)
{
  if (Segment::GetBoard(no) != nullptr)
    return false;

  boards.resize(len+1);
  boards[len].no = no;
  boards[len].extNo = 0;
  len++;
  if (no < bmin)
    bmin = no;
  if (no > bmax)
    bmax = no;
  return true;
}


Board * Segment::GetBoard(const unsigned no) 
{
  for (auto &p: boards)
  {
    if (p.no == no)
      return &p.board;
  }
  return nullptr;
}


bool Segment::SetTitleLIN(const string t)
{
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
  string s0[1];
  s0[0] = s.str();

  if (! Segment::SetTeams(s0, BRIDGE_FORMAT_LIN))
    return false;

  return true;
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
  const string list[],
  const formatType f)
{
  // In LIN this is 4 lines (last 4 fields of vg).
  // In PBN this is 2 lines (HomeTeam and VisitTeam).
  // In RBN this is 1 line.
  // In TXT this is 1 line.

  return seg.teams.Set(list, f); 
}


bool Segment::SetTeams(
  const string& s,
  const formatType f)
{
  // In LIN this is 4 lines (last 4 fields of vg).
  // In PBN this is 2 lines (HomeTeam and VisitTeam).
  // In RBN this is 1 line.
  // In TXT this is 1 line.

  string list[1];
  list[0] = s;
  return seg.teams.Set(list, f); 
}


bool Segment::SetPlayers(
  const string& s,
  const formatType f)
{
  // TODO
  UNUSED(s);
  UNUSED(f);
}


bool Segment::SetNumber(
  const unsigned no,
  const string& extStr)
{
  unsigned extNo;
  if (! StringToUnsigned(extStr, extNo))
  {
    LOG("Board number is not numerical");
    return false;
  }

  for (auto &p: boards)
  {
    if (p.no == no)
    {
      p.extNo = extNo;
      return true;
    }
  }

  LOG("Internal board number not found");
  return false;
}


bool Segment::SetNumber(
  const string& s,
  const formatType f)
{
  // TODO
  UNUSED(s);
  UNUSED(f);
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


string Segment::TitleAsLIN() const
{
  // In LIN this is the entire vg field.
  // We don't generate the specific "format" that Pavlicek uses
  // in his LIN files.
  
  stringstream s;
  s << "vg|";

  if (seg.title == "Bridge Base Online")
  {
    s << seg.title << ",IMPs,P,";
  }
  else if (seg.event == "Bridge Base Online")
  {
    s << seg.title << "," << 
        seg.event << "," << 
        seg.scoring.AsString(BRIDGE_FORMAT_LIN) << ",";
  }
  else
  {
    // Fudged extension of title format.
    // We also output the RBN mixture format in this way
    // (although we could recreate it with special scoring functions).

    s << seg.title << "%" <<
        seg.date.AsString(BRIDGE_FORMAT_LIN) << "%" <<
        seg.location.AsString(BRIDGE_FORMAT_LIN) << "%" <<
        seg.session.AsString(BRIDGE_FORMAT_LIN) << "," <<
        seg.event << "," << 
        seg.scoring.AsString(BRIDGE_FORMAT_LIN) << ",";
  }

  if (bInmin == 0)
    s << ",";
  else
    s << bInmin << ",";

  if (bInmax == 0)
    s << ",";
  else
    s << bInmax << ",";

  s << seg.teams.AsString(BRIDGE_FORMAT_LIN) << "|";
  
  return s.str();
}


string Segment::TitleAsString(
  const formatType f,
  const segOutputType s) const
{
  // In LIN this is the entire vg field.

  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";

  stringstream st;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::TitleAsLIN();

    case BRIDGE_FORMAT_PBN:
      st << "[Description \"" << seg.title << "\"]\n";
      return st.str();

    case BRIDGE_FORMAT_RBN:
      st << "T " << seg.title << "\n";
      return st.str();

    case BRIDGE_FORMAT_TXT:
      return seg.title;

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::DateAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  return seg.date.AsString(f);
}


string Segment::LocationAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  return seg.location.AsString(f);
}


string Segment::EventAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";

  stringstream st;
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN event format");
      return "";

    case BRIDGE_FORMAT_PBN:
      st << "[Event \"" + seg.event + "\"]\n";
      return st.str();

    case BRIDGE_FORMAT_RBN:
      st << "E " << seg.event << "\n";
      return st.str();

    case BRIDGE_FORMAT_TXT:
      return seg.event;

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::SessionAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  return seg.session.AsString(f);
}


string Segment::ScoringAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  return seg.scoring.AsString(f);
}


string Segment::TeamsAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  return seg.teams.AsString(f);
}


