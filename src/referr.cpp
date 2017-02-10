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
      if (! getNextWord(line, s) || s.at(0) == '{')
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
  FixType& fix)
{
  const unsigned l1 = list1.size();
  const unsigned l2 = list2.size();
  const unsigned lm = Min(l1, l2);

  unsigned i = 0;
  while (i < lm && list1[i] == list2[i])
    i++;
  // Get the tag corresponding to a difference in value.
  // i will be at the first tag for which a (tag, value) pair
  // is different.  j will be the last such tag.
  if (i == lm)
  {
    // The lists are identical.  Treat as a problem for now.
    return false;
  }

  if (i & 1)
    i--;
  
  unsigned j = 0;
  while (j < lm-i && list1[l1-1-j] == list2[l2-1-j])
    j++;
  if (j & 1)
    j--;
  
  listDelta.clear();
// cout << "i " << i << ", j " << j << endl;
  if (i+j+1 > l1)
  {
    if (i > l2-1-j)
      return false;

    fix = BRIDGE_REF_INSERT;
    for (unsigned k = i; k <= l2-1-j; k++)
      listDelta.push_back(list2[k]);
    return true;
  }
  else if (i+j+1 > l2)
  {
    fix = BRIDGE_REF_DELETE;
    for (unsigned k = i; k <= l1-1-j; k++)
      listDelta.push_back(list1[k]);
    return true;
  }
  else if (l1 == l2)
  {
    fix = BRIDGE_REF_REPLACE;
    for (unsigned k = i; k <= l1-1-j; k++)
      listDelta.push_back(list2[k]);
    return true;
  }
  else
    return false;
}


void classifyList(RefErrorClass& diff)
{
  const unsigned l = diff.list.size();
  const string& tag = diff.list[0];
  if (tag == "rs")
  {
    if (diff.type == BRIDGE_REF_REPLACE && l == 2)
    {
      // TODO: Should really count individual contracts.
    }
  }
  else if (tag == "md")
  {
    if (diff.type == BRIDGE_REF_REPLACE && l == 2)
    {
      // TODO: Do we distinguish between format and content?
    }
  }
  else if (tag == "sv")
  {
  }
  else if (tag == "mb")
  {
    if (diff.type == BRIDGE_REF_DELETE)
    {
      diff.numTags = l / 2;
      diff.code = ERR_LIN_MB_OVERLONG;
      return;
    }
  }
  else if (tag == "mc")
  {
    if (diff.type == BRIDGE_REF_REPLACE && l == 2)
    {
      diff.numTags = 1;
      diff.code = ERR_LIN_MC_CLAIM_WRONG;
      return;
    }
  }

  diff.code = ERR_SIZE;
  diff.numTags = 0;
  return;
}


static bool listIsPure(const vector<string>& list)
{
  const unsigned l = list.size();
  if (l == 0 || l % 2)
    return false;

  const string tag = list[0];
  for (unsigned i = 2; i < l; i += 2)
  {
    if (list[i] != tag)
      return false;
  }
  return true;
}


bool classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  RefErrorClass& diff)
{
  vector<string> listRef, listBuf;
  diff.type = refEntry.type;

  switch (refEntry.type)
  {
    case BRIDGE_REF_INSERT:
      // Split ref line
      diff.list.clear();
      lineToLINList(refEntry.value, diff.list);
      break;

    case BRIDGE_REF_REPLACE:
      // Split old and new line.
      listRef.clear();
      listBuf.clear();
      lineToLINList(bufferLine, listBuf);
      lineToLINList(refEntry.value, listRef);

      // If there's a single stretch of differences, work on this.
      // If not, fail for now.
      if (! deltaLINLists(listBuf, listRef, diff.list, diff.type))
      {
        diff.pureFlag = false;
        diff.code = ERR_SIZE;
        return false;
      }
      break;

    case BRIDGE_REF_DELETE:
      // Split old line.
      diff.list.clear();
      lineToLINList(bufferLine, diff.list);
      break;

    default:
      diff.pureFlag = false;
      diff.code = ERR_SIZE;
      return false;
  }

  if (listIsPure(diff.list))
  {
    diff.pureFlag = true;
    classifyList(diff);
    return (diff.code == ERR_SIZE ? false : true);
  }
  else
  {
    diff.pureFlag = false;
    diff.code = ERR_SIZE;
    diff.numTags = 0;
    return false;
  }
}

