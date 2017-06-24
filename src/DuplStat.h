/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DUPLSTAT_H
#define BRIDGE_DUPLSTAT_H

#include <iostream>
#include <list>


class Group;
class Segment;
class RefLines;


using namespace std;


class DuplStat
{
  private:

    const Group * groupPtr;
    const Segment * segmentPtr;
    unsigned segNoVal;
    const RefLines * reflinesPtr;
    
    list<unsigned> values;

  public:

    DuplStat();

    ~DuplStat();

    void reset();

    void set(
      const Group * group,
      const Segment * segment,
      const unsigned segNo,
      const RefLines * reflines);

    void append(const int hashVal);

    void sort();

    unsigned first() const;

    bool sameOrigin(const DuplStat& ds2) const;
    bool lexLessThan(const DuplStat& ds2) const;
    bool operator ==(const DuplStat& ds2) const;
    bool operator <=(const DuplStat& ds2) const;

    string str() const;
    string strSuggest(const string& tag) const;
};

#endif

