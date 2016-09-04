/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Segment.h"
#include "portab.h"
#include "debug.h"

extern Debug debug;


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

  firstStringFlag = true;

  seg.title = ""; 
  seg.date.Reset();
  seg.location = ""; 
  seg.event = ""; 
  seg.session = ""; 
  seg.scoring = BRIDGE_SCORING_UNDEFINED;
  seg.team1.Reset();
  seg.team2.Reset();
}


bool Segment::MakeBoard(const unsigned no)
{
  if (Segment::GetBoard(no) != nullptr)
    return false;

  boards.resize(len+1);
  boards[len].extNo = no;
  len++;
  return true;
}


Board * Segment::GetBoard(const unsigned extNo) 
{
  for (auto &p: boards)
  {
    if (p.extNo == extNo)
      return &p.board;
  }
  return nullptr;
}


bool Segment::SetTitleLIN(const string t[])
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetTitlePBN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetTitleRBN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetTitleTXT(const string t[])
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetTitle(
  const string t[],
  const formatType f)
{
  // In LIN this includes 2 lines (vg positions 1 and 2)
  // and may contain a number of other information (beyond the
  // LIN format).
  // In PBN it is the Description tag.
  // In RBN it is the T tag.
  // In TXT it 7 lines (derived from RBN tags T, D, L, E, S, F).
  
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::SetTitleLIN(t);

    case BRIDGE_FORMAT_PBN:
      return Segment::SetTitlePBN(t[0]);

    case BRIDGE_FORMAT_RBN:
      return Segment::SetTitlePBN(t[0]);

    case BRIDGE_FORMAT_TXT:
      return Segment::SetTitleTXT(t);

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
  const string& t)
{
  seg.location = t;
  return true;
}


bool Segment::SetEvent(
  const string& t)
{
  seg.event = t;
  return true;
}


bool Segment::SetSessionPBN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetSessionRBN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetSessionTXT(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetSession(
  const string& t,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN session format");
      return false;

    case BRIDGE_FORMAT_PBN:
      return Segment::SetSessionPBN(t);

    case BRIDGE_FORMAT_RBN:
      return Segment::SetSessionRBN(t);

    case BRIDGE_FORMAT_TXT:
      return Segment::SetSessionTXT(t);

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


bool Segment::SetScoringLIN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}

bool Segment::SetScoringPBN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetScoringRBN(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetScoringTXT(const string& t)
{
  // TODO
  UNUSED(t);
  return true;
}


bool Segment::SetScoring(
  const string& t,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::SetScoringLIN(t);

    case BRIDGE_FORMAT_PBN:
      return Segment::SetScoringPBN(t);

    case BRIDGE_FORMAT_RBN:
      return Segment::SetScoringRBN(t);

    case BRIDGE_FORMAT_TXT:
      return Segment::SetScoringTXT(t);

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


bool Segment::SetTeams(
  const string list[],
  const formatType f)
{
  // In LIN this is 4 lines (last 4 fields of vg).
  // In PBN this is 2 lines (HomeTeam and VisitTeam).
  // In RBN this is 1 line.
  // In TXT this is 1 line.

  bool b1 = seg.team1.Set(list, 1, f); 
  bool b2 = seg.team2.Set(list, 2, f); 

  if (b1 && b2)
    return true;
  else
    return false;
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
  else if (seg.team1 != s2.seg.team1)
    return false;
  else if (seg.team2 != s2.seg.team2)
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
  // TODO
  // In LIN this is the entire vg field.
}


string Segment::TitleAsPBN() const
{
 // TODO
}


string Segment::TitleAsRBN() const
{
 // TODO
}


string Segment::TitleAsTXT() const
{
 // TODO
}


string Segment::TitleAsString(
  const formatType f,
  const segOutputType s) const
{
  // In LIN this is the entire vg field.

  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Segment::TitleAsLIN();

    case BRIDGE_FORMAT_PBN:
      return Segment::TitleAsPBN();

    case BRIDGE_FORMAT_RBN:
      return Segment::TitleAsRBN();

    case BRIDGE_FORMAT_TXT:
      return Segment::TitleAsTXT();

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

  stringstream st;
  string sr;

  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN location format");
      return "";

    case BRIDGE_FORMAT_PBN:
      st << "[Site \"" << seg.location << "\"]\n";
      return st.str();

    case BRIDGE_FORMAT_RBN:
      st << "L " << seg.location << "\n";
      return st.str();

    case BRIDGE_FORMAT_TXT:
      sr = seg.location;
      size_t pos;
      if ((pos = sr.find(":", 0)) != string::npos)
        sr.replace(pos, 1, ", ");
      return sr;

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::EventAsPBN() const
{
  // TODO
}


string Segment::EventAsRBN() const
{
  // TODO
}


string Segment::EventAsTXT() const
{
  // TODO
}


string Segment::EventAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN event format");
      return "";

    case BRIDGE_FORMAT_PBN:
      return Segment::EventAsPBN();

    case BRIDGE_FORMAT_RBN:
      return Segment::EventAsRBN();

    case BRIDGE_FORMAT_TXT:
      return Segment::EventAsTXT();

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::SessionAsPBN() const
{
  // TODO
}


string Segment::SessionAsRBN() const
{
  // TODO
}


string Segment::SessionAsTXT() const
{
  // TODO
}


string Segment::SessionAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN session format");
      return "";

    case BRIDGE_FORMAT_PBN:
      return Segment::SessionAsPBN();

    case BRIDGE_FORMAT_RBN:
      return Segment::SessionAsRBN();

    case BRIDGE_FORMAT_TXT:
      return Segment::SessionAsTXT();

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::ScoringAsPBN() const
{
  // TODO
}


string Segment::ScoringAsRBN() const
{
  // TODO
}


string Segment::ScoringAsTXT() const
{
  // TODO
}


string Segment::ScoringAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      LOG("No LIN scoring format");
      return "";

    case BRIDGE_FORMAT_PBN:
      return Segment::ScoringAsPBN();

    case BRIDGE_FORMAT_RBN:
      return Segment::ScoringAsRBN();

    case BRIDGE_FORMAT_TXT:
      return Segment::ScoringAsTXT();

    default:
      LOG("Invalid format " + STR(f));
      return false;
  }
}


string Segment::TeamsAsString(
  const formatType f,
  const segOutputType s) const
{
  if (! firstStringFlag && s == SEGMENT_DELTA)
    return "";
  return seg.team1.AsString(seg.team2, f);
}


