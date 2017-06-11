/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "Group.h"
#include "Sheet.h"

#include "funcIMPSheet.h"
#include "Bexcept.h"


void dispatchIMPSheet(
  const Group& group,
  ostream& flog)
{
  for (auto const &segment: group)
  {
    if (segment.size() == 0)
      continue;

    flog << segment.strTitle(BRIDGE_FORMAT_TXT) << "\n";
    flog << segment.strIMPSheetHeader();

    unsigned score1, score2;
    segment.getCarry(score1, score2);

    for (auto &bpair: segment)
    {
      flog << bpair.board.strIMPSheetLine(
        segment.strNumber(bpair.extNo, BRIDGE_FORMAT_TXT), score1, score2);
    }

    flog << segment.strIMPSheetFooter(score1, score2);
  }
}

