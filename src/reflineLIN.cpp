/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <regex>

#include "reflineLIN.h"
#include "parse.h"
#include "Bexcept.h"

using namespace std;


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
    if (l == 2)
      return "";
    else
      return entry.substr(1, l-2);
  }
  else
    return entry;
}


static bool parseRefLIN(
  const string& line,
  RefFix& rf)
{
  // Only does insertLIN, replaceLIN and deleteLIN.

  regex re("^\\s*\"(.*)\"");
  smatch match;
  if (! regex_search(line, match, re) || match.size() < 1)
    return false;
  const string arg = match.str(1);

  const unsigned commas = static_cast<unsigned>(
    count(arg.begin(), arg.end(), ','));
  if (commas > 4)
    return false;

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


  if (rf.type == BRIDGE_REF_INSERT_LIN)
  {
    if (commas < 1)
      return false;
  
    // First one must be a tag number.
    if (! str2upos(v[0], rf.fixLIN.tagNo))
      return false;

    // Optional field number (say in an rs value).
    unsigned n = 1;
    if (str2upos(v[1], rf.fixLIN.fieldNo))
      n = 2;
    else
      rf.fixLIN.fieldNo = 0;

    // LIN tag.
    if (n == commas)
    {
      // Short version with no argument.
      rf.fixLIN.tag = "";
      rf.fixLIN.was = "";
      rf.fixLIN.is = v[n];
      rf.fixLIN.extent = 1;
    }
    else if (n == commas-1)
    {
      // Normal version.
      if (v[n].length() != 2)
        return false;
      rf.fixLIN.tag = v[n];
      rf.fixLIN.was = "";
      rf.fixLIN.is = parseRefLINEntry(v[n+1]);
      rf.fixLIN.extent = 1;
    }
    else
      return false;
  
  }
  else if (rf.type == BRIDGE_REF_REPLACE_LIN)
  {
    if (commas <= 1)
      return false;
  
    // First one must be a tag number.
    if (! str2upos(v[0], rf.fixLIN.tagNo))
      return false;

    // Optional field number (say in an rs value).
    unsigned n = 1;
    if (str2upos(v[1], rf.fixLIN.fieldNo))
      n = 2;
    else
      rf.fixLIN.fieldNo = 0;

    // LIN tag except perhaps in deletion.
    if (v[n].length() != 2 || n+2 != commas)
      return false;
    rf.fixLIN.tag = v[n];
    rf.fixLIN.was = parseRefLINEntry(v[n+1]);
    rf.fixLIN.is = parseRefLINEntry(v[n+2]);
    rf.fixLIN.extent = 1;
  }
  else if (rf.type == BRIDGE_REF_DELETE_LIN)
  {
    if (commas <= 1)
    {
      // First one must be a tag number.
      if (! str2upos(v[0], rf.fixLIN.tagNo))
        return false;
      rf.fixLIN.fieldNo = 0;
      if (commas == 1)
        rf.fixLIN.tag = parseRefLINEntry(v[1]);
      else
        rf.fixLIN.tag = "";
      rf.fixLIN.was = "";
      rf.fixLIN.is = "";
      rf.fixLIN.extent = 1;
    }
    else
    {
      // First one must be a tag number.
      if (! str2upos(v[0], rf.fixLIN.tagNo))
        return false;

      // Optional field number (say in an rs value).
      unsigned n = 1;
      if (str2upos(v[1], rf.fixLIN.fieldNo))
        n = 2;
      else
        rf.fixLIN.fieldNo = 0;

      // LIN tag except perhaps in deletion.
      if (v[n].length() != 2)
        return false;

      // Kludge in "is".
      rf.fixLIN.tag = v[n];
      rf.fixLIN.was = parseRefLINEntry(v[n+1]);
      rf.fixLIN.is = (rf.fixLIN.was == "" ? "non-empty" : "");
      rf.fixLIN.extent = 1;

      if (n+2 == commas)
      {
        // deleteLIN "1,7,rs,3NW=,4"
        // deleteLIN "3,mb,p,2"
        if (! str2upos(v[n+2], rf.fixLIN.extent))
          return false;
      }
      else if (n+1 != commas)
        return false;
    }
  }
  else
    return false;

  return true;
}


void parseReplaceLIN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(rer);

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


void parseInsertLIN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(rer);

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


void parseDeleteLIN(
  const string& refName,
  const string& line,
  const regex& rer,
  const regex& rep,
  smatch& match,
  RefFix& rf)
{
  UNUSED(rer);

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


void modifyReplaceLIN(
 const string& line,
 const unsigned start,
 const bool interiorFlag,
 const RefFix& rf,
 vector<string>& vLIN,
 vector<string>& fields)
{
  if (vLIN[start] != rf.fixLIN.tag)
    modifyLINFail(line, rf.fixLIN, "Different tags: " + vLIN[start]);

  if (interiorFlag)
  {
    if (fields[rf.fixLIN.fieldNo-1] != rf.fixLIN.was)
      modifyLINFail(line, rf.fixLIN, "Old field wrong");

    fields[rf.fixLIN.fieldNo-1] = rf.fixLIN.is;
    vLIN[start+1] = concat(fields, ",");
  }
  else
  {
    if (vLIN[start+1] != rf.fixLIN.was)
    {
      // Permit a not too short prefix.
      const unsigned l = rf.fixLIN.was.length();
      if (l < 2 || 
          l >= vLIN[start+1].length() ||
          vLIN[start+1].substr(0, l) != rf.fixLIN.was)
      modifyLINFail(line, rf.fixLIN, 
        "Old value wrong: " + vLIN[start+1]);
    }

    vLIN[start+1] = rf.fixLIN.is;
  }
}


void modifyInsertLIN(
 const string& line,
 const unsigned start,
 const bool interiorFlag,
 const RefFix& rf,
 vector<string>& vLIN,
 vector<string>& fields)
{
  if (interiorFlag)
  {
    if (vLIN[start] != rf.fixLIN.tag)
      modifyLINFail(line, rf.fixLIN, "Different tags");

    fields.insert(fields.begin()+static_cast<int>(rf.fixLIN.fieldNo-1), 
      rf.fixLIN.is);
    vLIN[start+1] = concat(fields, ",");
  }
  else if (rf.fixLIN.tag == "")
  {
    // Single insertion, i.e. could be a tag or a value.
    vLIN.insert(vLIN.begin()+static_cast<int>(start), rf.fixLIN.is);
  }
  else
  {
    vLIN.insert(vLIN.begin()+static_cast<int>(start), rf.fixLIN.is);
    vLIN.insert(vLIN.begin()+static_cast<int>(start), rf.fixLIN.tag);
  }
}


void modifyDeleteLIN(
 const string& line,
 const unsigned start,
 const bool interiorFlag,
 const RefFix& rf,
 vector<string>& vLIN,
 vector<string>& fields)
{
  if (interiorFlag)
  {
    if (vLIN[start] != rf.fixLIN.tag)
      modifyLINFail(line, rf.fixLIN, "Different tags: " + vLIN[start]);

    if (fields[rf.fixLIN.fieldNo-1] != rf.fixLIN.was)
      modifyLINFail(line, rf.fixLIN, "Old field wrong");

    auto sf = fields.begin() + static_cast<int>(rf.fixLIN.fieldNo-1);
    fields.erase(sf, sf + static_cast<int>(rf.fixLIN.extent));

    vLIN[start+1] = concat(fields, ",");
  }
  else if (rf.fixLIN.fieldNo == 0 && rf.fixLIN.tag == "")
  {
    // Delete a single entry without checking it.
    // Only use this when the entry is seriously messed up.
    auto s = vLIN.begin() + static_cast<int>(start);
    vLIN.erase(s);
  }
  else
  {
    if (vLIN[start] != rf.fixLIN.tag)
      modifyLINFail(line, rf.fixLIN, "Different tags: " + vLIN[start]);

    if (rf.fixLIN.is == "" && rf.fixLIN.was == "")
    {
      // Delete a single entry.
      // Only use this when the entry is seriously messed up.
      auto s = vLIN.begin() + static_cast<int>(start);
      vLIN.erase(s);
    }
    else
    {
      if (vLIN[start+1] != rf.fixLIN.was)
        modifyLINFail(line, rf.fixLIN, "Old value wrong");

      auto s = vLIN.begin() + static_cast<int>(start);
      vLIN.erase(s, s + static_cast<int>(2*rf.fixLIN.extent));
    }
  }
}

