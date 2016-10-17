/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>

#include "validate.h"
#include "valint.h"
#include "validateEML.h"
#include "parse.h"
#include "portab.h"


static bool isEMLPlayLine(
   const string& lineOut,
   const string& lineRef)
{
  const unsigned lOut = lineOut.length();
  const unsigned lRef = lineRef.length();

  if (lOut < 42 || lOut >= lRef)
    return false;

  // A bit thin -- could look more carefully.
  
  if (lineOut == lineRef.substr(0, lOut))
    return true;

  if (lRef > 78)
  {
    // The reference play may be shifted in by 3.
    string outShort = lineOut;
    outShort.erase(39, 3);
    if (outShort == lineRef.substr(0, lOut-3))
      return true;
  }

  return false;
}


bool validateEML(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats)
{
  UNUSED(frstr);
  if (isEMLPlayLine(running.out.line, running.ref.line))
  {
    stats.counts[BRIDGE_VAL_PLAY_SHORT]++;
    return true;
  }

  return false;
}

