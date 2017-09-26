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

#include "Group.h"

#include "Bexcept.h"


using namespace std;


void writeValuation(
  Group& group,
  ostream& flog)
{
  UNUSED(flog);

  for (auto segment = group.mbegin(); segment != group.mend(); segment++)
  {
    if (segment->size() == 0)
      continue;

    for (auto bp = segment->mbegin(); bp != segment->mend(); bp++)
    {
      // TODO: Delete commment.
      // flog << bp->board.strDeal(BRIDGE_FORMAT_TXT);

      bp->board.performValuation(true);
      // flog << bp->board.strValuation();
    }
  }
}


void dispatchValuation(
  Group& group,
  ostream& flog)
{
  try
  {
    writeValuation(group, flog);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

