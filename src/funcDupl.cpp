/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "Group.h"
#include "RefLines.h"
#include "DuplStats.h"

#include "Bexcept.h"


using namespace std;


void writeDupl(
  const Group& group,
  const RefLines& reflines,
  DuplStats& duplstats)
{
  unsigned segNo = 0;
  for (auto &segment: group)
  {
    if (segment.size() == 0)
      continue;

    duplstats.set(&group, &segment, segNo, &reflines);
    segNo++;

    for (auto &bp: segment)
      duplstats.append(bp.board.hash12());
    
    duplstats.sortActive();
  }
}


void dispatchDupl(
  const Group& group,
  const RefLines& reflines,
  DuplStats& duplstats,
  ostream& flog)
{
  try
  {
    writeDupl(group, reflines, duplstats);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

