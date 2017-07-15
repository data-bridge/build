/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <string>
#include <iomanip>
#include <sstream>

#include "DuplStat.h"
#include "Group.h"
#include "RefLines.h"
#include "parse.h"
#include "Bexcept.h"


DuplStat::DuplStat()
{
  DuplStat::reset();
}


DuplStat::~DuplStat()
{
}


void DuplStat::reset()
{
  fname = "";
  basename = "";

  segTitle = "";
  segDate = "";
  segLocation = "";
  segEvent = "";
  segSession = "";

  pnames.clear();
  playersFlag = false;

  numLines = 0;
  values.clear();
}


void DuplStat::extractPlayers()
{
  playersFlag = true;
  tokenize(players, pnames, ",");

  if (pnames.size() & 0x3)
    return;

  for (unsigned i = 0; i < pnames.size(); i += 4)
  {
    if (pnames[i] == "north")
      pnames[i] = "";
    if (pnames[i+1] == "east")
      pnames[i+1] = "";
    if (pnames[i+2] == "south")
      pnames[i+2] = "";
    if (pnames[i+3] == "west")
      pnames[i+3] = "";
  }
}


void DuplStat::set(
  const Group * group,
  const Segment * segment,
  const unsigned segNo,
  const RefLines * reflines)
{
  DuplStat::reset();

  fname = group->name();
  format = group->format();

  basename = fname;
  size_t l = basename.find_last_of("\\/");
  if (l != string::npos)
    basename.erase(0, l+1);
  l = basename.find_last_of(".");
  if (l != string::npos)
    basename = basename.substr(0, l);

  segTitle = segment->strTitle(BRIDGE_FORMAT_TXT);

  string s = segment->strDate(BRIDGE_FORMAT_TXT);
  if (s != "\n") 
    segDate = s;

  s = segment->strLocation(BRIDGE_FORMAT_TXT);
  if (s != "\n") 
    segLocation = s;

  s = segment->strEvent(BRIDGE_FORMAT_TXT);
  if (s != "\n") 
    segEvent = s;

  s = segment->strSession(BRIDGE_FORMAT_TXT);
  if (s != "\n") 
    segSession = s;

  teams = segment->strTeams(BRIDGE_FORMAT_TXT) + "\n";
  toLower(teams);

  players = segment->strPlayers(BRIDGE_FORMAT_LIN_VG);
  if (players.length() >= 10)
    players = players.substr(3, players.length()-9);

  toLower(players);
  DuplStat::extractPlayers();

  segNoVal = segNo;
  segSize = group->size();

  reflines->getHeaderData(numLines, numHands, numBoards);
}


void DuplStat::append(const int hashVal)
{
  values.push_back(static_cast<unsigned>(hashVal));
}


void DuplStat::sort()
{
  values.sort();
}


unsigned DuplStat::first() const
{
  if (values.size() == 0)
    THROW("List is empty");
  return values.front();
}


bool DuplStat::sameOrigin(const DuplStat& ds2) const
{
  if (basename == "")
    THROW("No group");
  return (basename == ds2.basename);
}


bool DuplStat::similarPlayerGroup(
  const DuplStat& ds2,
  const unsigned number,
  const unsigned ds2offset) const
{
  unsigned similar = 0;
  unsigned actual = 0;

  for (unsigned i = 0; i < number; i++)
  {
    const int l1 = static_cast<int>(pnames[i].length());
    const int l2 = static_cast<int>(ds2.pnames[ds2offset+i].length());

    if (l1 == 0 || l2 == 0)
      continue;
    else if (abs(l1-l2) > 2 || 
        ! levenshtein_test(pnames[i], ds2.pnames[ds2offset+i], 2))
    {
      actual++;
      continue;
    }
    else
    {
      actual++;
      similar++;
    }
  }

  // 4: Need 3.
  // 8: Need 5.
  return (similar >= 2 && 3*similar >= 2*actual-1);
}


bool DuplStat::similarPlayers(const DuplStat& ds2) const
{
  if (! playersFlag || ! ds2.playersFlag)
    return false;

  const unsigned l1 = pnames.size();
  const unsigned l2 = ds2.pnames.size();

  if (l1 == l2)
  {
    return DuplStat::similarPlayerGroup(ds2, l1, 0);
  }
  else if (l1 == 8 && (l2 & 0x7) == 0)
  {
    for (unsigned i = 0; i < l2; i += 8)
    {
      if (DuplStat::similarPlayerGroup(ds2, 8, i))
        return true;
    }
    return false;
  }
  else if (l2 == 8 && (l1 & 0x7) == 0)
  {
    for (unsigned i = 0; i < l1; i += 8)
    {
      if (ds2.similarPlayerGroup(* this, 8, i))
        return true;
    }
    return false;
  }
  else
    return false;
}


bool DuplStat::lexLessThan(const DuplStat& ds2) const
{
  // First by list, then by length, then by basename, then by format.

  for (auto it1 = values.cbegin(), it2 = ds2.values.cbegin();
      it1 != values.cend() && it2 != ds2.values.cend(); it1++, it2++)
  {
    if (*it1 < *it2)
      return true;
    if (*it1 > *it2)
      return false;
  }

  if (numHands < ds2.numHands)
    return true;
  if (numHands > ds2.numHands)
    return false;

  if (numBoards < ds2.numBoards)
    return true;
  if (numBoards > ds2.numBoards)
    return false;

  if (basename < ds2.basename)
    return true;
  if (basename > ds2.basename)
    return false;

  return (format < ds2.format);
}


bool DuplStat::operator == (const DuplStat& ds2) const
{
  if (numHands != ds2.numHands || numBoards != ds2.numBoards)
    return false;

  // Could also just compare (teams != ds2.teams)
  if (levenshtein(teams, ds2.teams) > 2)
    return false;

  for (auto it1 = values.cbegin(), it2 = ds2.values.cbegin();
      it1 != values.cend() && it2 != ds2.values.cend(); it1++, it2++)
  {
    if (*it1 != *it2)
      return false;
  }

  if (! DuplStat::similarPlayers(ds2))
    return false;

  return true;
}


bool DuplStat::operator <= (const DuplStat& ds2) const
{
  // This function is quite time-consuming.  The ordering of the
  // various tests is on purpose.

  if (numHands > ds2.numHands || numBoards > ds2.numBoards)
    return false;

  const int l1 = static_cast<int>(teams.length());
  const int l2 = static_cast<int>(ds2.teams.length());
  if (abs(l1-l2) > 2)
    return false;

  unsigned overlap = 0;
  auto it1 = values.cbegin();
  auto it2 = ds2.values.cbegin();
  while (it1 != values.cend() && it2 != ds2.values.cend())
  {
    if (*it1 < *it2)
      it1++;
    else if (*it1 > *it2)
      it2++;
    else
    {
      it1++;
      it2++;
      overlap++;
    }
  }
  if (overlap == 0 || (numHands >= 8 && overlap == 1))
    return false;

  if (! DuplStat::similarPlayers(ds2))
    return false;

  // Could also just compare (teams != ds2.teams)
  return levenshtein_test(teams, ds2.teams, 2);
}


string DuplStat::strRef() const
{
  if (numLines == 0)
    return "";

  stringstream ss;
  ss << "(" << numLines << "," << numHands << "," << numBoards << ")";
  return ss.str();
}


string DuplStat::strDiff(
  const string& snew,
  const string& sold) const
{
  if (snew == sold)
    return "";
  else
    return snew;
}


string DuplStat::str() const
{
  if (segTitle == "")
    THROW("No segment");

  stringstream ss;
  ss << fname << ", ref " << DuplStat::strRef();
  if (segSize != 1)
    ss << " - WARNING: segment " << segNoVal << " of " << segSize;

  ss << "\n" << 
    segTitle << 
    segDate <<
    segLocation <<
    segEvent <<
    segSession <<
    teams << 
    players << "\n";
  return ss.str();
}


string DuplStat::str(const DuplStat& ds2) const
{
  if (segTitle == "")
    THROW("No segment");

  stringstream ss;
  ss << fname << ", ref " << DuplStat::strRef();
  if (segSize != 1)
    ss << " - WARNING: segment " << segNoVal << " of " << segSize;

  ss << "\n" <<
    DuplStat::strDiff(segTitle, ds2.segTitle) <<
    DuplStat::strDiff(segDate, ds2.segDate) <<
    DuplStat::strDiff(segLocation, ds2.segLocation) <<
    DuplStat::strDiff(segEvent, ds2.segEvent) <<
    DuplStat::strDiff(segSession, ds2.segSession) <<
    DuplStat::strDiff(teams, ds2.teams) <<
    DuplStat::strDiff(players, ds2.players) << "\n";
  return ss.str();
}


string DuplStat::strSuggest(const bool fullFlag) const
{
  if (numLines == 0)
    THROW("No reflines");
  string tag;
  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
    tag = (fullFlag ? "ERR_LIN_DUPLICATE" : "ERR_LIN_SUBSET");
  else
    THROW("Can't yet skip format " + STR(format));

  return "skip {" + tag + DuplStat::strRef() + "}\n";
}

