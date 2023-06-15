/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef REF_ENTRY_H
#define REF_ENTRY_H

#include <string>

#include "RefCount.h"

using namespace std;


class RefEntry
{
  private:

    size_t files;

    size_t noRefLines;

    RefCount count;

  public:

    void reset();

    void setFiles(const size_t files);

    void setFileData(
      const size_t files,
      const size_t noRefLines);

    void setLines(const size_t linesIn);

    void setUnitsFrom(const RefEntry& re2);

    void set(
      const size_t unitsIn,
      const size_t handsIn,
      const size_t boardsIn);

    // TODO Why is this needed in Reflines?
    void fixSomething();

    size_t getFiles() const;

    size_t getUnits() const;

    void operator += (const RefEntry& re2);

    bool sameCountValues(const RefEntry& re2) const;

    string strCount() const;

    string strCountShort() const;

    string strLineHeader() const;

    string strLine() const;
};

#endif
