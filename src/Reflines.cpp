/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <regex>

#include "RefLines.h"
#include "parse.h"
#include "Bexcept.h"


RefLines::RefLines()
{
  RefLines::reset();
}


RefLines::~RefLines()
{
}


void RefLines::reset()
{
  lines.clear();
  bufferLines = 0;
  numHands = 0;
  numBoards = 0;
  control = ERR_REF_STANDARD;
  headerComment.reset();
}


void RefLines::setFileData(
  const unsigned bufIn,
  const unsigned numHandsIn,
  const unsigned numBoardsIn)
{
  bufferLines = bufIn;
  numHands = numHandsIn;
  numBoards = numBoardsIn;
}


bool RefLines::parseComment(
  const string& fname,
  string& line)
{
  string s;
  if (! getNextWord(line, s))
    return false;

  if (s == "skip")
    control = ERR_REF_SKIP;
  else if (s == "noval")
    control = ERR_REF_NOVAL;
  else if (s == "orderCOCO")
    control = ERR_REF_OUT_COCO;
  else if (s == "orderOOCC")
    control = ERR_REF_OUT_OOCC;
  else
    return false;

  unsigned end;
  headerComment.parse(fname, line, 0, end);
  return true;
}


void RefLines::read(const string& fname)
{
  regex re("\\.\\w+$");
  string refName = regex_replace(fname, re, string(".ref"));
  control = ERR_REF_STANDARD;

  // There might not be a .ref file (not an error).
  ifstream refstr(refName.c_str());
  if (! refstr.is_open())
    return;

  string line;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    RefLine refLine;
    if (refLine.parse(fname, line))
      lines.push_back(refLine);
    else if (! RefLines::parseComment(fname, line))
      THROW("Ref file " + refName + ": Bad line in '" + line + "'");
  }
  refstr.close();
}


bool RefLines::hasComments() const
{
  return (lines.size() > 0 || headerComment.isCommented());
}


bool RefLines::skip() const
{
  return (control == ERR_REF_SKIP);
}


bool RefLines::validate() const
{
  return (control != ERR_REF_NOVAL);
}


bool RefLines::orderCOCO() const
{
  return (control == ERR_REF_OUT_COCO);
}


bool RefLines::orderOOCC() const
{
  return (control == ERR_REF_OUT_OOCC);
}


bool RefLines::getHeaderEntry(
  CommentType& cat,
  RefEntry& re) const
{
  if (bufferLines == 0)
    return false;

  if (headerComment.isCommented())
  {
    headerComment.getEntry(cat, re);

    // Fix the order.
    re.count.lines = re.count.units;
    re.count.units = 0;
    re.noRefLines = 1;

    if (cat == ERR_SIZE)
      cat = static_cast<CommentType>(0);
  }
  else
  {
    // Synthesize a mini-header.
    cat = static_cast<CommentType>(0);
    re.files = 1;
    re.noRefLines = lines.size();
    re.count.lines = bufferLines;
    re.count.hands = numHands;
    re.count.boards = numBoards;
  }

  return true;
}


bool RefLines::getControlEntry(
  CommentType& cat,
  RefEntry& re) const
{
  if (control == ERR_REF_STANDARD)
    return false;

  headerComment.getEntry(cat, re);
  re.count.lines = 1;
  return true;
}

