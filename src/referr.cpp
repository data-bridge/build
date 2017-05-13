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
#include "parse.h"
#include "Bexcept.h"

using namespace std;


static bool lineToLINList(
  const string& line,
  vector<string>& list);


void readRefFile(
  const string& fname,
  vector<Refline>& reflines,
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
  smatch match;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    Refline refline;
    if (refline.parse(fname, line))
    {
      reflines.push_back(refline);
      continue;
    }

    // TODO: Check the skip reason and numbers.

    if (! getNextWord(line, s))
      THROW("Ref file " + refName + ": No special word in '" + line + "'");

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
  refstr.close();
}

