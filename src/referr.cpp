/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>

#include "referr.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


static bool lineToLINList(
  const string& line,
  vector<string>& list);


void readRefFix(
  const string& fname,
  vector<RefFix>& refFix)
{
  regex re("\\.\\w+$");
  string refName = regex_replace(fname, re, string(".ref"));

  // There might not be a .ref file (not an error).
  ifstream refstr(refName.c_str());
  if (! refstr.is_open())
    return;

  string line, s;
  RefFix rf;
  regex rer("^\\s*\"(.*)\"\\s*$");
  regex rep("^\\s*\"(.*)\"\\s+\\{.*\\}\\s*$");
  smatch match;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (! str2unsigned(s, rf.lno))
      THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      
    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (s == "insert")
    {
      rf.type = BRIDGE_REF_INSERT;
      if (regex_search(line, match, rer) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = false;
      }
      else if (regex_search(line, match, rep) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = false;
      }
      else
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");
    }
    else if (s == "replace")
    {
      rf.type = BRIDGE_REF_REPLACE;
      if (regex_search(line, match, rer) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = false;
      }
      else if (regex_search(line, match, rep) && match.size() >= 1)
      {
        rf.value = match.str(1);
        rf.count = 1;
        rf.partialFlag = true;
      }
      else
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");
    }
    else if (s == "delete")
    {
      rf.type = BRIDGE_REF_DELETE;
      if (! getNextWord(line, s))
      {
        rf.count = 1;
        rf.partialFlag = false;
      }
      else if (! str2unsigned(s, rf.count))
      {
        THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      }
      else if (getNextWord(line, s))
        rf.partialFlag = true;
      else
        rf.partialFlag = false;

    }
    else
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    refFix.push_back(rf);
  }
  refstr.close();
}


static bool lineToLINList(
  const string& line,
  vector<string>& list)
{
  // Split on |
  vector<string> temp;
  temp.clear();
  tokenize(line, temp, "|");

  unsigned const l = temp.size();
  unsigned i = 0;
  while (i < l)
  {
    // Must start with token.
    if (temp[i].length() != 2 || i+1 == l)
      return false;

    if (temp[i] == "nt" || temp[i] == "pg" || temp[i] == "ob" ||
        temp[i] == "sa" || temp[i] == "mn" || temp[i] == "em" ||
	temp[i] == "bt")
    {
      // Skip over any chats with embedded |'s.
      i += 2;
      while (i < l && temp[i].length() != 2)
        i++;
    }
    else
    {
      list.push_back(temp[i]);

      // Skip over any chats with embedded |'s.
      string st = temp[i+1];
      i += 2;
      while (i < l && temp[i].length() != 2)
        st += temp[i++];
      list.push_back(st);
    }
  }
  return true;
}


static bool deltaLINLists(
  const vector<string>& list1,
  const vector<string>& list2,
  vector<string>& listDelta,
  bool& list1LongerFlag)
{
  const unsigned l1 = list1.size();
  const unsigned l2 = list2.size();
  const unsigned lm = Min(l1, l2);

  unsigned i = 0;
  while (i < lm && list1[i] == list2[i])
    i++;
  // Get the tag corresponding to a difference in value.
  if (i & 1)
    i--;
  
  unsigned j = 0;
  while (j < lm && list1[l1-1-j] == list2[l2-1-j])
    j++;
  if ((j & 1) == 0)
    j++;
  
  listDelta.clear();
  if (i > l1-1-j)
  {
    if (i > l2-1-j)
      return false;

    list1LongerFlag = false;
    for (unsigned k = i; k <= l2-1-j; k++)
      listDelta.push_back(list2[k]);
    return true;
  }
  else if (i > l2-1-j)
  {
    list1LongerFlag = true;
    for (unsigned k = i; k <= l1-1-j; k++)
      listDelta.push_back(list1[k]);
    return true;
  }
  else
    return false;
}


string strRefFix(const RefFix& refFix)
{
  string st;
  st = STR(refFix.lno) + " ";
  if (refFix.type == BRIDGE_REF_INSERT)
    st += "insert";
  else if (refFix.type == BRIDGE_REF_REPLACE)
    st += "replace";
  else if (refFix.type == BRIDGE_REF_DELETE)
    st += "delete";
  else
    st += "ERROR";

  st += " ";
  if (refFix.count == 1)
    st += "\"" + refFix.value + "\"";
  else
    st += STR(refFix.count);

  return st;
}


RefErrorsType classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  unsigned& numLINTags)
{
  vector<string> listRef, listBuf, listDelta;
  bool bufLongerFlag;
  numLINTags = 0;

  // TODO
  // Start out with local changes that only affect one qx
  
  switch (refEntry.type)
  {
    case BRIDGE_REF_INSERT:
      // Split ref line
      listRef.clear();
      lineToLINList(refEntry.value, listRef);

      if (listRef.size() != 2)
        return ERR_SIZE;

      return ERR_SIZE;

    case BRIDGE_REF_REPLACE:
      // Split old and new line.
      listRef.clear();
      listBuf.clear();
      lineToLINList(bufferLine, listBuf);
      lineToLINList(refEntry.value, listRef);

      // If there's a single stretch of differences, work on this.
      // If not, fail for now.
      if (! deltaLINLists(listBuf, listRef, listDelta, bufLongerFlag))
        return ERR_SIZE;
      
      if (listDelta.size() != 2)
        return ERR_SIZE;

      if (bufLongerFlag)
      {
        if (listDelta[0] == "mb")
	{
	}
	else if (listDelta[0] == "sv")
	{
	}
	else if (listDelta[0] == "mb")
	{
	}
	else
          return ERR_SIZE;
        return ERR_SIZE;
      }
      else
      {
        if (listDelta[0] == "mb")
	{
	}
	else if (listDelta[0] == "sv")
	{
	}
	else if (listDelta[0] == "mb")
	{
	}
	else
          return ERR_SIZE;
        return ERR_SIZE;
      }

      // rs is a special case with known format
      // ERR_LIN_SV_WRONG
      // ERR_LIN_MB_WRONG
      // ERR_LIN_MB_OVERLONG
      // ERR_LIN_MC_CLAIM_WRONG
      return ERR_SIZE;
      

    case BRIDGE_REF_DELETE:
      // Split old line.
      listBuf.clear();
      lineToLINList(bufferLine, listBuf);

      // mc deletion could be unneeded (in fact, it should be).
      // ERR_LIN_MB_OVERLONG
      return ERR_SIZE;

    default:
      return ERR_SIZE;
  }
}

