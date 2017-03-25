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

static bool lineToLINListRaw(
  const string& line,
  vector<string>& list);


static string parseRefLINEntry(const string& entry)
{
  const unsigned l = entry.length();
  if (l == 0)
    return "";

  if (entry.at(0) == '\'')
  {
    // Unquote.
    if (l == 1)
      THROW("Single quote");
    if (entry.at(l-1) != '\'')
      THROW("Not ending on single quote");
    return entry.substr(1, l-2);
  }
  else
    return entry;
}


static bool parseRefLIN(
  const string& line,
  RefFix& rf)
{
  regex re("^\\s*\"(.*)\"");
  smatch match;
  if (! regex_search(line, match, re) || match.size() < 1)
    return false;
  const string arg = match.str(1);

  // TODO: Maybe have a switch on rf.type instead --cleaner.

  const unsigned commas = static_cast<unsigned>(
    count(arg.begin(), arg.end(), ','));

  vector<string> v(commas+1);
  v.clear();
  tokenize(arg, v, ",");

  if (v[0].at(0) == '-')
  {
    // Permit counts from the back of the line as well.
    v[0] = v[0].substr(1);
    rf.fixLIN.reverseFlag = true;
  }
  else
    rf.fixLIN.reverseFlag = false;

  if (commas == 0 && rf.type == BRIDGE_REF_DELETE_LIN)
  {
    // First one must be a tag number.
    if (! str2upos(v[0], rf.fixLIN.tagNo))
      return false;
    rf.fixLIN.fieldNo = 0;
    rf.fixLIN.tag = "";
    rf.fixLIN.is = "";
    rf.fixLIN.was = "";
    return true;
  }

  if (commas < 1 && commas > 4)
    return false;
  if (commas == 1 && rf.type != BRIDGE_REF_INSERT_LIN)
    return false;
  
  // First one must be a tag number.
  if (! str2upos(v[0], rf.fixLIN.tagNo))
    return false;

  // Optional field number (say in an rs value).
  // Possible collision with INSERT_LIN, hopefully a minor one...
  unsigned n = 1;
  if (str2upos(v[1], rf.fixLIN.fieldNo))
    n = 2;
  else
    rf.fixLIN.fieldNo = 0;

  // LIN tag except perhaps in deletion.
  if (v[n].length() != 2 && rf.type != BRIDGE_REF_INSERT_LIN)
    return false;
  rf.fixLIN.tag = v[n];
  n++;
  if (n > commas)
  {
    if (rf.type == BRIDGE_REF_INSERT_LIN)
    {
      rf.fixLIN.is = rf.fixLIN.tag;
      rf.fixLIN.tag = "";
      rf.fixLIN.was = "";
      return true;
    }
    else
      return false;
  }

  const string s1 = parseRefLINEntry(v[n]);
  if (rf.type == BRIDGE_REF_INSERT_LIN)
  {
    if (n < commas)
      return false;
    rf.fixLIN.was = "";
    rf.fixLIN.is = s1;
  }
  else if (rf.type == BRIDGE_REF_REPLACE_LIN)
  {
    if (n+1 != commas)
      return false;
    rf.fixLIN.was = s1;
    rf.fixLIN.is = parseRefLINEntry(v[n+1]);
  }
  else if (rf.type == BRIDGE_REF_DELETE_LIN)
  {
    if (n < commas)
      return false;
    rf.fixLIN.was = s1;
    rf.fixLIN.is = "";
  }
  else
    return false;
  
  return true;
}


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
        rf.partialFlag = true;
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
      else if (s.at(0) == '{')
      {
        rf.count = 1;
        rf.partialFlag = true;
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
    else if (s == "insertLIN")
    {
      rf.type = BRIDGE_REF_INSERT_LIN;
      rf.count = 1;
      rf.value = "";
      if (! parseRefLIN(line, rf))
        THROW("Ref file " + refName + ": Bad LIN in '" + line + "'");
      if (regex_search(line, match, rep) && match.size() >= 1)
        rf.partialFlag = true;
      else
        rf.partialFlag = false;
    }
    else if (s == "replaceLIN")
    {
      rf.type = BRIDGE_REF_REPLACE_LIN;
      rf.count = 1;
      rf.value = "";
      if (! parseRefLIN(line, rf))
        THROW("Ref file " + refName + ": Bad LIN in '" + line + "'");
      if (regex_search(line, match, rep) && match.size() >= 1)
        rf.partialFlag = true;
      else
        rf.partialFlag = false;
    }
    else if (s == "deleteLIN")
    {
      rf.type = BRIDGE_REF_DELETE_LIN;
      rf.count = 1;
      rf.value = "";
      if (! parseRefLIN(line, rf))
        THROW("Ref file " + refName + ": Bad LIN in '" + line + "'");
      if (regex_search(line, match, rep) && match.size() >= 1)
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


static string strRefFixNormalRest(const RefFix& refFix)
{
  string st = " ";
  if (refFix.count == 1)
    st += "\"" + refFix.value + "\"";
  else
    st += STR(refFix.count);

  return st;
}


static string strRefFixLINComponent(const string& text)
{
  const unsigned e = text.find(' ');
  if (e == string::npos)
    return text;
  else
    return "'" + text + "'";
}


static string strRefFixLINRest(const RefFix& refFix)
{
  string st = " \"" + STR(refFix.fixLIN.tagNo) + " ";
  if (refFix.fixLIN.fieldNo > 0)
    st += STR(refFix.fixLIN.fieldNo) + " ";
  st += refFix.fixLIN.tag + " ";

  if (refFix.type == BRIDGE_REF_INSERT_LIN)
    st += strRefFixLINComponent(refFix.fixLIN.is);
  else if (refFix.type == BRIDGE_REF_REPLACE_LIN)
  {
    st += strRefFixLINComponent(refFix.fixLIN.was) + " " +
        strRefFixLINComponent(refFix.fixLIN.is);
  }
  else if (refFix.type == BRIDGE_REF_DELETE_LIN)
    st += strRefFixLINComponent(refFix.fixLIN.was);
  else
    THROW("Bad fixLIN");

  return st + "\"";
}


string strRefFix(const RefFix& refFix)
{
  string st;
  st = STR(refFix.lno) + " ";
  switch(refFix.type)
  {
    case BRIDGE_REF_INSERT:
      st += "insert" + strRefFixNormalRest(refFix);
      break;

    case BRIDGE_REF_REPLACE:
      st += "replace" + strRefFixNormalRest(refFix);
      break;

    case BRIDGE_REF_DELETE:
      st += "delete" + strRefFixNormalRest(refFix);
      break;

    case BRIDGE_REF_INSERT_LIN:
      st += "insertLIN" + strRefFixLINRest(refFix);
      break;

    case BRIDGE_REF_REPLACE_LIN:
      st += "replaceLIN" + strRefFixLINRest(refFix);
      break;

    case BRIDGE_REF_DELETE_LIN:
      st += "deleteLIN" + strRefFixLINRest(refFix);
      break;
    
    default:
      THROW("Bad type");
  }

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


static bool lineToLINListRaw(
  const string& line,
  vector<string>& list)
{
  // Split on |
  list.clear();
  tokenize(line, list, "|");
  if (line.length() > 0 && line.at(line.length()-1) == '|')
    list.pop_back();
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


static void printModify(
  const string& line,
  const RefFixLIN& fixLIN)
{
  cout << "line   : " << line << "\n";
  if (fixLIN.reverseFlag)
    cout << "tagNo  : " << fixLIN.tagNo << " (from back)\n";
  else
    cout << "tagNo  : " << fixLIN.tagNo << "\n";
  cout << "fieldNo: " << fixLIN.fieldNo << "\n";
  cout << "tag    : " << fixLIN.tag << "\n";
  cout << "was    : " << fixLIN.was << "\n";
  cout << "is     : " << fixLIN.is << "\n";
}


static string concatFields(
  const vector<string>& list,
  const string& delim)
{
  string st;
  for (auto &f: list)
  {
    st += f + delim;
  }
  if (list.size() > 0)
    st.pop_back(); // Drop last delimiter

  return st;
}


void modifyLINFail(
  const string& line,
  const RefFixLIN& fixLIN,
  const string& text)
{
  printModify(line, fixLIN);
  THROW(text);
}


bool modifyLINLine(
  const string& line,
  const RefFix& refFix,
  string& lineNew)
{
  vector<string> v;
  v.clear();
  if (! lineToLINListRaw(line, v))
    modifyLINFail(line, refFix.fixLIN, "Couldn't convert to list");

  if (refFix.fixLIN.tagNo == 0)
    modifyLINFail(line, refFix.fixLIN, "No tag number");

  if (2 * refFix.fixLIN.tagNo > v.size())
    modifyLINFail(line, refFix.fixLIN, "Tag number too large");

  unsigned start;
  if (refFix.fixLIN.reverseFlag)
    start = v.size() - 2*refFix.fixLIN.tagNo;
  else
    start = 2*(refFix.fixLIN.tagNo-1);

  bool interiorFlag = false;
  vector<string> f;
  if (refFix.fixLIN.fieldNo > 0)
  {
    interiorFlag = true;

    const unsigned commas = static_cast<unsigned>(
      count(v[start+1].begin(), v[start+1].end(), ','));
    if (commas < 1)
      modifyLINFail(line, refFix.fixLIN, "No commas");

    f.resize(commas+1);
    f.clear();
    tokenize(v[start+1], f, ",");

    if (refFix.fixLIN.fieldNo-1 >= commas+1)
      modifyLINFail(line, refFix.fixLIN, "Field too far");
  }

  if (refFix.type == BRIDGE_REF_INSERT_LIN)
  {
    if (interiorFlag)
    {
      if (v[start] != refFix.fixLIN.tag)
        modifyLINFail(line, refFix.fixLIN, "Different tags");

      f.insert(f.begin()+static_cast<int>(refFix.fixLIN.fieldNo-1), 
        refFix.fixLIN.is);
      v[start+1] = concatFields(f, ",");
    }
    else if (refFix.fixLIN.tag == "")
    {
      // Single insertion, i.e. could be a tag or a value.
      v.insert(v.begin()+static_cast<int>(start), refFix.fixLIN.is);
    }
    else
    {
      v.insert(v.begin()+static_cast<int>(start), refFix.fixLIN.is);
      v.insert(v.begin()+static_cast<int>(start), refFix.fixLIN.tag);
    }
  }
  else if (refFix.type == BRIDGE_REF_REPLACE_LIN)
  {
    if (v[start] != refFix.fixLIN.tag)
      modifyLINFail(line, refFix.fixLIN, "Different tags: " + v[start]);

    if (interiorFlag)
    {
      if (f[refFix.fixLIN.fieldNo-1] != refFix.fixLIN.was)
        modifyLINFail(line, refFix.fixLIN, "Old field wrong");

      f[refFix.fixLIN.fieldNo-1] = refFix.fixLIN.is;
      v[start+1] = concatFields(f, ",");
    }
    else
    {
      if (v[start+1] != refFix.fixLIN.was)
        modifyLINFail(line, refFix.fixLIN, 
          "Old value wrong: " + v[start+1]);

      v[start+1] = refFix.fixLIN.is;
    }
  }
  else if (refFix.type == BRIDGE_REF_DELETE_LIN)
  {
    if (interiorFlag)
    {
      if (v[start] != refFix.fixLIN.tag)
        modifyLINFail(line, refFix.fixLIN, "Different tags: " + v[start]);

      if (f[refFix.fixLIN.fieldNo-1] != refFix.fixLIN.was)
        modifyLINFail(line, refFix.fixLIN, "Old field wrong");

      f.erase(f.begin() + static_cast<int>(refFix.fixLIN.fieldNo-1));
      v[start+1] = concatFields(f, ",");
    }
    else if (refFix.fixLIN.fieldNo == 0 && refFix.fixLIN.tag == "")
    {
      // Delete a single entry without checking it.
      // Only use this when the entry is seriously messed up.
      auto s = v.begin() + static_cast<int>(start);
      v.erase(s);
    }
    else
    {
      if (v[start] != refFix.fixLIN.tag)
        modifyLINFail(line, refFix.fixLIN, "Different tags: " + v[start]);

      if (v[start+1] != refFix.fixLIN.was)
        modifyLINFail(line, refFix.fixLIN, "Old value wrong");

      auto s = v.begin() + static_cast<int>(start);
      v.erase(s, s+2);
    }
  }
  else
    THROW("Bad type");
  
  lineNew = concatFields(v, "|") + "|";
  return true;
}


bool classifyRefLine(
  const RefFix& refEntry,
  const string& bufferLine,
  RefErrorClass& diff)
{
  vector<string> listRef, listBuf;
  string dummy;
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

    case BRIDGE_REF_INSERT_LIN:
      if (! modifyLINLine(bufferLine, refEntry, dummy))
        return false;

      diff.type = BRIDGE_REF_INSERT;
      diff.code = ERR_SIZE;
      diff.list.push_back(refEntry.fixLIN.tag);
      diff.list.push_back(refEntry.fixLIN.is);
      diff.pureFlag = true;
      diff.numTags = 1;
      break;

    case BRIDGE_REF_REPLACE_LIN:
      if (! modifyLINLine(bufferLine, refEntry, dummy))
        return false;

      diff.type = BRIDGE_REF_REPLACE;
      diff.code = ERR_SIZE;
      diff.list.push_back(refEntry.fixLIN.tag);
      diff.list.push_back(refEntry.fixLIN.is);
      diff.pureFlag = true;
      diff.numTags = 1;
      break;

    case BRIDGE_REF_DELETE_LIN:
      if (! modifyLINLine(bufferLine, refEntry, dummy))
        return false;

      diff.type = BRIDGE_REF_REPLACE;
      diff.code = ERR_SIZE;
      diff.list.push_back(refEntry.fixLIN.tag);
      diff.list.push_back(refEntry.fixLIN.was);
      diff.pureFlag = true;
      diff.numTags = 1;
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

