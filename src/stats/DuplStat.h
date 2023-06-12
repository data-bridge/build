/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DUPLSTAT_H
#define BRIDGE_DUPLSTAT_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <list>
#pragma warning(pop)

#include "../bconst.h"


class Group;
class Segment;
class RefLines;


using namespace std;


class DuplStat
{
  private:

    string fname;
    string basename;
    Format format;

    string segTitle;
    string segDate;
    string segLocation;
    string segEvent;
    string segSession;

    string teams;
    string players;
    unsigned segNoVal;
    unsigned segSize;

    bool playersFlag;
    vector<string> pnames;

    unsigned numLines;
    unsigned numHands;
    unsigned numBoards;
    
    list<unsigned> values;

    void extractPlayers();

    bool similarPlayerGroup(
      const DuplStat& ds2,
      const size_t number,
      const size_t ds2offset) const;

    bool similarPlayers(const DuplStat& ds2) const;

    string strRef() const;
    string strDiff(
      const string& snew,
      const string& sold) const;

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
    string str(const DuplStat& ds2) const;
    string strSuggest(const bool fullFlag) const;
};

#endif

