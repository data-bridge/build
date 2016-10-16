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
#include "validateRBN.h"
#include "parse.h"


bool validateRBN(
  ifstream& frstr,
  ValExample& running,
  ValFileStats& stats)
{
  char rf, of;

  unsigned lOut = running.out.line.length();
  unsigned lRef = running.ref.line.length();

  if (lOut == 0 || lRef == 0)
    return false;

  of = running.out.line.at(0);
  rf = running.ref.line.at(0);

  if (of == rf)
  {

    switch (of)
    {
      case 'T':
        if (lOut > 2)
          return false;
        valError(stats, running, BRIDGE_VAL_TITLE);
        return true;

      case 'D':
        if (lOut > 2)
          return false;
        valError(stats, running, BRIDGE_VAL_DATE);
        return true;
        
      case 'L':
        if (lOut > 2)
          return false;
        valError(stats, running, BRIDGE_VAL_LOCATION);
        return true;
        
      case 'E':
        if (lOut > 2)
          return false;
        valError(stats, running, BRIDGE_VAL_EVENT);
        return true;
        
      case 'S':
        if (lOut > 2)
          return false;
        valError(stats, running, BRIDGE_VAL_SESSION);
        return true;

      case 'P':
        if (lOut >= lRef || 
            running.ref.line.substr(0, lOut) != running.out.line)
          return false;
        valError(stats, running, BRIDGE_VAL_PLAY_SHORT);
        return true;

      default:
        return false;
    }
  }

  if (rf == 'K')
  {
    valError(stats, running, BRIDGE_VAL_TEAMS);

    if (! valProgress(frstr, running.ref))
    {
      stats.counts[BRIDGE_VAL_REF_SHORT]++;
      return false;
    }
  }
 
  return (running.out.line == running.ref.line);
}

