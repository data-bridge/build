/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DUPLSTATS_H
#define BRIDGE_DUPLSTATS_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <list>
#pragma warning(pop)

#include "DuplStat.h"
#include "bconst.h"


class Group;
class Segment;


using namespace std;


class DuplStats
{
  private:

    struct DuplElem
    {
      DuplStat stat;
      bool activeFlag;
    };

    list<DuplElem> statList;
    DuplElem * activeList;

    vector<vector<DuplElem *>> hashedList;

  public:

    DuplStats();

    ~DuplStats();

    void reset();

    void set(
      const Group * group,
      const Segment * segment,
      const unsigned segNo,
      const RefLines * reflines);

    void append(const int hashVal);

    void sortActive();

    void operator += (const DuplStats& dupl2);

    void sortOverall();

    string strSame() const;
    string strSubset() const;

    void print(ostream& fstr) const;
};

#endif

