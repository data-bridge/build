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
#include <map>

#include "referr.h"
#include "refcodes.h"
#include "reflineGen.h"
#include "reflineLIN.h"
#include "reflinePBN.h"
#include "reflineRBN.h"
#include "reflineTXT.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


typedef void (*RefParsePtr)(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf);

static map<string, RefParsePtr> RefMap;

static bool lineToLINList(
  const string& line,
  vector<string>& list);

static bool lineToLINListRaw(
  const string& line,
  vector<string>& list);


void setRefTable()
{
  RefMap["replace"] = &parseReplaceGen;
  RefMap["insert"] = &parseInsertGen;
  RefMap["delete"] = &parseDeleteGen;

  RefMap["replaceLIN"] = &parseReplaceLIN;
  RefMap["insertLIN"] = &parseInsertLIN;
  RefMap["deleteLIN"] = &parseDeleteLIN;

  RefMap["replacePBN"] = &parseReplacePBN;
  RefMap["insertPBN"] = &parseInsertPBN;
  RefMap["deletePBN"] = &parseDeletePBN;

  RefMap["replaceRBN"] = &parseReplaceRBN;
  RefMap["insertRBN"] = &parseInsertRBN;
  RefMap["deleteRBN"] = &parseDeleteRBN;

  RefMap["replaceTXT"] = &parseReplaceTXT;
  RefMap["insertTXT"] = &parseInsertTXT;
  RefMap["deleteTXT"] = &parseDeleteTXT;
}


void readRefFix(
  const string& fname,
  vector<RefFix>& refFix,
  RefControl& refControl)
{
  regex re("\\.\\w+$");
  string refName = regex_replace(fname, re, string(".ref"));
  refControl = ERR_REF_STANDARD;

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
    {
      if (s == "skip")
        refControl = ERR_REF_SKIP;
      else if (s == "noval")
        refControl = ERR_REF_NOVAL;
      else if (s == "orderCOCO")
        refControl = ERR_REF_OUT_COCO;
      else if (s == "orderOOCC")
        refControl = ERR_REF_OUT_OOCC;
      else
        THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      continue;
    }
      
    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    auto it = RefMap.find(s);
    if (it == RefMap.end())
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");
    
    (* it->second)(refName, line, rer, rep, match, rf);

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

  // Can be expanded.
  if (tag == "mb")
  {
    if (diff.type == BRIDGE_REF_DELETE)
    {
      diff.numTags = l / 2;
      diff.code = ERR_LIN_MB_TRAILING;
      return;
    }
  }
  else if (tag == "mc")
  {
    if (diff.type == BRIDGE_REF_REPLACE && l == 2)
    {
      diff.numTags = 1;
      diff.code = ERR_LIN_MC_REPLACE;
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
  {
    if (refFix.fixLIN.is == "" && refFix.fixLIN.was == "" &&
        2 * refFix.fixLIN.tagNo == v.size() + 1)
    {
      // Last tag, no argument.
    }
    else
      modifyLINFail(line, refFix.fixLIN, "Tag number too large");
  }

  const bool endsOnPipe = 
    (line.length() > 0 && line.at(line.length()-1) == '|');

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
    {
      if (refFix.type != BRIDGE_REF_INSERT_LIN)
        modifyLINFail(line, refFix.fixLIN, "Field too far");
      else if (refFix.fixLIN.fieldNo-1 >= commas+2)
        modifyLINFail(line, refFix.fixLIN, "Insert field too far");
    }
  }

  if (refFix.type == BRIDGE_REF_REPLACE_LIN)
    modifyReplaceLIN(line, start, interiorFlag, refFix, v, f);
  else if (refFix.type == BRIDGE_REF_INSERT_LIN)
    modifyInsertLIN(line, start, interiorFlag, refFix, v, f);
  else if (refFix.type == BRIDGE_REF_DELETE_LIN)
    modifyDeleteLIN(line, start, interiorFlag, refFix, v, f);
  else
    THROW("Bad type");
  
  lineNew = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    lineNew += "|";
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

