/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef REF_COUNT_H
#define REF_COUNT_H

#include <string>

using namespace std;


class RefCount
{
  private:

    size_t lines;
    size_t units;
    size_t hands; // qx
    size_t boards; // bd

  public:

    void reset();

    void setLines(size_t linesIn);

    void setUnitsFrom(const RefCount& rc2);

    void set(
      size_t unitsIn,
      size_t handsIn,
      size_t boardsIn);

    // TODO Why is this needed in Reflines?
    void fixSomething();

    void operator += (const RefCount& rc2);

    size_t getUnits() const;

    bool sameValues(const RefCount& rc2) const;

    string str() const;

    string strShort() const;

    string strLineHeader() const;

    string strLine() const;
};

#endif
