/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DDINFO_H
#define BRIDGE_DDINFO_H

#include <string>
#include <vector>
#include <list>
#include <map>

#include "bconst.h"

using namespace std;


class DDInfo
{
  private:

    typedef map<unsigned, string> BoardResults;
    typedef map<string, BoardResults*> FileResults;

    struct DirEntry
    {
      FileResults * fileRes;
      string fnameDD;
      bool dirtyFlag;
    };

    list<BoardResults> boardResults;
    list<FileResults> fileResults;
    map<string, DirEntry> dirResults;


  public:

    DDInfo();

    ~DDInfo();

    void reset();

    void read(const string& resName);

    bool boardsHaveResults(
      const string& fname,
      const vector<unsigned>& boardsIn,
      vector<unsigned>& boardsMissing) const;

    void add(
      const string& fname,
      const vector<unsigned>& boardsMissing,
      const vector<string>& infoMissing);

    void write(const string& fnameDD = "tableaux.log") const;
};

#endif

