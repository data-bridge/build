/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include "funcRefStats.h"

#include "../files/Buffer.h"

#include "../edits/RefLines.h"

#include "../stats/RefStats.h"

#include "../handling/Bexcept.h"


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
    {
      refstats.logFile(REFSTATS_SOURCE);
      refstats.log(REFSTATS_SOURCE, static_cast<CommentType>(0), re);
    }
    else
      THROW("No header in refLines");

    if (cat == static_cast<CommentType>(0))
    {
      // We might have inferred a non-standard board order,
      // but it was not stated.
      re.setLines(1);
      if (refLines.orderCOCO())
      {
        refstats.logFile(REFSTATS_ORDER);
        if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
          refstats.log(REFSTATS_ORDER, ERR_LIN_ORDER_COCO_INFER, re);
        else if (format == BRIDGE_FORMAT_PBN)
          refstats.log(REFSTATS_ORDER, ERR_PBN_ORDER_COCO_INFER, re);
        else if (format == BRIDGE_FORMAT_RBN)
          refstats.log(REFSTATS_ORDER, ERR_RBN_ORDER_COCO_INFER, re);
        else if (format == BRIDGE_FORMAT_RBX)
          refstats.log(REFSTATS_ORDER, ERR_RBX_ORDER_COCO_INFER, re);
        else
          THROW("Bad format");
      }
      else if (refLines.orderOOCC())
      {
        refstats.logFile(REFSTATS_ORDER);
        if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
          refstats.log(REFSTATS_ORDER, ERR_LIN_ORDER_OOCC_INFER, re);
        else if (format == BRIDGE_FORMAT_PBN)
          refstats.log(REFSTATS_ORDER, ERR_PBN_ORDER_OOCC_INFER, re);
        else if (format == BRIDGE_FORMAT_RBN)
          refstats.log(REFSTATS_ORDER, ERR_RBN_ORDER_OOCC_INFER, re);
        else if (format == BRIDGE_FORMAT_RBX)
          refstats.log(REFSTATS_ORDER, ERR_RBX_ORDER_OOCC_INFER, re);
        else
          THROW("Bad format");
      }
    }
    else
    {
      // A non-standard board order might have been stated.
      if (refLines.orderCOCO() || refLines.orderOOCC())
      {
        refstats.logFile(REFSTATS_ORDER);
        refstats.log(REFSTATS_ORDER, cat, re);
      }
    }

    if (! refLines.hasComments())
      return;

    refstats.logFile(REFSTATS_REF);

    if (refLines.skip())
    {
      refstats.logFile(REFSTATS_SKIP);
      refstats.log(REFSTATS_SKIP, cat, re);
      return;
    }
    else if (! refLines.validate())
    {
      refstats.logFile(REFSTATS_NOVAL);
      refstats.log(REFSTATS_NOVAL, cat, re);
    }

    for (auto &rl: refLines)
    {
      rl.getEntry(cat, re);
      refstats.log(REFSTATS_REF, cat, re);
    }
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

