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
  int seen = static_cast<int>(count(players.begin(), players.end(), ','));
  if (seen != 7)
    return;

  playersFlag = true;
  tokenize(players, pnames, ",");
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

  players = segment->strPlayers(BRIDGE_FORMAT_LIN_VG);
  if (players.length() >= 10)
    players = players.substr(3, players.length()-9);

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


bool DuplStat::differentPlayers(const DuplStat& ds2) const
{
  // Detect the pairs case where both are missing the closed-room
  // players, and the open-room players are all different.
  // Also detects when all 8 players are different.

  if (! playersFlag || ! ds2.playersFlag)
    return false;

  bool emptyFlag = true;
  for (unsigned j = 4; j < 8; j++)
  {
    if (pnames[j] != "" || ds2.pnames[j] != "")
    {
      emptyFlag = false;
      break;
    }
  }

  const unsigned upper = (emptyFlag ? 4u : 8u);
  for (unsigned j = 0; j < upper; j++)
  {
    if (pnames[j] == ds2.pnames[j])
      return false;
  }
  return true;
}


bool DuplStat::samePlayers(const DuplStat& ds2) const
{
  // All 8 players have to be identical.
  if (! playersFlag || ! ds2.playersFlag)
    return false;

  for (unsigned j = 0; j < 8; j++)
  {
    if (pnames[j] != ds2.pnames[j])
      return false;
  }
  return true;
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
  string t1 = teams;
  toLower(t1);
  string t2 = ds2.teams;
  toLower(t2);
  if (levenshtein(t1, t2) > 2)
    return false;

  if (DuplStat::differentPlayers(ds2))
    return false;

  if (! DuplStat::samePlayers(ds2))
    return false;

  for (auto it1 = values.cbegin(), it2 = ds2.values.cbegin();
      it1 != values.cend() && it2 != ds2.values.cend(); it1++, it2++)
  {
    if (*it1 != *it2)
      return false;
  }
  return true;
}


bool DuplStat::operator <= (const DuplStat& ds2) const
{
  if (numHands > ds2.numHands || numBoards > ds2.numBoards)
    return false;

  // Could also just compare (teams != ds2.teams)
  string t1 = teams;
  toLower(t1);
  string t2 = ds2.teams;
  toLower(t2);
  if (levenshtein(t1, t2) > 2)
    return false;

  if (DuplStat::differentPlayers(ds2))
    return false;

  if (! DuplStat::samePlayers(ds2))
    return false;

  auto it1 = values.cbegin();
  for (auto it2 = ds2.values.cbegin();
      it1 != values.cend() && it2 != ds2.values.cend(); it2++)
  {
    if (*it1 == *it2)
      it1++;
  }
  return (it1 == values.cend());
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

