/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "../Buffer.h"
#include "../RefLines.h"

#include "../stats/RefStats.h"

#include "funcRefStats.h"

#include "../Bexcept.h"


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

    if (cat == static_cast<CommentType>(0))
    {
      // We might have inferred a non-standard board order,
      // but it was not stated.
      // re.noRefLines = 1;
      re.setLines(1);
      if (refLines.orderCOCO())
        refstats.logOrder(ORDER_COCO, format, re);
      else if (refLines.orderOOCC())
        refstats.logOrder(ORDER_OOCC, format, re);
    }
    else
    {
      // A non-standard board order might have been stated.
      if (refLines.orderCOCO() || refLines.orderOOCC())
        refstats.logOrder(cat, re);
    }

    if (! refLines.hasComments())
      return;

    refstats.logRefFile();

    if (refLines.skip())
    {
      refstats.logSkip(cat, re);
      return;
    }
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

