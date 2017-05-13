/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <regex>

#include "Reflines.h"
#include "parse.h"
#include "Bexcept.h"


Reflines::Reflines()
{
  Reflines::reset();
}


Reflines::~Reflines()
{
}


void Reflines::reset()
{
  lines.clear();
  control = ERR_REF_STANDARD;
}


void Reflines::read(const string& fname)
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

    Refline refline;
    if (refline.parse(fname, line))
    {
      lines.push_back(refline);
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


bool Reflines::skip() const
{
  return (control == ERR_REF_SKIP);
}

bool Reflines::validate() const
{
  return (control != ERR_REF_NOVAL);
}

bool Reflines::orderCOCO() const
{
  return (control == ERR_REF_OUT_COCO);
}

bool Reflines::orderOOCC() const
{
  return (control == ERR_REF_OUT_OOCC);
}


