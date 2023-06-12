/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "../Group.h"
#include "../Buffer.h"

#include "../stats/CompStats.h"

#include "funcCompare.h"
#include "funcRead.h"

#include "../Bexcept.h"
#include "../Bdiff.h"


void dispatchCompare(
  const string& fname,
  const Format format,
  const Options& options,
  const string& text,
  const Group& group,
  CompStats& cstats,
  ostream& flog)
{
  try
  {
    Group groupNew;
    if (group.isCOCO() &&
       (format == BRIDGE_FORMAT_TXT ||
        format == BRIDGE_FORMAT_EML ||
        format == BRIDGE_FORMAT_REC))
      groupNew.setCOCO(format);

    Buffer buffer;
    buffer.split(text, format);

    BoardOrder orderSeen;
    dispatchReadBuffer(format, options, buffer, groupNew, orderSeen, flog);

    if (orderSeen == ORDER_COCO &&
        format != BRIDGE_FORMAT_TXT &&
        format != BRIDGE_FORMAT_EML &&
        format != BRIDGE_FORMAT_REC)
    {
      // This also swaps teams in some cases.
      groupNew.setCOCO(format);
    }

    group == groupNew;
    cstats.add(true, format);
  }
  catch (Bdiff& bdiff)
  {
    cout << "Difference: " << fname << ", format " <<
      FORMAT_NAMES[format] << "\n";

    bdiff.print(flog);
    cstats.add(false, format);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    cstats.add(false, format);
  }
}

