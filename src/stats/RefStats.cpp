/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#pragma warning(pop)

#include "RefStats.h"
#include "../RefEntry.h"

#include "../RefComment.h"
#include "../Bexcept.h"

using namespace std;


static const vector<string> RefTableNames =
{
  "Source files", "Ref files", "Skip files", "Noval files", "Order files"
};


RefStats::RefStats()
{
  data.resize(REFSTATS_SIZE);
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    data[table].resize(ERR_SIZE);

  catSeen.resize(ERR_SIZE);
  numFiles.resize(REFSTATS_SIZE);

  RefStats::reset();
}


RefStats::~RefStats()
{
}


void RefStats::resetSeen()
{
  for (unsigned cat = 0; cat < ERR_SIZE; cat++)
    catSeen[cat] = false;
}


void RefStats::resetRefEntry(RefEntry& re) const
{
  re.reset();
}


void RefStats::reset()
{
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    for (unsigned cat = 0; cat < ERR_SIZE; cat++)
      RefStats::resetRefEntry(data[table][cat]);

  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    numFiles[table] = 0;

  RefStats::resetSeen();
}


void RefStats::incrRefEntry(
  RefEntry& re,
  const RefEntry& re2) const
{
  re += re2;
}


void RefStats::incr(
  const RefTables table,
  const CommentType cat,
  const RefEntry& re)
{
  data[table][cat] += re;
}


void RefStats::logFile(const RefEntry& re)
{
  RefStats::incr(REFSTATS_SOURCE, static_cast<CommentType>(0), re);
  numFiles[REFSTATS_SOURCE]++;
}


void RefStats::logRefFile()
{
  resetSeen();
  numFiles[REFSTATS_REF]++;
}


void RefStats::logSkip(
  const CommentType cat,
  const RefEntry& re)
{
  RefStats::incr(REFSTATS_SKIP, cat, re);
  numFiles[REFSTATS_SKIP]++;
}


void RefStats::logNoval(
  const CommentType cat,
  const RefEntry& re)
{
  RefStats::incr(REFSTATS_NOVAL, cat, re);
  numFiles[REFSTATS_NOVAL]++;
}


void RefStats::logOrder(
  const CommentType cat,
  const RefEntry& re)
{
  RefStats::incr(REFSTATS_ORDER, cat, re);
  numFiles[REFSTATS_ORDER]++;
}


void RefStats::logOrder(
  const BoardOrder order,
  const Format format,
  const RefEntry& re)
{
  CommentType cat;
  if (order == ORDER_COCO)
  {
    if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
      cat = ERR_LIN_ORDER_COCO_INFER;
    else if (format == BRIDGE_FORMAT_PBN)
      cat = ERR_PBN_ORDER_COCO_INFER;
    else if (format == BRIDGE_FORMAT_RBN)
      cat = ERR_RBN_ORDER_COCO_INFER;
    else if (format == BRIDGE_FORMAT_RBX)
      cat = ERR_RBX_ORDER_COCO_INFER;
    else  
      THROW("Bad format");
  }
  else if (order == ORDER_OOCC)
  {
    if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
      cat = ERR_LIN_ORDER_OOCC_INFER;
    else if (format == BRIDGE_FORMAT_PBN)
      cat = ERR_PBN_ORDER_OOCC_INFER;
    else if (format == BRIDGE_FORMAT_RBN)
      cat = ERR_RBN_ORDER_OOCC_INFER;
    else if (format == BRIDGE_FORMAT_RBX)
      cat = ERR_RBX_ORDER_OOCC_INFER;
    else  
      THROW("Bad format");
  }
  else
    THROW("Bad order");
  
  RefStats::logOrder(cat, re);
}


void RefStats::logRef(
  const CommentType cat,
  RefEntry& re)
{
  re.setFiles(catSeen[cat] ? 0u : 1u);
  catSeen[cat] = true;

  RefStats::incr(REFSTATS_REF, cat, re);
}


void RefStats::operator +=(const RefStats& rf2)
{
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    for (unsigned cat = 0; cat < ERR_SIZE; cat++)
      RefStats::incr(static_cast<RefTables>(table), 
        static_cast<CommentType>(cat), rf2.data[table][cat]);

  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    numFiles[table] += rf2.numFiles[table];
}


void RefStats::print(ostream& fstr) const
{
  if (numFiles[REFSTATS_SOURCE] == 0)
    return;

  RefComment comment; // Kludge to get at names
  RefEntry refSum;

  string dashes;
  dashes.resize(0);
  dashes.insert(0, 72, '-');

  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
  {
    if (numFiles[table] == 0)
      continue;

    fstr << RefTableNames[table] << "\n\n";

    fstr << setw(28) << left << "Reference" <<
      refSum.strLineHeader();

    RefStats::resetRefEntry(refSum);

    for (unsigned comm = 0; comm < ERR_SIZE; comm++)
    {
      if (data[table][comm].getFiles() == 0)
        continue;

      RefStats::incrRefEntry(refSum, data[table][comm]);

      if (table == 0)
        continue; // Only show sum

      fstr << setw(28) << left << 
        comment.comment2str(static_cast<CommentType>(comm)) <<
        data[table][comm].strLine();
    }

    fstr << dashes << "\n";

    fstr << setw(28) << left << "Sum" << refSum.strLine() << "\n";
  }
}

