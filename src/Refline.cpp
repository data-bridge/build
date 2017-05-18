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

typedef void (RefLine::*ModifyPtr)(string& line) const;
static ModifyPtr ModifyList[BRIDGE_REF_FIX_SIZE];

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

  edit.type = EDIT_TYPE_SIZE;
  edit.tagno = 0;
  edit.reverseFlag = false;
  edit.tagcount = 0;
  edit.fieldno = 0;
  edit.charno = 0;
  edit.was = "";
  edit.is = "";

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


  ModifyList[BRIDGE_REF_REPLACE_GEN] = &RefLine::modifyReplaceGen;
  ModifyList[BRIDGE_REF_INSERT_GEN] = &RefLine::modifyInsertGen;
  ModifyList[BRIDGE_REF_DELETE_GEN] = &RefLine::modifyDeleteGen;

  ModifyList[BRIDGE_REF_REPLACE_LIN] = &RefLine::modifyReplaceLIN;
  ModifyList[BRIDGE_REF_INSERT_LIN] = &RefLine::modifyInsertLIN;
  ModifyList[BRIDGE_REF_DELETE_LIN] = &RefLine::modifyDeleteLIN;

  ModifyList[BRIDGE_REF_REPLACE_PBN] = &RefLine::modifyReplacePBN;
  ModifyList[BRIDGE_REF_INSERT_PBN] = &RefLine::modifyInsertPBN;
  ModifyList[BRIDGE_REF_DELETE_PBN] = &RefLine::modifyDeletePBN;

  ModifyList[BRIDGE_REF_REPLACE_RBN] = &RefLine::modifyReplaceRBN;
  ModifyList[BRIDGE_REF_INSERT_RBN] = &RefLine::modifyInsertRBN;
  ModifyList[BRIDGE_REF_DELETE_RBN] = &RefLine::modifyDeleteRBN;

  ModifyList[BRIDGE_REF_REPLACE_RBX] = &RefLine::modifyReplaceRBX;
  ModifyList[BRIDGE_REF_INSERT_RBX] = &RefLine::modifyInsertRBX;
  ModifyList[BRIDGE_REF_DELETE_RBX] = &RefLine::modifyDeleteRBX;

  ModifyList[BRIDGE_REF_REPLACE_TXT] = &RefLine::modifyReplaceTXT;
  ModifyList[BRIDGE_REF_INSERT_TXT] = &RefLine::modifyInsertTXT;
  ModifyList[BRIDGE_REF_DELETE_TXT] = &RefLine::modifyDeleteTXT;

  ModifyList[BRIDGE_REF_REPLACE_WORD] = &RefLine::modifyReplaceWORD;
  ModifyList[BRIDGE_REF_INSERT_WORD] = &RefLine::modifyInsertWORD;
  ModifyList[BRIDGE_REF_DELETE_WORD] = &RefLine::modifyDeleteWORD;
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
  if (field.at(0) == '-')
  {
    // Permit tag counts from the back of the line as well.
    edit.reverseFlag = true;
    if (! str2upos(field.substr(1), edit.tagno))
      THROW("Ref file " + refName + ": Bad negative tag '" + field + "'");
  }
  else if (! str2upos(field, edit.tagno))
    THROW("Ref file " + refName + ": Bad tag number '" + field + "'");
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

  edit.is = quote;
}


void RefLine::parseInsertGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": insert line count '" + quote + "'");

  comment.checkAction(fix);

  edit.is = quote;
}


void RefLine::parseDeleteGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": delete line count '" + quote + "'");

  comment.checkAction(fix);

  edit.is = quote;
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
  if (str2upos(v[1], edit.fieldno))
    n = 2;

  if (vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[n]);
  edit.type = EDIT_TAG_ONLY;
  edit.tag = v[n];
  edit.was = RefLine::unquote(v[n+1]);
  edit.is = RefLine::unquote(v[n+2]);
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
  if (str2upos(v[1], edit.fieldno))
    n = 2;

  if (vlen == n+1)
  {
    // Short version.  The inserted value may or may not be a tag.
    edit.type = EDIT_TAG_ONLY;
    edit.is = RefLine::unquote(v[n]);
    return;
  }
  else if (vlen == n+2)
  {
    // Long version, "2,mb,p".
    comment.checkAction(fix);
    comment.checkTag(v[n]);
    edit.type = EDIT_TAG_ONLY;
    edit.tag = v[n];
    edit.is = RefLine::unquote(v[n+1]);
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
  if (str2upos(v[1], edit.fieldno))
    n = 2;

  if (vlen == n+1)
  {
    // Special case "deleteLIN "1,general text", for serious cases.
    comment.checkAction(fix);
    edit.was = RefLine::unquote(v[n]);
    return;
  }

  if (vlen != n+2 && vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(fix);
  comment.checkTag(v[n]);
  edit.tag = v[n];
  edit.was = RefLine::unquote(v[n+1]);

  // Kludge to recognize the case where an empty field is deleted.
  edit.is = (edit.was == "" ? "non-empty" : "");

  // deleteLIN "1,7,rs,3NW,4"
  // deleteLIN "3,mb,p,2"
  if (vlen == n+2)
    edit.type = EDIT_TAG_ONLY;
  else if (str2upos(v[n+2], edit.tagcount))
    edit.type = EDIT_TAG_FIELD;
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
  edit.type = EDIT_TAG_ONLY;
  edit.tag = v[0];
  edit.was = v[1];
  edit.is = v[2];
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
  edit.type = EDIT_TAG_ONLY;
  edit.tag = t;
  edit.is = v;
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
  edit.type = EDIT_TAG_ONLY;
  edit.tag = t;
  edit.was = v;
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
  edit.tag = v[0];
  if (vlen == 3)
  {
    edit.type = EDIT_TAG_ONLY;
    edit.was = v[1];
    edit.is = v[2];
  }
  else
  {
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");

    edit.was = v[2];
    edit.is = v[3];
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
  edit.tag = v[0];

  if (vlen == 3)
  {
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");

    edit.is = v[2];
  }
  else
  {
    // N,7,3,C.  Should only be used for RBX.
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.tagno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    if (! str2upos(v[2], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");

    edit.is = v[3];
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
  edit.type = EDIT_TAG_FIELD;
  edit.tag = v[0];
  if (vlen == 2)
    edit.was = v[1];
  else if (vlen == 3)
  {
    edit.type = EDIT_TAG_FIELD;
    if (! str2upos(v[1], edit.fieldno))
      THROW("Ref file " + refName + ": Bad field '" + quote + "'");
    edit.was = v[2];
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

  edit.type = EDIT_CHAR;
  if (vlen == 3)
  {
    if (! str2upos(v[0], edit.charno))
      THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

    edit.was = v[1];
    edit.is = v[2];
  }
  else
  {
    edit.was = v[0];
    edit.is = v[1];
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

  edit.type = EDIT_CHAR;
  if (! str2upos(v[0], edit.charno))
    THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

  edit.is = v[1];
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

  edit.type = EDIT_CHAR;
  if (vlen == 1)
    edit.is = v[0];
  else
  {
    if (! str2upos(v[0], edit.charno))
      THROW("Ref file " + refName + ": Bad charno '" + quote + "'");

    edit.is = v[1];
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
  edit.type = EDIT_WORD;
  if (! str2upos(v[0], edit.tagno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
    
  edit.was = v[1];
  edit.is = v[2];
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
  edit.type = EDIT_WORD;
  if (! str2upos(v[0], edit.tagno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
  edit.is = v[1];
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
  edit.type = EDIT_WORD;
  if (! str2upos(v[0], edit.tagno))
    THROW("Ref file " + refName + ": Not a word number '" + quote + "'");
  edit.was = v[1];
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
  return edit.tag;
}


string RefLine::is() const
{
  return edit.is;
}


string RefLine::was() const
{
  return edit.was;
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


void RefLine::modifyFail(
  const string& line,
  const string& reason) const
{
   THROW("Modify fail: " + line + "\n" + reason + "\n\n" + RefLine::str());
}


void RefLine::modify(string& line) const
{
  if (! setFlag)
    THROW("RefLine not set: " + line);

  (this->* ModifyList[fix])(line);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyGen functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::modifyReplaceGen(string& line) const
{
  line = edit.is;
}


void RefLine::modifyInsertGen(string& line) const
{
  line = edit.is;
}


void RefLine::modifyDeleteGen(string& line) const
{
  UNUSED(line);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyLIN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::modifyLINCommon(
  const string& line,
  unsigned& start,
  vector<string>& v,
  vector<string>& f,
  bool& endsOnPipe) const
{
  v.clear();
  tokenizeMinus(line, v, "|");
  const unsigned vlen = v.size();

  if (edit.tagno == 0)
    modifyFail(line, "No tag number");

  if (2 * edit.tagno > vlen)
  {
    if (edit.is == "" && 
       edit.tag == "" && 
       2 * edit.tagno == vlen+1)
    {
      // Last tag, no argument.
    }
    else
      RefLine::modifyFail(line, "Tag number too large");
  }

  endsOnPipe = (line.length() > 0 && line.at(line.length()-1) == '|');

  if (edit.reverseFlag)
    start = vlen - 2 * edit.tagno;
  else
    start = 2 * (edit.tagno-1);

  if (edit.fieldno > 0)
  {
    f.clear();
    tokenize(v[start+1], f, ",");

    if (edit.fieldno >= f.size()+1)
    {
      if (fix != BRIDGE_REF_INSERT_LIN)
        modifyFail(line, "Field too far");
      else if (edit.fieldno >= f.size()+2)
        modifyFail(line, "Insert field too far");
    }
  }
}


void RefLine::modifyReplaceLIN(string& line) const
{
  unsigned start;
  vector<string> v, f;
  bool endsOnPipe;
  RefLine::modifyLINCommon(line, start, v, f, endsOnPipe);

  if (v[start] != edit.tag)
    modifyFail(line, "Different LIN tags");

  if (edit.fieldno > 0)
  {
    if (f[edit.fieldno-1] != edit.was)
      modifyFail(line, "Old field wrong");

    f[edit.fieldno-1] = edit.is;
    v[start+1] = concat(f, ",");
  }
  else
  {
    if (v[start+1] != edit.was)
    {
      // Permit a not too short prefix.
      const unsigned l = edit.was.length();
      if (l < 2 ||
          l >= v[start+1].length() ||
          v[start+1].substr(0, l) != edit.was)
        modifyFail(line, "Old value wrong: " + v[start+1]);
    }
    v[start+1] = edit.is;
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


void RefLine::modifyInsertLIN(string& line) const
{
  unsigned start;
  vector<string> v, f;
  bool endsOnPipe;
  RefLine::modifyLINCommon(line, start, v, f, endsOnPipe);

  if (edit.fieldno > 0)
  {
    if (v[start] != edit.tag)
      modifyFail(line, "Different LIN tags");

    f.insert(f.begin() + static_cast<int>(edit.fieldno-1), edit.is);
    v[start+1] = concat(f, ",");
  }
  else if (edit.tag == "")
  {
    // Single insertion, i.e. could be a tag or a value.
    v.insert(v.begin() + static_cast<int>(start), edit.is);
  }
  else
  {
    v.insert(v.begin() + static_cast<int>(start), edit.is);
    v.insert(v.begin() + static_cast<int>(start), edit.tag);
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


void RefLine::modifyDeleteLIN(string& line) const
{
  unsigned start;
  vector<string> v, f;
  bool endsOnPipe;
  RefLine::modifyLINCommon(line, start, v, f, endsOnPipe);

  if (edit.fieldno > 0)
  {
    if (v[start] != edit.tag)
      modifyFail(line, "Different LIN tags");

    if (f[edit.fieldno-1] != edit.was)
      modifyFail(line, "Old field wrong");

    auto sf = f.begin() + static_cast<int>(edit.fieldno-1);
    const unsigned d = (edit.tagcount == 0 ? 1 : edit.tagcount);
    f.erase(sf, sf + static_cast<int>(d));

    v[start+1] = concat(f, ",");
  }
  else if (edit.fieldno == 0 && edit.tag == "")
  {
    // Delete a single entry without checking it.
    // Only use this when the entry is seriously messed up.
    if (edit.was != "" && v[start] != edit.was)
      modifyFail(line, "Old value wrong");
      
    v.erase(v.begin() + static_cast<int>(start));
  }
  else
  {
    if (v[start] != edit.tag)
      modifyFail(line, "Different LIN tags");
    
    if (edit.is == "" && edit.tag == "")
    {
      // Delete a single entry without checking it.
      // Only use this when the entry is seriously messed up.
      v.erase(v.begin() + static_cast<int>(start));
    }
    else
    {
      if (v[start+1] != edit.was)
        modifyFail(line, "Old value wrong");
      
      auto s = v.begin() + static_cast<int>(start);
      const unsigned d = (edit.tagcount == 0 ? 2 : 2*edit.tagcount);
      v.erase(s, s + static_cast<int>(d));
    }
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyPBN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::modifyReplacePBN(string& line) const
{
  const regex rep("^\\[(\\w+)\\s+\"(.*)\"\\]\\s*$");
  smatch match;
  if (! regex_search(line, match, rep))
    THROW("Bad PBN line: " + line);

  if (edit.tag != match.str(1))
    modifyFail(line, "Different PBN tags");

  if (edit.was != match.str(2))
    modifyFail(line, "Old value wrong");

  line = "[" + edit.tag + " \"" + edit.is + "\"" + "]";
}


void RefLine::modifyInsertPBN(string& line) const
{
  line = "[" + edit.tag + " \"" + edit.is + "\"" + "]";
}


void RefLine::modifyDeletePBN(string& line) const
{
  const regex rep("^\\[(\\w+)\\s+\"(.*)\"\\]\\s*$");
  smatch match;
  if (! regex_search(line, match, rep))
    THROW("Bad PBN line: " + line);

  if (edit.tag != match.str(1))
    modifyFail(line, "Different PBN tags");

  if (edit.was != match.str(2))
    modifyFail(line, "Old value wrong");

  line = "";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyRBN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefLine::modifyRBNCommon(
  const string& line,
  string& s) const
{
  const unsigned l = line.length();
  if (l == 0)
    THROW("RBN line too short: " + line);

  if (line.substr(0, 1) != edit.tag)
    THROW("RBN tag wrong: " + line);

  if (l > 1 && line.at(1) != ' ')
    THROW("RBN syntax: " + line);

  if (l <= 2)
    s = "";
  else
    s = line.substr(2);
}


void RefLine::modifyReplaceRBN(string& line) const
{
  string s;
  RefLine::modifyRBNCommon(line, s);

  if (edit.fieldno == 0)
  {
    // R,5+400:+14,6+420:14
    if (s != edit.was)
      THROW("RBN old value: " + line);
    line = edit.tag + " " + edit.is;
  }
  else
  {
    // P,2,SJ235,SJ234
    vector<string> f;
    f.clear();
    tokenize(s, f, ":");
    const unsigned flen = f.size();
    if (edit.fieldno > flen)
      THROW("RBN field number too large: " + line);
    if (f[edit.fieldno-1] != edit.was)
      THROW("RBN old field value: " + line);
    f[edit.fieldno-1] = edit.is;
    line = edit.tag + " " + concat(f, ":");
  }
}

void RefLine::modifyInsertRBN(string& line) const
{
  string s;
  RefLine::modifyRBNCommon(line, s);

  if (edit.fieldno == 0)
    THROW("Not a field insertion: " + line);

  vector<string> f;
  f.clear();
  tokenize(s, f, ":");
  const unsigned flen = f.size();
  if (edit.fieldno > flen+1)
    THROW("RBN field number too large: " + line);
  else if (edit.fieldno == flen+1)
    f.push_back(edit.is);
  else
    f.insert(f.begin() + static_cast<int>(edit.fieldno-1), edit.is);
  line = edit.tag + " " + concat(f, ":");
}


void RefLine::modifyDeleteRBN(string& line) const
{
  string s;
  if (line != "" || edit.was != "")
    RefLine::modifyRBNCommon(line, s);

  if (edit.fieldno == 0)
  {
    // Could delete without checking.  Only use in dire cases.
    if (edit.was != "" && s != edit.was)
      THROW("RBN old value: " + line);
    line = ""; 
  }
  else
  {
    vector<string> f;
    f.clear();
    tokenize(s, f, ":");
    const unsigned flen = f.size();
    if (edit.fieldno > flen)
      THROW("RBN field number too large: " + line);
    if (f[edit.fieldno-1] != edit.was)
      THROW("RBN old field value: " + line);
    f.erase(f.begin() + static_cast<int>(edit.fieldno-1));
    line = edit.tag + " " + concat(f, ":");
  }
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyRBX functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


bool RefLine::modifyCommonRBX(
  const string& line,
  vector<string>& v,
  string& s,
  unsigned& pos) const
{
  v.clear();
  tokenize(line, v, "}");
  v.pop_back(); // Last empty field
  const unsigned vlen = v.size();

  for (unsigned i = 0; i < vlen; i++)
  {
    const unsigned flen = v[i].length();
    if (flen <= 1)
      continue;

    if (v[i].substr(0, 1) == edit.tag && v[i].at(1) == '{')
    {
      pos = i;
      if (flen == 2)
        s = "";
      else
        s = edit.tag + " " + v[i].substr(2);
      return true;
    }
  }
  return false;
}


void RefLine::modifyReplaceRBX(string& line) const
{
  vector<string> v;
  string s;
  unsigned pos;

  if (! RefLine::modifyCommonRBX(line, v, s, pos))
    THROW("RBN tag not found: " + line);

  RefLine::modifyReplaceRBN(s);
  if (s.length() <= 2)
    v[pos] = edit.tag + "{";
  else
    v[pos] = edit.tag + "{" + s.substr(2);

  line = concat(v, "}") + "}";
}


void RefLine::modifyInsertRBX(string& line) const
{
  vector<string> v;
  string s;
  unsigned pos;

  if (RefLine::modifyCommonRBX(line, v, s, pos))
  {
    // Insertion of a field in an existing tag.
    if (edit.fieldno == 0)
      THROW("RBX tag already present: " + line);
    RefLine::modifyInsertRBN(s);
    if (s.length() <= 2)
      v[pos] = edit.tag + "{";
    else
      v[pos] = edit.tag + "{" + s.substr(2);
  }
  else
  {
    // Insertion of a new tag.
    if (edit.tagno > v.size()+1)
      THROW("RBX tag number too large: " + line);
    if (edit.tagno == 0)
      THROW("RBX tag number too small: " + line);
      
    s = edit.tag + "{" + edit.is;
    if (edit.tagno == v.size()+1)
      v.push_back(s);
    else
      v.insert(v.begin() + static_cast<int>(edit.tagno-1), s);
  }
  line = concat(v, "}") + "}";
}


void RefLine::modifyDeleteRBX(string& line) const
{
  vector<string> v;
  string s;
  unsigned pos;

  if (! RefLine::modifyCommonRBX(line, v, s, pos))
    THROW("RBN tag not found: " + line);

  RefLine::modifyDeleteRBN(s);
  v.erase(v.begin() + static_cast<int>(pos));

  line = concat(v, "}") + "}";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyTXT functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

unsigned RefLine::modifyCommonTXT(const string& line) const
{
  if (edit.charno == 0)
  {
    const unsigned p = line.find(edit.was);
    if (p == string::npos)
      return 0;
    else
      return p+1;
  }
  else
    return edit.charno; // As already 1-based
}


void RefLine::modifyReplaceTXT(string& line) const
{
  const unsigned p = modifyCommonTXT(line);
  if (p == 0)
    THROW("No character position and string not found: " + line);

  unsigned lw = edit.was.length();
  unsigned li = edit.is.length();

  if (line.substr(p-1, lw) != edit.was)
    THROW("Old TXT value: " + line.substr(p-1, lw) + " vs " + edit.was);

  if (lw > li)
    line.erase(p-1, lw-li);
  else if (lw < li)
    line.insert(p-1, " ", li-lw);

  line.replace(p-1, li, edit.is);
}


void RefLine::modifyInsertTXT(string& line) const
{
  if (edit.charno == 0)
    THROW("No character position");

  if (edit.charno > line.length())
    THROW("Character position too large");
  else if (edit.charno == line.length())
    line += edit.is;
  else
    line.insert(edit.charno, edit.is);
}


void RefLine::modifyDeleteTXT(string& line) const
{
  const unsigned p = modifyCommonTXT(line);
  if (p == 0)
    THROW("No character position and string not found");

  if (line.substr(p-1, edit.was.length()) != edit.was)
    THROW("Old TXT value");

  line.erase(p-1, edit.was.length());
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyWORD functions                                               //
//                                                                    //
////////////////////////////////////////////////////////////////////////

unsigned RefLine::modifyCommonWORD(const string& line) const
{
  unsigned pos = 0;
  const unsigned l = line.length();
  for (unsigned wno = 0; wno < edit.tagno; wno++)
  {
    while (pos < l && line.at(pos) == ' ')
      pos++;

    if (wno == edit.tagno-1)
      break;

    while (pos < l && line.at(pos) != ' ')
      pos++;
  }

  if (pos == l)
    return 0;
  else
    return pos+1;
}


void RefLine::modifyReplaceWORD(string& line) const
{
  const unsigned pos = RefLine::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "replaceWORD: Too few words");

  const unsigned lw = edit.was.length();
  const unsigned li = edit.is.length();

  if (line.substr(pos-1, lw) != edit.was)
    modifyFail(line, "Old value wrong");

  if (lw > li)
    line.erase(pos-1, lw-li);
  else if (lw < li)
    line.insert(pos-1, " ", li-lw);

  line.replace(pos-1, li, edit.is);
}


void RefLine::modifyInsertWORD(string& line) const
{
  const unsigned pos = RefLine::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "insertWORD: Too few words");

  line.insert(pos-1, edit.is);
}

void RefLine::modifyDeleteWORD(string& line) const
{
  const unsigned pos = RefLine::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "replaceWORD: Too few words");

  const unsigned lw = edit.was.length();
  if (line.substr(pos-1, lw) != edit.was)
    modifyFail(line, "Old value wrong");

  line.erase(pos-1, lw);
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

  if (edit.reverseFlag)
    ss << setw(14) << "Tag number" << edit.tagno << " (from the back)\n";
  else
    ss << setw(14) << "Tag number" << edit.tagno << "\n";

  ss << setw(14) << "Tag" << edit.tag << "\n";
  if (edit.fieldno > 0)
  {
    if (edit.tagcount == 0)
      ss << setw(14) << "Field" << edit.fieldno << "\n";
    else
      ss << setw(14) << "Fields" << edit.fieldno << " to " <<
        edit.fieldno + edit.tagcount - 1 << "\n";
  }

  if (edit.charno > 0)
    ss << setw(14) << "Char. number" << edit.charno << "\n";
  
  ss << setw(14) << "String was" << "'" << edit.was << "'" << "\n";
  ss << setw(14) << "String is" << "'" << edit.is << "'" << "\n";

  if (comment.isCommented())
    ss << comment.str();

  return ss.str();
}

