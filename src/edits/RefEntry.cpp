/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#include <iomanip>
#include <sstream>

#include "RefEntry.h"

using namespace std;


void RefEntry::reset()
{
  files = 0;
  noRefLines = 0;
  count.reset();
}


void RefEntry::setFiles(const size_t filesIn)
{
  files = filesIn;
}


void RefEntry::setFileData(
  const size_t filesIn,
  const size_t noRefLinesIn)
{
  files = filesIn;
  noRefLines = noRefLinesIn;
}


void RefEntry::setLines(const size_t linesIn)
{
  count.setLines(linesIn);
}


void RefEntry::setUnitsFrom(const RefEntry& re2)
{
  count.setUnitsFrom(re2.count);
}


void RefEntry::set(
  const size_t unitsIn,
  const size_t handsIn,
  const size_t boardsIn)
{
  files = 1;
  noRefLines = 1;
  count.set(unitsIn, handsIn, boardsIn);
}


void RefEntry::fixSomething()
{
  // TODO Why is this needed in Reflines?
  count.fixSomething();
}


size_t RefEntry::getFiles() const
{
  return files;
}


size_t RefEntry::getUnits() const
{
  return count.getUnits();
}


void RefEntry::operator += (const RefEntry& re2)
{
  files += re2.files;
  noRefLines += re2.noRefLines;
  count += re2.count;
}


bool RefEntry::sameCountValues(const RefEntry& re2) const
{
  return count.sameValues(re2.count);
}


string RefEntry::strCount() const
{
  return count.str();
}


string RefEntry::strCountShort() const
{
  return count.strShort();
}


string RefEntry::strLineHeader() const
{
  stringstream ss;
  ss <<
    setw(7) << right << "Files" <<
    setw(7) << right << "Refs" <<
    count.strLineHeader() << "\n";
  return ss.str();
}


string RefEntry::strLine() const
{
  stringstream ss;
  ss <<
    setw(7) << right << files <<
    setw(7) << right << noRefLines <<
    count.strLine() << "\n";
  return ss.str();
}

