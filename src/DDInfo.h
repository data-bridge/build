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

using namespace std;


class DDInfo
{
  private:

    typedef map<string, string> BoardResults;
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
      const vector<string>& boardsIn,
      vector<string>& boardsMissing) const;

    void add(
      const string& fname,
      const vector<string>& boardsMissing,
      const vector<string>& infoMissing);

    void write(const string& fnameDD = "tableau.log") const;
};

#endif
