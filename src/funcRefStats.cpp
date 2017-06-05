/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "funcRefStats.h"
#include "Bexcept.h"


void dispatchRefStats(
  const RefLines& refLines,
  RefStats& refstats,
  ostream& flog)
{
  try
  {
    CommentType cat;
    RefEntry re;

    if (refLines.getHeaderEntry(cat, re))
      refstats.logFile(re);
    else
      THROW("No header in refLines");

    if (! refLines.hasComments())
      return;

    refstats.logRefFile();

    if (refLines.skip())
    {
      refstats.logSkip(cat, re);
      return;
    }
    else if (refLines.orderCOCO())
      refstats.logOrder(ERR_LIN_QX_ORDER_COCO, re);
    else if (refLines.orderOOCC())
      refstats.logOrder(ERR_LIN_QX_ORDER_OOCC, re);
    else if (! refLines.validate())
      refstats.logNoval(cat, re);

    for (auto &rl: refLines)
    {
      rl.getEntry(cat, re);
      refstats.logRef(cat, re);
    }
    
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

