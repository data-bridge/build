/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "Buffer.h"
#include "funcRefStats.h"
#include "Bexcept.h"


void dispatchRefStats(
  const string& fname,
  const Format format,
  RefLines& refLines,
  RefStats& refstats,
  ostream& flog)
{
  try
  {
    // First check the header.
    if (refLines.skip())
    {
      // Ugh.  Poor man's counter of hands and boards.
      Buffer buffer;
      buffer.readForce(fname, format);

      unsigned numLines, numHands, numBoards;
      numLines = buffer.lengthOrig();
      vector<string> lines;
      for (unsigned i = 1; i <= numLines; i++)
        lines.push_back(buffer.getLine(i));

      RefLine rl;
      rl.countHands(lines, FORMAT_INPUT_MAP[format], numHands, numBoards);
      refLines.setFileData(numLines, numHands, numBoards);
    }

    refLines.checkHeader();

    // Then check the other lines.
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

