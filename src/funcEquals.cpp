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
  flog << group.name() << ":";
  for (auto &segment: group)
  {
    if (segment.size() == 0)
      continue;

    for (auto &bp: segment)
      flog << " " << bp.board.hash8();
  }
  flog << "\n";
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

