/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "RefStats.h"
#include "RefComment.h"
#include "Bexcept.h"

using namespace std;


static const vector<string> RefTableNames =
{
  "Source files", "Ref files", "Skip files", "Noval files"
};


RefStats::RefStats()
{
  RefStats::reset();
}


RefStats::~RefStats()
{
}


void RefStats::resetSeen()
{
  numSourceFiles = 0;
  numRefFiles = 0;
  for (unsigned tag = 0; tag < ERR_SIZE; tag++)
    tagSeen[tag] = false;
}


void RefStats::resetRefEntry(RefEntry& re) const
{
  re.files = 0;
  re.noRefLines = 0;
  re.count.lines = 0;
  re.count.units = 0;
  re.count.hands = 0;
  re.count.boards = 0;
}


void RefStats::incrRefEntry(
  RefEntry& re,
  const RefEntry& re2) const
{
  re.files += re2.files;
  re.noRefLines += re2.noRefLines;
  re.count.lines += re2.count.lines;
  re.count.units += re2.count.units;
  re.count.hands += re2.count.hands;
  re.count.boards += re2.count.boards;
}


void RefStats::reset()
{
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    for (unsigned tag = 0; tag < ERR_SIZE; tag++)
      RefStats::resetRefEntry(data[table][tag]);

  RefStats::resetSeen();
}


void RefStats::incr(
  const RefTables table,
  const RefTag tag,
  const RefEntry& re)
{
  data[table][tag].files += re.files;
  data[table][tag].noRefLines += re.noRefLines;
  data[table][tag].count.lines += re.count.lines;
  data[table][tag].count.units += re.count.units;
  data[table][tag].count.hands += re.count.hands;
  data[table][tag].count.boards += re.count.boards;
}


void RefStats::logFile(const RefEntry& re)
{
  RefStats::incr(REFSTATS_SOURCE, static_cast<RefTag>(0), re);
}


void RefStats::logRefFile()
{
  numSourceFiles++;
  numRefFiles++;
  resetSeen();
}


void RefStats::logSkip(
  const RefTag tag,
  const RefEntry& re)
{
  RefStats::incr(REFSTATS_SKIP, tag, re);
}


void RefStats::logNoval(
  const RefTag tag,
  const RefEntry& re)
{
  RefStats::incr(REFSTATS_NOVAL, tag, re);
}


void RefStats::logRef(
  const RefTag tag,
  RefEntry& re)
{
  re.files = (tagSeen[tag] ? 0u : 1u);
  tagSeen[tag] = true;

  RefStats::incr(REFSTATS_REF, tag, re);
}


void RefStats::operator +=(const RefStats& rf2)
{
  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
    for (unsigned tag = 0; tag < ERR_SIZE; tag++)
      RefStats::incr(static_cast<RefTables>(table), 
        static_cast<RefTag>(tag), rf2.data[table][tag]);

  numRefFiles += rf2.numRefFiles;
}


void RefStats::print(ostream& fstr) const
{
  if (numSourceFiles == 0)
    return;

  RefComment comment; // Kludge to get at tag names
  RefEntry refSum;

  string dashes;
  dashes.resize(0);
  dashes.insert(0, 70, '-');

  for (unsigned table = 0; table < REFSTATS_SIZE; table++)
  {
    fstr << RefTableNames[table] << "\n\n";

    fstr << setw(28) << left << "Reference" <<
      setw(7) << right << "Files" <<
      setw(7) << right << "Lines" <<
      setw(7) << right << "Refs" <<
      setw(7) << right << "Units" <<
      setw(7) << right << "Hands" <<
      setw(7) << right << "Boards" << "\n";

    RefStats::resetRefEntry(refSum);

    for (unsigned comm = 0; comm < ERR_SIZE; comm++)
    {
      if (data[table][comm].files == 0)
        continue;

      RefStats::incrRefEntry(refSum, data[table][comm]);

      if (table == 0)
        continue; // Only show sum

      fstr << setw(28) << left << 
        comment.comment2str(static_cast<CommentType>(comm)) <<
        setw(7) << right << data[table][comm].files <<
        setw(7) << right << data[table][comm].noRefLines <<
        setw(7) << right << data[table][comm].count.lines <<
        setw(7) << right << data[table][comm].count.units <<
        setw(7) << right << data[table][comm].count.hands <<
        setw(7) << right << data[table][comm].count.hands << "\n";
    }

    fstr << dashes << "\n";

    fstr << setw(28) << left << "Sum" <<
      setw(7) << right << refSum.files <<
      setw(7) << right << refSum.noRefLines <<
      setw(7) << right << refSum.count.lines <<
      setw(7) << right << refSum.count.units <<
      setw(7) << right << refSum.count.hands <<
      setw(7) << right << refSum.count.hands << "\n\n";
  }

  fstr << setw(28) << left << "Number of source files" <<
    setw(7) << right << numSourceFiles << "\n";
  fstr << setw(28) << left << "Number of ref files" <<
    setw(7) << right << numRefFiles << "\n\n";
}

