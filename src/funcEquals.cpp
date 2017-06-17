/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "Group.h"

#include "Bexcept.h"


using namespace std;


void writeEquals(
  const Group& group,
  ostream& flog)
{
  string st;
  st = group.name() + ":";
  for (auto &segment: group)
  {
    if (segment.size() == 0)
      continue;

    for (auto &bp: segment)
      st += " " + STR(bp.board.hash12());
  }
  st += "\n";
  flog << st;
}


void dispatchEquals(
  const Group& group,
  ostream& flog)
{
  try
  {
    writeEquals(group, flog);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

