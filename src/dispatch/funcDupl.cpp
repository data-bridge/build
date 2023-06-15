/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iomanip>
#include <sstream>
#include <fstream>
#pragma warning(pop)

#include "../records/Group.h"

#include "../edits/RefLines.h"

#include "../stats/DuplStats.h"

#include "../handling/Bexcept.h"


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

    duplstats.set(group, segment, segNo, reflines);
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

