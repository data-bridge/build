/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <map>
#include <regex>
#include <mutex>
#include <assert.h>

#include "RefEdit.h"

#include "../util/parse.h"
#include "../handling/Bexcept.h"

using namespace std;


typedef void (RefEdit::*ModifyPtr)(string& line) const;
static ModifyPtr ModifyList[REF_ACTION_SIZE];

static mutex mtx;
static bool setEditTables = false;


RefEdit::RefEdit()
{
  RefEdit::reset();
  if (! setEditTables)
  {
    mtx.lock();
    if (! setEditTables)
      RefEdit::setTables();
    setEditTables = true;
    mtx.unlock();
  }
}


void RefEdit::reset()
{
  tagno = 0;
  reverseFlag = false;
  tagVal = "";
  tagcount = 0;
  fieldno = 0;
  charno = 0;
  wasVal = "";
  isVal = "";

  RefEdit::setCount(0);
}


void RefEdit::setCount(const unsigned v)
{
  assert(v == 0);
  count.reset();
  /*
  count.lines = v;
  count.units = v;
  count.hands = v;
  count.boards = v;
  */
}


void RefEdit::setTables()
{
  ModifyList[REF_ACTION_REPLACE_GEN] = &RefEdit::modifyReplaceGen;
  ModifyList[REF_ACTION_INSERT_GEN] = &RefEdit::modifyInsertGen;
  ModifyList[REF_ACTION_DELETE_GEN] = &RefEdit::modifyDeleteGen;

  ModifyList[REF_ACTION_REPLACE_LIN] = &RefEdit::modifyReplaceLIN;
  ModifyList[REF_ACTION_INSERT_LIN] = &RefEdit::modifyInsertLIN;
  ModifyList[REF_ACTION_DELETE_LIN] = &RefEdit::modifyDeleteLIN;

  ModifyList[REF_ACTION_REPLACE_PBN] = &RefEdit::modifyReplacePBN;
  ModifyList[REF_ACTION_INSERT_PBN] = &RefEdit::modifyInsertPBN;
  ModifyList[REF_ACTION_DELETE_PBN] = &RefEdit::modifyDeletePBN;

  ModifyList[REF_ACTION_REPLACE_RBN] = &RefEdit::modifyReplaceRBN;
  ModifyList[REF_ACTION_INSERT_RBN] = &RefEdit::modifyInsertRBN;
  ModifyList[REF_ACTION_DELETE_RBN] = &RefEdit::modifyDeleteRBN;

  ModifyList[REF_ACTION_REPLACE_RBX] = &RefEdit::modifyReplaceRBX;
  ModifyList[REF_ACTION_INSERT_RBX] = &RefEdit::modifyInsertRBX;
  ModifyList[REF_ACTION_DELETE_RBX] = &RefEdit::modifyDeleteRBX;

  ModifyList[REF_ACTION_REPLACE_TXT] = &RefEdit::modifyReplaceTXT;
  ModifyList[REF_ACTION_INSERT_TXT] = &RefEdit::modifyInsertTXT;
  ModifyList[REF_ACTION_DELETE_TXT] = &RefEdit::modifyDeleteTXT;

  ModifyList[REF_ACTION_REPLACE_WORD] = &RefEdit::modifyReplaceWORD;
  ModifyList[REF_ACTION_INSERT_WORD] = &RefEdit::modifyInsertWORD;
  ModifyList[REF_ACTION_DELETE_WORD] = &RefEdit::modifyDeleteWORD;
}


void RefEdit::setTagNumber(const unsigned noIn)
{
  tagno = noIn;
}


void RefEdit::setReverse()
{
  reverseFlag = true;
}


void RefEdit::setTag(const string& tagIn)
{
  tagVal = tagIn;
}


void RefEdit::setFieldNumber(const unsigned fieldnoIn)
{
  fieldno = fieldnoIn;
}


void RefEdit::setTagCount(const unsigned countIn)
{
  tagcount = countIn;
}


void RefEdit::setCharNumber(const unsigned countIn)
{
  charno = countIn;
}


void RefEdit::setWas(const string& wasIn)
{
  wasVal = wasIn;
}


void RefEdit::setIs(const string& isIn)
{
  isVal = isIn;
}


string RefEdit::tag() const
{
  return tagVal;
}


string RefEdit::was() const
{
  return wasVal;
}


string RefEdit::is() const
{
  return isVal;
}


unsigned RefEdit::repeatCount() const
{
  return tagcount;
}

unsigned RefEdit::fieldCount() const
{
  return fieldno;
}


void RefEdit::modifyFail(
  const string& line,
  const string& reason) const
{
   THROW("Modify fail: " + line + "\n" + reason + "\n\n");
}


void RefEdit::modify(
  string& line,
  const ActionType act) const
{
  (this->* ModifyList[act])(line);
}


void RefEdit::modify(
  vector<string>& lines,
  const ActionType act) const
{
  if (act != REF_ACTION_DELETE_GEN &&
      act != REF_ACTION_DELETE_PBN &&
      act != REF_ACTION_DELETE_RBN &&
      act != REF_ACTION_DELETE_RBX &&
      act != REF_ACTION_DELETE_EML)
    THROW("Multi-line must be delete: " + to_string(act) + ", " + lines[0]);
    
  if (act == REF_ACTION_DELETE_EML)
    modifyDeleteEML(lines);
  else
  {
    for (auto &line: lines)
      (this->* ModifyList[act])(line);
  }
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyGen functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefEdit::modifyReplaceGen(string& line) const
{
  line = isVal;
}


void RefEdit::modifyInsertGen(string& line) const
{
  line = isVal;
}


void RefEdit::modifyDeleteGen(string& line) const
{
  line = "";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyLIN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefEdit::modifyLINCommon(
  const string& line,
  const bool insertFlag,
  size_t& start,
  vector<string>& v,
  vector<string>& f,
  bool& endsOnPipe) const
{
  v.clear();
  tokenizeMinus(line, v, "|");
  const size_t vlen = v.size();

  if (tagno == 0)
    modifyFail(line, "No tag number");

  if (2 * tagno > vlen)
  {
    if (isVal == "" && 
       tagVal == "" && 
       2 * tagno == vlen+1)
    {
      // Last tag, no argument.
    }
    else
      RefEdit::modifyFail(line, "Tag number too large");
  }

  endsOnPipe = (line.length() > 0 && line.at(line.length()-1) == '|');

  if (reverseFlag)
    start = vlen - 2 * tagno;
  else
    start = 2 * (tagno-1);

  if (fieldno > 0)
  {
    f.clear();
    tokenize(v[start+1], f, ",");

    if (fieldno >= f.size()+1)
    {
      if (! insertFlag)
        modifyFail(line, "Field too far");
      else if (fieldno >= f.size()+2)
        modifyFail(line, "Insert field too far");
    }
  }
}


unsigned RefEdit::countUnitsLIN() const
{
  unsigned numPipes = static_cast<unsigned>
    (std::count(isVal.begin(), isVal.end(), '|'));
  if (numPipes % 2)
    THROW("Uneven isVal: " + isVal);

  return 1 + numPipes/2;
}


void RefEdit::modifyReplaceLIN(string& line) const
{
  size_t start;
  vector<string> v, f;
  bool endsOnPipe;
  RefEdit::modifyLINCommon(line, false, start, v, f, endsOnPipe);

  if (v[start] != tagVal)
    modifyFail(line, "Different LIN tags");

  if (fieldno > 0)
  {
    if (f[fieldno-1] != wasVal)
      modifyFail(line, "Old field wrong");

    f[fieldno-1] = isVal;
    v[start+1] = concat(f, ",");
  }
  else
  {
    if (v[start+1] != wasVal)
    {
      // Permit a not too short prefix.
      const size_t l = wasVal.length();
      if (l < 2 ||
          l >= v[start+1].length() ||
          v[start+1].substr(0, l) != wasVal)
        modifyFail(line, "Old value wrong: " + v[start+1]);
    }
    v[start+1] = isVal;
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


void RefEdit::modifyInsertLIN(string& line) const
{
  size_t start;
  vector<string> v, f;
  bool endsOnPipe;
  RefEdit::modifyLINCommon(line, true, start, v, f, endsOnPipe);

  if (fieldno > 0)
  {
    if (v[start] != tagVal)
      modifyFail(line, "Different LIN tags");

    f.insert(f.begin() + static_cast<int>(fieldno-1), isVal);
    v[start+1] = concat(f, ",");
  }
  else if (tagVal == "")
  {
    // Single insertion, i.e. could be a tag or a value.
    v.insert(v.begin() + static_cast<int>(start), isVal);
  }
  else
  {
    v.insert(v.begin() + static_cast<int>(start), isVal);
    v.insert(v.begin() + static_cast<int>(start), tagVal);
  }

  line = concat(v, "|");
  if (endsOnPipe && v.size() > 0)
    line += "|";
}


void RefEdit::modifyDeleteLIN(string& line) const
{
  size_t start;
  vector<string> v, f;
  bool endsOnPipe;
  RefEdit::modifyLINCommon(line, false, start, v, f, endsOnPipe);

  if (fieldno > 0)
  {
    if (v[start] != tagVal)
      modifyFail(line, "Different LIN tags");

    if (f[fieldno-1] != wasVal)
      modifyFail(line, "Old field wrong");

    auto sf = f.begin() + static_cast<int>(fieldno-1);
    const unsigned d = (tagcount == 0 ? 1 : tagcount);
    f.erase(sf, sf + static_cast<int>(d));

    v[start+1] = concat(f, ",");
  }
  else if (fieldno == 0 && tagVal == "")
  {
    // Delete a single entry without checking it.
    // Only use this when the entry is seriously messed up.
    if (wasVal != "" && v[start] != wasVal)
      modifyFail(line, "Old value wrong");
      
    v.erase(v.begin() + static_cast<int>(start));
  }
  else
  {
    if (v[start] != tagVal)
      modifyFail(line, "Different LIN tags");
    
    if (isVal == "" && tagVal == "")
    {
      // Delete a single entry without checking it.
      // Only use this when the entry is seriously messed up.
      v.erase(v.begin() + static_cast<int>(start));
    }
    else
    {
      if (v[start+1] != wasVal)
        modifyFail(line, "Old value wrong");
      
      auto s = v.begin() + static_cast<int>(start);
      const unsigned d = (tagcount == 0 ? 2 : 2*tagcount);
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

void RefEdit::modifyReplacePBN(string& line) const
{
  const regex rep("^\\[(\\w+)\\s+\"(.*)\"\\]\\s*$");
  smatch match;
  if (! regex_search(line, match, rep))
    THROW("Bad PBN line: " + line);

  if (tagVal != match.str(1))
    modifyFail(line, "Different PBN tags");

  if (wasVal != match.str(2))
    modifyFail(line, "Old value wrong");

  line = "[" + tagVal + " \"" + isVal + "\"" + "]";
}


void RefEdit::modifyInsertPBN(string& line) const
{
  line = "[" + tagVal + " \"" + isVal + "\"" + "]";
}


void RefEdit::modifyDeletePBN(string& line) const
{
  const regex rep("^\\[(\\w+)\\s+\"(.*)\"\\]\\s*$");
  smatch match;
  if (! regex_search(line, match, rep))
    THROW("Bad PBN line: " + line);

  if (tagVal != match.str(1))
    modifyFail(line, "Different PBN tags");

  if (wasVal != match.str(2))
    modifyFail(line, "Old value wrong");

  line = "";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyRBN functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefEdit::modifyRBNCommon(
  const string& line,
  string& s) const
{
  const size_t l = line.length();
  if (l == 0)
    THROW("RBN line too short: " + line);

  if (line.substr(0, 1) != tagVal)
    THROW("RBN tag wrong: " + line);

  if (l > 1 && line.at(1) != ' ')
    THROW("RBN syntax: " + line);

  if (l <= 2)
    s = "";
  else
    s = line.substr(2);
}


void RefEdit::modifyReplaceRBN(string& line) const
{
  string s;
  RefEdit::modifyRBNCommon(line, s);

  if (fieldno == 0)
  {
    // R,5+400:+14,6+420:14
    if (s != wasVal)
      THROW("RBN old value: " + line);
    line = tagVal + " " + isVal;
  }
  else
  {
    // P,2,SJ235,SJ234
    vector<string> f;
    f.clear();
    tokenize(s, f, ":");
    const size_t flen = f.size();
    if (fieldno > flen)
      THROW("RBN field number too large: " + line);
    if (f[fieldno-1] != wasVal)
      THROW("RBN old field value: " + line);
    f[fieldno-1] = isVal;
    line = tagVal + " " + concat(f, ":");
  }
}

void RefEdit::modifyInsertRBN(string& line) const
{
  string s;
  RefEdit::modifyRBNCommon(line, s);

  if (fieldno == 0)
    THROW("Not a field insertion: " + line);

  vector<string> f;
  f.clear();
  tokenize(s, f, ":");
  const size_t flen = f.size();
  if (fieldno > flen+1)
    THROW("RBN field number too large: " + line);
  else if (fieldno == flen+1)
    f.push_back(isVal);
  else
    f.insert(f.begin() + static_cast<int>(fieldno-1), isVal);

  line = tagVal + " " + concat(f, ":");
}


void RefEdit::modifyDeleteRBN(string& line) const
{
  string s;
  if (line != "" || wasVal != "")
    RefEdit::modifyRBNCommon(line, s);

  if (fieldno == 0)
  {
    // Could delete without checking.  Only use in dire cases.
    if (wasVal != "" && s != wasVal)
      THROW("RBN old value: " + line);
    line = ""; 
  }
  else
  {
    vector<string> f;
    f.clear();
    tokenize(s, f, ":");
    const size_t flen = f.size();
    if (fieldno > flen)
      THROW("RBN field number too large: " + line);
    if (f[fieldno-1] != wasVal)
      THROW("RBN old field value: " + line);

    if (tagcount > 0 && fieldno+tagcount > f.size()+1)
      THROW("RBN field deletion too large: " + line);

    auto st = f.begin() + static_cast<int>(fieldno-1);
    if (tagcount == 0)
      f.erase(st);
    else
      f.erase(st, st + static_cast<int>(tagcount));

    line = tagVal + " " + concat(f, ":");
  }
}



////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyRBX functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////


bool RefEdit::modifyCommonRBX(
  const string& line,
  vector<string>& v,
  string& s,
  size_t& pos) const
{
  v.clear();
  tokenize(line, v, "}");
  v.pop_back(); // Last empty field
  const size_t vlen = v.size();

  for (size_t i = 0; i < vlen; i++)
  {
    const size_t flen = v[i].length();
    if (flen <= 1)
      continue;

    if (v[i].substr(0, 1) == tagVal && v[i].at(1) == '{')
    {
      pos = i;
      if (flen == 2)
        s = "";
      else
        s = tagVal + " " + v[i].substr(2);
      return true;
    }
  }
  return false;
}


void RefEdit::modifyReplaceRBX(string& line) const
{
  vector<string> v;
  string s;
  size_t pos;

  if (! RefEdit::modifyCommonRBX(line, v, s, pos))
    THROW("RBN tag not found: " + line);

  RefEdit::modifyReplaceRBN(s);
  if (s.length() <= 2)
    v[pos] = tagVal + "{";
  else
    v[pos] = tagVal + "{" + s.substr(2);

  line = concat(v, "}") + "}";
}


void RefEdit::modifyInsertRBX(string& line) const
{
  vector<string> v;
  string s;
  size_t pos;

  if (RefEdit::modifyCommonRBX(line, v, s, pos))
  {
    // Insertion of a field in an existing tag.
    if (fieldno == 0)
      THROW("RBX tag already present: " + line);
    RefEdit::modifyInsertRBN(s);
    if (s.length() <= 2)
      v[pos] = tagVal + "{";
    else
      v[pos] = tagVal + "{" + s.substr(2);
  }
  else
  {
    // Insertion of a new tag.
    if (tagno > v.size()+1)
      THROW("RBX tag number too large: " + line);
    if (tagno == 0)
      THROW("RBX tag number too small: " + line);
      
    s = tagVal + "{" + isVal;
    if (tagno == v.size()+1)
      v.push_back(s);
    else
      v.insert(v.begin() + static_cast<int>(tagno-1), s);
  }
  line = concat(v, "}") + "}";
}


void RefEdit::modifyDeleteRBX(string& line) const
{
  vector<string> v;
  string s;
  size_t pos;

  if (! RefEdit::modifyCommonRBX(line, v, s, pos))
    THROW("RBN tag not found: " + line);

  RefEdit::modifyDeleteRBN(s);
  if (s == "")
    v.erase(v.begin() + static_cast<int>(pos));
  else
    v[pos] = tagVal + "{" + s.substr(2);

  line = concat(v, "}") + "}";
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyTXT functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

unsigned RefEdit::modifyCommonTXT(const string& line) const
{
  if (charno == 0)
  {
    const size_t p = line.find(wasVal);
    if (p == string::npos)
      return 0;
    else
      return static_cast<unsigned>(p+1);
  }
  else
    return charno; // As already 1-based
}


void RefEdit::modifyReplaceTXT(string& line) const
{
  const unsigned p = RefEdit::modifyCommonTXT(line);
  if (p == 0)
    THROW("No character position and string not found: " + line);

  size_t lw = wasVal.length();
  size_t li = isVal.length();

  if (line.substr(p-1, lw) != wasVal)
    THROW("Old TXT value: " + line.substr(p-1, lw) + " vs " + wasVal);

  if (lw > li)
    line.erase(p-1, lw-li);
  else if (lw < li)
    line.insert(p-1, " ", li-lw);

  line.replace(p-1, li, isVal);
}


void RefEdit::modifyInsertTXT(string& line) const
{
  if (charno == 0)
    THROW("No character position");

  if (charno > line.length())
  {
    if (charno > 128)
      THROW("Character position too large");

    line += string(charno-line.length(), ' ');
  }

  if (charno == line.length())
    line += isVal;
  else
    line.insert(charno, isVal);
}


void RefEdit::modifyDeleteTXT(string& line) const
{
  const unsigned p = RefEdit::modifyCommonTXT(line);
  if (p == 0)
    THROW("No character position and string not found");

  if (line.substr(p-1, wasVal.length()) != wasVal)
    THROW("Old TXT value");

  line.erase(p-1, wasVal.length());
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyWORD functions                                               //
//                                                                    //
////////////////////////////////////////////////////////////////////////

unsigned RefEdit::modifyCommonWORD(const string& line) const
{
  unsigned pos = 0;
  const size_t l = line.length();
  for (unsigned wno = 0; wno < tagno; wno++)
  {
    while (pos < l && line.at(pos) == ' ')
      pos++;

    if (wno == tagno-1)
      break;

    while (pos < l && line.at(pos) != ' ')
      pos++;
  }

  if (pos == l)
    return 0;
  else
    return pos+1;
}


void RefEdit::modifyReplaceWORD(string& line) const
{
  const unsigned pos = RefEdit::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "replaceWORD: Too few words");

  const size_t lw = wasVal.length();
  const size_t li = isVal.length();

  if (line.substr(pos-1, lw) != wasVal)
    modifyFail(line, "Old value wrong");

  if (lw > li)
    line.erase(pos-1, lw-li);
  else if (lw < li)
    line.insert(pos-1, " ", li-lw);

  line.replace(pos-1, li, isVal);
}


void RefEdit::modifyInsertWORD(string& line) const
{
  const unsigned pos = RefEdit::modifyCommonWORD(line);
  if (pos == 0)
  {
    // Not checking whether the word number is way too large.
    if (line.length() > 0 && line.at(line.length()-1) == ' ')
      line += isVal;
    else
      line += " " + isVal;
  }
  else if (line.at(pos-1) == ' ')
    line.insert(pos-1, isVal);
  else
    line.insert(pos-1, isVal + " ");
}

void RefEdit::modifyDeleteWORD(string& line) const
{
  const size_t pos = RefEdit::modifyCommonWORD(line);
  if (pos == 0)
    modifyFail(line, "deleteWORD: Too few words");

  const size_t lw = wasVal.length();
  if (line.substr(pos-1, lw) != wasVal)
    modifyFail(line, "Old value wrong");

  line.erase(pos-1, lw);
}


////////////////////////////////////////////////////////////////////////
//                                                                    //
// modifyEML functions                                                //
//                                                                    //
////////////////////////////////////////////////////////////////////////

void RefEdit::modifyDeleteEML(vector<string>& lines) const
{
  size_t lc = lines.size();
  size_t resLine = 0;
  while (resLine < lc &&
      (lines[resLine].length() < 49 || 
       lines[resLine].substr(42, 6) != "Result"))
  {
    resLine++;
  }

  if (resLine == lc)
    THROW("No Result line");

  if (resLine < 2)
    THROW("Result line too high");
  
  size_t nextLine = resLine+1;
  while (nextLine < lc && lines[nextLine].length() > 48)
    nextLine++;

  if (nextLine == lc)
    THROW("No empty line before end");

  if (nextLine-resLine > 3)
    THROW("Too many intermediate lines");
  
  size_t start = 2;
  for (size_t i = 0; i < lc; i++)
  {
    if (i >= resLine && i <= nextLine)
    {
      if (lines[start].length() < 42)
        lines[start] += string(42-lines[start].length(), ' ');
      
      if (i == nextLine)
        lines[start] += "Contract: " + isVal;
      else
        lines[start] += lines[i].substr(42);
      start++;
    }

    if (lines[i].length() > 42)
      lines[i].erase(42);

    lines[i] = trimTrailing(lines[i]);
  }
}


string RefEdit::str() const
{
  stringstream ss;

  if (reverseFlag)
    ss << left << setw(14) << "Tag number" << tagno << " (from the back)\n";
  else
    ss << left << setw(14) << "Tag number" << tagno << "\n";

  ss << setw(14) << "Tag" << tagVal << "\n";
  if (fieldno > 0)
  {
    if (tagcount == 0)
      ss << setw(14) << "Field" << fieldno << "\n";
    else
      ss << setw(14) << "Fields" << fieldno << " to " <<
        fieldno + tagcount - 1 << "\n";
  }

  if (charno > 0)
    ss << setw(14) << "Char. number" << charno << "\n";
  
  ss << setw(14) << "String was" << "'" << wasVal << "'" << "\n";
  ss << setw(14) << "String is" << "'" << isVal << "'" << "\n";

  return ss.str();
}

