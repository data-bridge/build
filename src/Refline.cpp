/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <regex>
#include <map>

#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include "RefLine.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


static map<string, FixType> FixMap;
static string FixTable[BRIDGE_REF_FIX_SIZE];

typedef void (RefLine::*ParsePtr)(
  const string& refName, 
  const string& quote);
static ParsePtr ParseList[BRIDGE_REF_FIX_SIZE];

static mutex mtx;
static bool setRefLineTables = false;


RefLine::RefLine()
{
  RefLine::reset();
  if (! setRefLineTables)
  {
    mtx.lock();
    if (! setRefLineTables)
      RefLine::setTables();
    setRefLineTables = true;
    mtx.unlock();
  }
}


RefLine::~RefLine()
{
}


void RefLine::reset()
{
  setFlag = false;

  range.lno = 0;
  range.lcount = 0;

  fix = BRIDGE_REF_FIX_SIZE;

  edit.reset();
  comment.reset();
}


void RefLine::setFixTables()
{
  FixMap["replace"] = BRIDGE_REF_REPLACE_GEN;
  FixMap["insert"] = BRIDGE_REF_INSERT_GEN;
  FixMap["delete"] = BRIDGE_REF_DELETE_GEN;

  FixMap["replaceLIN"] = BRIDGE_REF_REPLACE_LIN;
  FixMap["insertLIN"] = BRIDGE_REF_INSERT_LIN;
  FixMap["deleteLIN"] = BRIDGE_REF_DELETE_LIN;

  FixMap["replacePBN"] = BRIDGE_REF_REPLACE_PBN;
  FixMap["insertPBN"] = BRIDGE_REF_INSERT_PBN;
  FixMap["deletePBN"] = BRIDGE_REF_DELETE_PBN;

  FixMap["replaceRBN"] = BRIDGE_REF_REPLACE_RBN;
  FixMap["insertRBN"] = BRIDGE_REF_INSERT_RBN;
  FixMap["deleteRBN"] = BRIDGE_REF_DELETE_RBN;

  FixMap["replaceRBX"] = BRIDGE_REF_REPLACE_RBX;
  FixMap["insertRBX"] = BRIDGE_REF_INSERT_RBX;
  FixMap["deleteRBX"] = BRIDGE_REF_DELETE_RBX;

  FixMap["replaceTXT"] = BRIDGE_REF_REPLACE_TXT;
  FixMap["insertTXT"] = BRIDGE_REF_INSERT_TXT;
  FixMap["deleteTXT"] = BRIDGE_REF_DELETE_TXT;

  FixMap["replaceWORD"] = BRIDGE_REF_REPLACE_WORD;
  FixMap["insertWORD"] = BRIDGE_REF_INSERT_WORD;
  FixMap["deleteWORD"] = BRIDGE_REF_DELETE_WORD;

  for (auto &s: FixMap)
    FixTable[s.second] = s.first;
}


void RefLine::setDispatch()
{
  ParseList[BRIDGE_REF_REPLACE_GEN] = &RefLine::parseReplaceGen;
  ParseList[BRIDGE_REF_INSERT_GEN] = &RefLine::parseInsertGen;
  ParseList[BRIDGE_REF_DELETE_GEN] = &RefLine::parseDeleteGen;

  ParseList[BRIDGE_REF_REPLACE_LIN] = &RefLine::parseReplaceLIN;
  ParseList[BRIDGE_REF_INSERT_LIN] = &RefLine::parseInsertLIN;
  ParseList[BRIDGE_REF_DELETE_LIN] = &RefLine::parseDeleteLIN;

  ParseList[BRIDGE_REF_REPLACE_PBN] = &RefLine::parseReplacePBN;
  ParseList[BRIDGE_REF_INSERT_PBN] = &RefLine::parseInsertPBN;
  ParseList[BRIDGE_REF_DELETE_PBN] = &RefLine::parseDeletePBN;

  ParseList[BRIDGE_REF_REPLACE_RBN] = &RefLine::parseReplaceRBN;
  ParseList[BRIDGE_REF_INSERT_RBN] = &RefLine::parseInsertRBN;
  ParseList[BRIDGE_REF_DELETE_RBN] = &RefLine::parseDeleteRBN;

  ParseList[BRIDGE_REF_REPLACE_RBX] = &RefLine::parseReplaceRBN;
  ParseList[BRIDGE_REF_INSERT_RBX] = &RefLine::parseInsertRBN;
  ParseList[BRIDGE_REF_DELETE_RBX] = &RefLine::parseDeleteRBN;

  ParseList[BRIDGE_REF_REPLACE_TXT] = &RefLine::parseReplaceTXT;
  ParseList[BRIDGE_REF_INSERT_TXT] = &RefLine::parseInsertTXT;
  ParseList[BRIDGE_REF_DELETE_TXT] = &RefLine::parseDeleteTXT;

  ParseList[BRIDGE_REF_REPLACE_WORD] = &RefLine::parseReplaceWORD;
  ParseList[BRIDGE_REF_INSERT_WORD] = &RefLine::parseInsertWORD;
  ParseList[BRIDGE_REF_DELETE_WORD] = &RefLine::parseDeleteWORD;
}


void RefLine::setTables()
{
  RefLine::setFixTables();
  RefLine::setDispatch();
}


bool RefLine::isSpecial(const string& word) const
{
  if (word == "orderCOCO" ||
      word == "orderOOCC" ||
      word == "skip" ||
      word == "noval" ||
      word == "results")
    return true;
  else
    return false;
}


void RefLine::parseRange(
  const string& refName,
  const string& line,
  const string& rtext,
  const unsigned start,
  const unsigned end)
{
  // Start with either upos (unsigned positive number) or upos-upos.
  // Set lno and count.

  if (start >= end)
    THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

  unsigned dash = rtext.find("-");
  if (dash == string::npos || dash == rtext.length()-1)
  {
    // upos.
    if (! str2upos(rtext, range.lno))
      THROW("Ref file " + refName + ": No line number in '" + line + "'");
    range.lcount = 1;
  }
  else
  {
    // upos-upos.
    if (! str2upos(rtext, range.lno))
      THROW("Ref file " + refName + ": No line number in '" + line + "'");
    unsigned c;
    if (! str2upos(rtext.substr(dash+1), c))
      THROW("Ref file " + refName + ": No end line in '" + line + "'");
    if (c <= range.lno)
      THROW("Ref file " + refName + ": Bad range in '" + line + "'");
    range.lcount = c + 1 - range.lno;
  }
}


FixType RefLine::parseAction(
  const string& refName,
  const string& line,
  const string& action)
{
  // Set fix.

  auto it = FixMap.find(action);
  if (it == FixMap.end())
    THROW("Ref file " + refName + ": Bad action in '" + line + "'");

  return it->second;
}


bool RefLine::parse(
  const string& refName,
  const string& line)
{
  if (setFlag)
    THROW("RefLine already set: " + line);

  inputLine = line;

  string r, a, q;
  unsigned start = 0;
  unsigned end = line.length()-1;

  if (! readNextWord(line, 0, r))
    THROW("Ref file " + refName + ": Line start '" + line + "'");

  // If the first word is special, we kick it back upstairs.
  if (RefLine::isSpecial(r))
    return false;

  // Otherwise it should be a number or a range.
  start = r.length()+1;
  RefLine::parseRange(refName, line, r, start, end);

  // Then the action word, e.g. replaceLIN.
  if (! readNextWord(line, start, a))
    THROW("Ref file " + refName + ": No action in '" + line + "'");

  start += a.length()+1;
  fix = RefLine::parseAction(refName, line, a);

  // Check whether there is a comment.
  comment.parse(refName, line, start, end);

  if (start >= end)
  {
    setFlag = true;
    return true;
  }

  if (line.at(start) != '"')
    THROW("Ref file " + refName + ": No opening quote in '" + line + "'");

  while (line.at(end) != '"')
    end--;

  if (start == end)
    THROW("Ref file " + refName + ": No closing quote in '" + line + "'");

  // The details of the quoted string depend heavily on the action.
  q = line.substr(start+1, end-start-1);
  (this->*ParseList[fix])(refName, q);

  setFlag = true;
  return true;
}


void RefLine::parseFlexibleNumber(
  const string& refName,
  const string& field)
{
  unsigned tno;
  if (field.at(0) == '-')
  {
    // Permit tag counts from the back of the line as well.
    edit.setReverse();
    if (! str2upos(field.substr(1), tno))
      THROW("Ref file " + refName + ": Bad negative tag '" + field + "'");
  }
  else if (! str2upos(field, tno))
    THROW("Ref file " + refName + ": Bad tag number '" + field + "'");

  edit.setTagNumber(tno);
}


string RefLine::unquote(const string& entry) const
{
  const unsigned l = entry.length();
  if (l == 0)
    return "";

  if (entry.at(0) == '\'')
  {
    if (l == 1)
      THROW("Single quote");
    if (entry.at(l-1) != '\'')
      THROW("Not ending on single quote");
    if (l == 2)
      return "";
    else
      return entry.substr(1, l-2);
  }
  else
    return entry;
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseGen functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void RefLine::parseReplaceGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": replace line count '" + quote + "'");

  comment.checkAction(fix);
  edit.setIs(quote);
}


void RefLine::parseInsertGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": insert line count '" + quote + "'");

  comment.checkAction(fix);
  edit.setIs(quote);
}


void RefLine::parseDeleteGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": delete line count '" + quote + "'");

  comment.checkAction(fix);
  edit.setIs(quote);
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseLIN functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::parseReplaceLIN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen <= 1)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  RefLine::parseFlexibleNumber(refName, v[0]);

  unsigned n = 1;
  unsigned fno;
  if (str2upos(v[1], fno))
  {
    edit.setFieldNumber(fno);
    n = 2;
  }

  if (vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[n]);
  edit.setTag(v[n]);
  edit.setWas(RefLine::unquote(v[n+1]));
  edit.setIs(RefLine::unquote(v[n+2]));
}


void RefLine::parseInsertLIN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen <= 1)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  RefLine::parseFlexibleNumber(refName, v[0]);

  unsigned n = 1;
  unsigned fno;
  if (str2upos(v[1], fno))
  {
    edit.setFieldNumber(fno);
    n = 2;
  }

  if (vlen == n+1)
  {
    // Short version.  The inserted value may or may not be a tag.
    edit.setIs(RefLine::unquote(v[n]));
    return;
  }
  else if (vlen == n+2)
  {
    // Long version, "2,mb,p".
    comment.checkAction(fix);
    comment.checkTag(v[n]);
    edit.setTag(v[n]);
    edit.setIs(RefLine::unquote(v[n+1]));
  }
  else
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");
}


void RefLine::parseDeleteLIN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen == 0)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  RefLine::parseFlexibleNumber(refName, v[0]);

  // delete "3" is allowed for desperate cases.
  if (vlen == 1)
    return;

  unsigned n = 1;
  unsigned fno;
  if (str2upos(v[1], fno))
  {
    edit.setFieldNumber(fno);
    n = 2;
  }

  if (vlen == n+1)
  {
    // Special case "deleteLIN "1,general text", for serious cases.
    comment.checkAction(fix);
    edit.setWas(RefLine::unquote(v[n]));
    return;
  }

  if (vlen != n+2 && vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[n]);
  edit.setTag(v[n]);
  edit.setWas(RefLine::unquote(v[n+1]));

  // Kludge to recognize the case where an empty field is deleted.
  edit.setIs(edit.was() == "" ? "non-empty" : "");

  // deleteLIN "1,7,rs,3NW,4"
  // deleteLIN "3,mb,p,2"
  if (vlen == n+2)
    return;

  unsigned tc;
  if (str2upos(v[n+2], tc))
    edit.setTagCount(tc);
  else
    THROW("Ref file " + refName + ": Bad tag/field count '" + quote + "'");
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// parsePBN functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::parseReplacePBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[0]);
  edit.setTag(v[0]);
  edit.setWas(v[1]);
  edit.setIs(v[2]);
}


void RefLine::parseInsertPBN(
  const string& refName,
  const string& quote)
{
  const size_t pos = quote.find(",");
  if (pos == 0 || pos == string::npos)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  const string t = quote.substr(0, pos);
  const string v = quote.substr(pos+1);
  comment.checkAction(fix);
  comment.checkTag(t);
  edit.setTag(t);
  edit.setIs(v);
}


void RefLine::parseDeletePBN(
  const string& refName,
  const string& quote)
{
  const size_t pos = quote.find(",");
  if (pos == 0 || pos == string::npos)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  const string t = quote.substr(0, pos);
  const string v = quote.substr(pos+1);
  comment.checkAction(fix);
  comment.checkTag(t);
  edit.setTag(t);
  edit.setWas(v);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseRBN functions (also RBX)                                      //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::parseReplaceRBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3 && vlen != 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[0]);
  edit.setTag(v[0]);
  if (vlen == 3)
  {
    edit.setWas(v[1]);
    edit.setIs(v[2]);
  }
  else
  {
    unsigned fno;
    if (! str2upos(v[1], fno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.setFieldNumber(fno);

    edit.setWas(v[2]);
    edit.setIs(v[3]);
  }
}


void RefLine::parseInsertRBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3 && vlen != 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[0]);
  edit.setTag(v[0]);

  if (vlen == 3)
  {
    unsigned fno;
    if (! str2upos(v[1], fno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.setFieldNumber(fno);

    edit.setIs(v[2]);
  }
  else
  {
    // N,7,3,C.  Should only be used for RBX.
    unsigned u;
    if (! str2upos(v[1], u))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.setTagNumber(u);

    if (! str2upos(v[2], u))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.setFieldNumber(u);

    edit.setIs(v[3]);
  }
}


void RefLine::parseDeleteRBN(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen < 1 && vlen > 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[0]);
  edit.setTag(v[0]);
  if (vlen == 2)
    edit.setWas(v[1]);
  else if (vlen == 3)
  {
    unsigned fno;
    if (! str2upos(v[1], fno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.setFieldNumber(fno);
    edit.setWas(v[2]);
  }
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseTXT functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void RefLine::parseReplaceTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen < 2 && vlen > 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  if (vlen == 3)
  {
    unsigned cno;
    if (! str2upos(v[0], cno))
      THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

    edit.setCharNumber(cno);
    edit.setWas(v[1]);
    edit.setIs(v[2]);
  }
  else
  {
    edit.setWas(v[0]);
    edit.setIs(v[1]);
  }
}


void RefLine::parseInsertTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  unsigned cno;
  if (! str2upos(v[0], cno))
    THROW("Ref file " + refName + ": Bad charno '" + quote + "'");
  edit.setCharNumber(cno);

  edit.setIs(v[1]);
}


void RefLine::parseDeleteTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 1 && vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  if (vlen == 1)
    edit.setIs(v[0]);
  else
  {
    unsigned cno;
    if (! str2upos(v[0], cno))
      THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

    edit.setCharNumber(cno);
    edit.setIs(v[1]);
  }
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseWORD functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void RefLine::parseReplaceWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  unsigned tno;
  if (! str2upos(v[0], tno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
    
  edit.setTagNumber(tno);
  edit.setWas(v[1]);
  edit.setIs(v[2]);
}


void RefLine::parseInsertWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[0]);

  unsigned tno;
  if (! str2upos(v[0], tno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");

  edit.setTagNumber(tno);
  edit.setIs(v[1]);
}


void RefLine::parseDeleteWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const unsigned vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[0]);

  unsigned tno;
  if (! str2upos(v[0], tno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");

  edit.setTagNumber(tno);
  edit.setWas(v[1]);
}


unsigned RefLine::lineno() const
{
  return range.lno;
}


bool RefLine::isSet() const
{
  return setFlag;
}


string RefLine::line() const
{
  return inputLine;
}


bool RefLine::isCommented() const
{
  return comment.isCommented();
}


string RefLine::tag() const
{
  return edit.tag();
}


string RefLine::is() const
{
  return edit.is();
}


string RefLine::was() const
{
  return edit.was();
}


bool RefLine::isUncommented() const
{
  return comment.isUncommented();
}


RefCommentCategory RefLine::type() const
{
  if (fix == BRIDGE_REF_DELETE_GEN ||
      fix == BRIDGE_REF_DELETE_PBN ||
      fix == BRIDGE_REF_DELETE_RBN)
    return REF_COMMENT_DELETE_LINE;
  else if (fix == BRIDGE_REF_INSERT_GEN ||
      fix == BRIDGE_REF_INSERT_PBN)
    return REF_COMMENT_INSERT_LINE;
  else if (fix == BRIDGE_REF_FIX_SIZE)
    return REF_COMMENT_ERROR;
  else
    return REF_COMMENT_GENERAL;
}


unsigned RefLine::deletion() const
{
  return range.lcount;
}


void RefLine::modify(string& line) const
{
  if (! setFlag)
    THROW("RefLine not set: " + line);

  edit.modify(line, fix);
}


string RefLine::str() const
{
  if (! setFlag)
    return "RefLine not set\n";
  if (range.lno == 0)
    return "Line number not set\n";
    
  stringstream ss;
  ss << setw(14) << left << "Action" << FixTable[fix] << "\n";

  if (range.lcount <= 1)
    ss << setw(14) << "Line number" << range.lno << "\n";
  else
    ss << setw(14) << "Lines" << range.lno << " to " << 
      range.lno + range.lcount - 1 << "\n";

  ss << edit.str();

  if (comment.isCommented())
    ss << comment.str();

  return ss.str();
}

