/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
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
#pragma warning(pop)

#include "RefLine.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


typedef void (RefLine::*ParsePtr)(
  const string& refName, 
  const string& quote);

static ParsePtr ParseList[REF_ACTION_SIZE];

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
  action.reset();
  edit.reset();
  comment.reset();
  fnameEmbed = "";
  rangeEmbed.lno = 0;
  rangeEmbed.lcount = 0;
}


void RefLine::setTables()
{
  ParseList[REF_ACTION_REPLACE_GEN] = &RefLine::parseReplaceGen;
  ParseList[REF_ACTION_INSERT_GEN] = &RefLine::parseInsertGen;
  ParseList[REF_ACTION_DELETE_GEN] = &RefLine::parseDeleteGen;

  ParseList[REF_ACTION_REPLACE_LIN] = &RefLine::parseReplaceLIN;
  ParseList[REF_ACTION_INSERT_LIN] = &RefLine::parseInsertLIN;
  ParseList[REF_ACTION_DELETE_LIN] = &RefLine::parseDeleteLIN;

  ParseList[REF_ACTION_REPLACE_PBN] = &RefLine::parseReplacePBN;
  ParseList[REF_ACTION_INSERT_PBN] = &RefLine::parseInsertPBN;
  ParseList[REF_ACTION_DELETE_PBN] = &RefLine::parseDeletePBN;

  ParseList[REF_ACTION_REPLACE_RBN] = &RefLine::parseReplaceRBN;
  ParseList[REF_ACTION_INSERT_RBN] = &RefLine::parseInsertRBN;
  ParseList[REF_ACTION_DELETE_RBN] = &RefLine::parseDeleteRBN;

  ParseList[REF_ACTION_REPLACE_RBX] = &RefLine::parseReplaceRBN;
  ParseList[REF_ACTION_INSERT_RBX] = &RefLine::parseInsertRBN;
  ParseList[REF_ACTION_DELETE_RBX] = &RefLine::parseDeleteRBN;

  ParseList[REF_ACTION_REPLACE_TXT] = &RefLine::parseReplaceTXT;
  ParseList[REF_ACTION_INSERT_TXT] = &RefLine::parseInsertTXT;
  ParseList[REF_ACTION_DELETE_TXT] = &RefLine::parseDeleteTXT;

  ParseList[REF_ACTION_REPLACE_WORD] = &RefLine::parseReplaceWORD;
  ParseList[REF_ACTION_INSERT_WORD] = &RefLine::parseInsertWORD;
  ParseList[REF_ACTION_DELETE_WORD] = &RefLine::parseDeleteWORD;

  ParseList[REF_ACTION_REPLACE_FROM] = &RefLine::parseReplaceFrom;
  ParseList[REF_ACTION_INSERT_FROM] = &RefLine::parseInsertFrom;

  ParseList[REF_ACTION_DELETE_EML] = &RefLine::parseDeleteEML;
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


unsigned RefLine::parseUpos(
  const string& refName,
  const string& quote,
  const string& str) const
{
  unsigned no;
  if (! str2upos(str, no))
    THROW("Ref file " + refName + ": Not unsigned '" + quote + "'");

  return no;
}


void RefLine::parseRange(
  const string& refName,
  const string& line,
  const string& rtext,
  Range& rangeIn)
{
  // Start with either upos (unsigned positive number) or upos-upos.
  // Set lno and count.

  size_t dash = rtext.find("-");
  if (dash == string::npos || dash == rtext.length()-1)
  {
    // upos.
    if (! str2upos(rtext, rangeIn.lno))
      THROW("Ref file " + refName + ": No line number in '" + line + "'");
    rangeIn.lcount = 1;
  }
  else
  {
    // upos-upos.
    if (! str2upos(rtext, rangeIn.lno))
      THROW("Ref file " + refName + ": No line number in '" + line + "'");
    unsigned c;
    if (! str2upos(rtext.substr(dash+1), c))
      THROW("Ref file " + refName + ": No end line in '" + line + "'");
    if (c <= rangeIn.lno)
      THROW("Ref file " + refName + ": Bad range in '" + line + "'");
    rangeIn.lcount = c + 1 - rangeIn.lno;
  }
}


bool RefLine::parse(
  const string& refName,
  const string& line)
{
  if (setFlag)
    THROW("RefLine already set: " + line);

  inputLine = line;

  string r, a, q;
  size_t start = 0;
  size_t end = line.length()-1;

  if (! readNextWord(line, 0, r))
    THROW("Ref file " + refName + ": Line start '" + line + "'");

  // If the first word is special, we kick it back upstairs.
  if (RefLine::isSpecial(r))
    return false;

  // Otherwise it should be a number or a range.
  start = r.length()+1;
  if (start >= end)
    THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

  RefLine::parseRange(refName, line, r, range);

  // Then the action word, e.g. replaceLIN.
  if (! readNextWord(line, start, a))
    THROW("Ref file " + refName + ": No action in '" + line + "'");

  start += a.length()+1;
  action.set(refName, a);

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
  (this->*ParseList[action.number()])(refName, q);

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
    edit.setReverse();
    edit.setTagNumber(RefLine::parseUpos(refName, field, field.substr(1)));
  }
  else
    edit.setTagNumber(RefLine::parseUpos(refName, field, field));
}


string RefLine::unquote(const string& entry) const
{
  const size_t l = entry.length();
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

  comment.checkAction(action.number());
  edit.setIs(quote);
}


void RefLine::parseInsertGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": insert line count '" + quote + "'");

  comment.checkAction(action.number());
  edit.setIs(quote);
}


void RefLine::parseDeleteGen(
  const string& refName,
  const string& quote)
{
  if (range.lcount != 1)
    THROW("Ref file " + refName + ": delete line count '" + quote + "'");

  comment.checkAction(action.number());
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
  const size_t vlen = v.size();

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

  comment.checkAction(action.number());
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
  const size_t vlen = v.size();

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
    comment.checkAction(action.number());
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
  const size_t vlen = v.size();

  if (vlen == 0)
    THROW("Ref file " + refName + ": Short quotes '" + quote + "'");

  RefLine::parseFlexibleNumber(refName, v[0]);
  edit.setTagCount(1); // Default to be overwritten

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
    comment.checkAction(action.number());
    edit.setWas(RefLine::unquote(v[n]));
    return;
  }

  if (vlen != n+2 && vlen != n+3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  comment.checkTag(v[n]);
  edit.setTag(v[n]);
  edit.setWas(RefLine::unquote(v[n+1]));

  // Kludge to recognize the case where an empty field is deleted.
  edit.setIs(edit.was() == "" ? "non-empty" : "");

  // deleteLIN "1,7,rs,3NW,4"
  // deleteLIN "3,mb,p,2"
  if (vlen == n+2)
    return;

  edit.setTagCount(RefLine::parseUpos(refName, quote, v[n+2]));
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
  const size_t vlen = v.size();

  if (vlen != 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
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
  comment.checkAction(action.number());
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
  comment.checkAction(action.number());
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
  const size_t vlen = v.size();

  if (vlen != 3 && vlen != 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  comment.checkTag(v[0]);
  edit.setTag(v[0]);
  if (vlen == 3)
  {
    edit.setWas(v[1]);
    edit.setIs(v[2]);
  }
  else
  {
    edit.setFieldNumber(RefLine::parseUpos(refName, quote, v[1]));
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
  const size_t vlen = v.size();

  if (vlen != 3 && vlen != 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  comment.checkTag(v[0]);
  edit.setTag(v[0]);

  if (vlen == 3)
  {
    edit.setFieldNumber(RefLine::parseUpos(refName, quote, v[1]));
    edit.setIs(v[2]);
  }
  else
  {
    // N,7,3,C.  Should only be used for RBX.
    edit.setTagNumber(RefLine::parseUpos(refName, quote, v[1]));
    edit.setFieldNumber(RefLine::parseUpos(refName, quote, v[2]));
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
  const size_t vlen = v.size();

  if (vlen < 1 && vlen > 4)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  comment.checkTag(v[0]);
  edit.setTag(v[0]);
  if (vlen == 1)
    return;
  else if (vlen == 2)
    edit.setWas(v[1]);
  else
  {
    edit.setFieldNumber(RefLine::parseUpos(refName, quote, v[1]));
    edit.setWas(v[2]);

    if (vlen == 4)
      edit.setTagCount(RefLine::parseUpos(refName, quote, v[3]));
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
  const size_t vlen = v.size();

  if (vlen < 2 && vlen > 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  if (vlen == 3)
  {
    edit.setCharNumber(RefLine::parseUpos(refName, quote, v[0]));
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
  const size_t vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  edit.setCharNumber(RefLine::parseUpos(refName, quote, v[0]));
  edit.setIs(v[1]);
}


void RefLine::parseDeleteTXT(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const size_t vlen = v.size();

  if (vlen != 1 && vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  if (vlen == 1)
    edit.setWas(v[0]);
  else
  {
    edit.setCharNumber(RefLine::parseUpos(refName, quote, v[0]));
    edit.setWas(v[1]);
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
  const size_t vlen = v.size();

  if (vlen != 3)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  edit.setTagNumber(RefLine::parseUpos(refName, quote, v[0]));
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
  const size_t vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  edit.setTagNumber(RefLine::parseUpos(refName, quote, v[0]));
  edit.setIs(v[1]);
}


void RefLine::parseDeleteWORD(
  const string& refName,
  const string& quote)
{
  vector<string> v;
  v.clear();
  tokenize(quote, v, ",");
  const size_t vlen = v.size();

  if (vlen != 2)
    THROW("Ref file " + refName + ": Wrong-length quotes '" + quote + "'");

  comment.checkAction(action.number());
  edit.setTagNumber(RefLine::parseUpos(refName, quote, v[0]));
  edit.setWas(v[1]);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseFrom functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void RefLine::parseFrom(
  const string& refName,
  const string& quote)
{
  const size_t pos = quote.find_first_of(":");
  if (pos == string::npos)
    THROW("Ref file " + refName + ": No colon in '" + quote + "'");
  
  fnameEmbed = quote.substr(0, pos);
  RefLine::parseRange(refName, quote, quote.substr(pos+1), rangeEmbed);
}


void RefLine::parseReplaceFrom(
  const string& refName,
  const string& quote)
{
  RefLine::parseFrom(refName, quote);
}


void RefLine::parseInsertFrom(
  const string& refName,
  const string& quote)
{
  RefLine::parseFrom(refName, quote);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// parseEML functions                                                 //
//                                                                    //
////////////////////////////////////////////////////////////////////////


void RefLine::parseDeleteEML(
  const string& refName,
  const string& quote)
{
  // Only works for a whole canvas.
  if (range.lcount == 1)
    THROW("Ref file " + refName + ": delete line count '" + quote + "'");

  comment.checkAction(action.number());
  edit.setIs(quote);
}



unsigned RefLine::lineno() const
{
  return range.lno;
}


unsigned RefLine::linenoEmbed() const
{
  return rangeEmbed.lno;
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


string RefLine::embeddedRef() const
{
  return fnameEmbed;
}


bool RefLine::isUncommented() const
{
  return comment.isUncommented();
}


ActionCategory RefLine::type() const
{
  return action.category();
}


unsigned RefLine::rangeCount() const
{
  return range.lcount;
}


unsigned RefLine::rangeEmbedCount() const
{
  return rangeEmbed.lcount;
}


void RefLine::checkEntries(
  const RefEntry& re,
  const RefEntry& ractual) const
{
  if (re.sameCountValues(ractual))
    return;

  THROW(comment.strComment() + ": (" + 
    ractual.strCountShort() + ")");
}


void RefLine::checkCounts() const
{
  CommentType cat;
  RefEntry re, ractual;
  comment.getEntry(cat, re);
  const RefCountType rc = comment.countType();
  const ActionType act = action.number();

  if (rc == REF_COUNT_INACTIVE)
  {
    if (act == REF_ACTION_INSERT_LIN && comment.isTag(edit.is()))
    {
      // This is often of the form insertLIN "2,mb" where edit.is
      // in fact a tag.
      comment.checkTag(edit.is());

      ractual.set(1, 1, 1);
      RefLine::checkEntries(re, ractual);
      return;
    }
    else if ((act == REF_ACTION_DELETE_LIN && edit.is() == "") ||
        act == REF_ACTION_REPLACE_TXT)
    {
      // It may also be deleteLIN "2"
      ractual.set(1, 1, 1);
      RefLine::checkEntries(re, ractual);
      return;
    }
    else
      THROW("checkCounts INACTIVE: " + inputLine);
  }
  else if (rc == REF_COUNT_HEADER)
  {
    // These are done separately from Reflines::checkHeader,
    // as we don't know the global numbers here yet.
  }
  else if (rc == REF_COUNT_SINGLE)
  {
    ractual.set(1, 1, 1);
    RefLine::checkEntries(re, ractual);
  }
  else if (rc == REF_COUNT_LIN_IS)
  {
    size_t u = edit.countUnitsLIN();
    if (action.number() == ACTION_INSERT_LINE)
      u--;

    ractual.set(u, 1, 1);
    RefLine::checkEntries(re, ractual);
  }
  else if (rc == REF_COUNT_LIN_REPEAT)
  {
    ractual.set(edit.repeatCount(), 1, 1);
    RefLine::checkEntries(re, ractual);
  }
  else if (rc == REF_COUNT_LIN_FIELDS)
  {
    const unsigned u = edit.repeatCount();
    if (u == 1)
    {
      ractual.set(1, 1, 1);
    }
    else
    {
      ractual.set(1, u, u/2);
    }
    RefLine::checkEntries(re, ractual);
  }
  else
    THROW("Bad count type: " + to_string(rc) + ", " + inputLine);
}


void RefLine::modify(string& line) const
{
  if (! setFlag)
    THROW("RefLine not set: " + line);

  edit.modify(line, action.number());

  if (comment.isUncommented())
    return;

  RefLine::checkCounts();
}


void RefLine::countHandsLIN(
  const string& line,
  vector<unsigned>& seen) const
{
  size_t pos = 0, p;
  unsigned b;
  while (1)
  {
    p = line.find("qx|", pos);
    if (p == string::npos || p+5 >= line.size())
      break;

    if (! str2upos(line.substr(p+4), b))
      break; // Not a good sign, but let's not throw

    if (b >= seen.size())
      seen.resize(2*seen.size());
    seen[b]++;

    pos = p+3;
  }
}


void RefLine::countHandsPBN(
  const string& line,
  unsigned &h,
  unsigned &b) const
{
  if (line.substr(0, 7) != "[Board ")
    return;

  h++;
  if (line != "[Board \"#\"]")
    b++;
}


void RefLine::countHandsRBN(
  const string& line,
  unsigned &h,
  unsigned &b) const
{
  // Not super-accurate.

  if (line == "")
  {
    h++;
    return;
  }

  if (line.length() <= 2)
    return;

  if (line.substr(0, 2) == "B ")
    b++;
}


void RefLine::countHandsTXT(
  const string& line,
  unsigned &h) const
{
  // Not super-accurate.
  if (line.length() <= 6)
    return;

  if (line.substr(0, 6) == "West  ")
    h++;
}


void RefLine::countHandsEML(
  const string& line,
  unsigned &h) const
{
  // Not super-accurate.
  if (line.length() <= 54)
    return;

  if (line.substr(48, 6) == "Board ")
    h++;
}


void RefLine::countHandsREC(
  const string& line,
  unsigned &h) const
{
  // Not super-accurate.
  if (line.length() <= 8)
    return;

  if (line.substr(0, 6) == "Board ")
    h++;
}


void RefLine::countVector(
  const vector<unsigned>& seen,
  unsigned& h,
  unsigned& b) const
{
  for (unsigned u = 0; u < seen.size(); u++)
  {
    if (seen[u] > 0)
    {
      h += seen[u];
      b++;
    }
  }
}


void RefLine::countHands(
  const vector<string>& lines,
  const Format format,
  unsigned& h,
  unsigned& b) const
{
  h = 0;
  b = 0;
  vector<unsigned> seen;
  seen.resize(64);

  if (format == BRIDGE_FORMAT_LIN)
  {
    for (auto &line: lines)
      RefLine::countHandsLIN(line, seen);
    RefLine::countVector(seen, h, b);
  }
  else if (format == BRIDGE_FORMAT_PBN)
  {
    for (auto &line: lines)
      RefLine::countHandsPBN(line, h, b);
  }
  else if (format == BRIDGE_FORMAT_RBN)
  {
    for (auto &line: lines)
      RefLine::countHandsRBN(line, h, b);
  }
  else if (format == BRIDGE_FORMAT_RBX)
  {
    // Short-cut.
    h = range.lcount;
    b = range.lcount/2;
  }
  else if (format == BRIDGE_FORMAT_TXT)
  {
    for (auto &line: lines)
      RefLine::countHandsTXT(line, h);
    b = h/2;
  }
  else if (format == BRIDGE_FORMAT_EML)
  {
    for (auto &line: lines)
      RefLine::countHandsEML(line, h);
    b = h/2;
  }
  else if (format == BRIDGE_FORMAT_REC)
  {
    for (auto &line: lines)
      RefLine::countHandsREC(line, h);
    b = h/2;
  }
  else
    THROW("Bad format");
}


void RefLine::checkMultiLineCounts(const vector<string>& lines) const
{
  CommentType cat;
  RefEntry re, ractual;
  comment.getEntry(cat, re);
  const RefCountType rc = comment.countType();

  if (rc == REF_COUNT_SINGLE)
  {
    // Convenience for RBN L, shouldn't really happen.
    ractual.set(1, 1, 1);
    RefLine::checkEntries(re, ractual);
  }
  else if (rc == REF_COUNT_HEADER)
  {
    // These are done separately from Reflines::checkHeader,
    // as we don't know the global numbers here yet.
  }
  else if (rc == REF_COUNT_HANDS)
  {
    // The following is not super-accurate.
    const Format format = comment.format();
    unsigned h, b;
    RefLine::countHands(lines, format, h, b);

    if (h <= 1)
    {
      ractual.set(0, 1, 1);
    }
    else
    {
      ractual.set(0, h, b);
    }
    RefLine::checkEntries(re, ractual);
  }
  else
    THROW("Bad count type: " + to_string(rc) + ", " + inputLine);
}


void RefLine::modify(vector<string>& lines) const
{
  RefLine::checkMultiLine(lines);
  edit.modify(lines, action.number());
}


void RefLine::checkMultiLine(const vector<string>& lines) const
{
  if (! setFlag)
    THROW("RefLine not set (multi-line)");

  if (comment.isCommented())
    RefLine::checkMultiLineCounts(lines);
}


void RefLine::checkHeader(
  const RefEntry& ractual,
  map<string, vector<RefEntry>>& cumulCount) const
{
  if (comment.isUncommented() || 
      comment.countType() != REF_COUNT_HEADER)
    return;

  CommentType cat;
  RefEntry re;
  comment.getEntry(cat, re);
  const ActionCategory act = action.category();

  if (act == ACTION_DELETE_LINE)
  {
    // Must be the full count.
    if (re.getUnits() > 1)
      THROW("Line delete should have <= 1 units");

    re.setUnitsFrom(ractual);
    RefLine::checkEntries(re, ractual);
    return;
  }

  // If it fits, we just assume the tag only shows up once.
  //
  // TODO 2023-06-13 It used to be that re.count.units should be 1,
  // and the test was for this.  Now I copy the units from ractual
  // and test sameCountValues against raction.  Not sure this is
  // the same as before?
  re.setUnitsFrom(ractual);
  if (re.sameCountValues(ractual))
    return;

  if (comment.format() != BRIDGE_FORMAT_LIN)
    THROW("Can only count LIN headers here");

  string tag;
  if (action.number() == REF_ACTION_REPLACE_GEN ||
      action.number() == REF_ACTION_INSERT_GEN)
  {
    tag = edit.is().substr(0, 2);
    if (! comment.isTag(tag))
      THROW("Expected a tag leading " + inputLine);
  }
  else
    tag = edit.tag();

  cumulCount[tag].push_back(re);
}


void RefLine::getEntry(
  CommentType& cat,
  RefEntry& re) const
{
  if (! setFlag)
    THROW("RefLine not set");
    
  comment.getEntry(cat, re);
  re.setLines(range.lcount);
}


string RefLine::str() const
{
  if (! setFlag)
    return "RefLine not set\n";
  if (range.lno == 0)
    return "Line number not set\n";
    
  stringstream ss;
  ss << setw(14) << left << "Action" << action.str() << "\n";

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

