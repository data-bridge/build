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
  control = ERR_REF_STANDARD;
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

  string line, s;
  smatch match;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    RefLine refLine;
    if (refLine.parse(fname, line))
    {
      lines.push_back(refLine);
      continue;
    }

    // TODO: Check the skip reason and numbers.

    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": No special word in '" + line + "'");

    if (s == "skip")
      control = ERR_REF_SKIP;
    else if (s == "noval")
      control = ERR_REF_NOVAL;
    else if (s == "orderCOCO")
      control = ERR_REF_OUT_COCO;
    else if (s == "orderOOCC")
      control = ERR_REF_OUT_OOCC;
    else
      THROW("Ref file " + refName + ": Bad number in '" + line + "'");
    continue;
  }
  refstr.close();
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


