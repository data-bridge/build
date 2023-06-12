/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DDINFO_H
#define BRIDGE_DDINFO_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#include <vector>
#include <list>
#include <map>
#pragma warning(pop)

#include "bconst.h"

using namespace std;

typedef map<string, string> CaseResults;

class DDInfo
{
  private:

    typedef map<string, CaseResults*> FileResults;

    struct DirEntry
    {
      FileResults * fileRes;
      string fnameDD;
      bool dirtyFlag;
    };

    list<CaseResults> caseResults;
    list<FileResults> fileResults;
    map<string, DirEntry> dirResults;


  public:

    DDInfo();

    ~DDInfo();

    void reset();

    void read(const string& resName);

    bool haveResults(
      const string& fname,
      const vector<string>& casesIn,
      CaseResults& infoSeen,
      vector<string>& casesMissing) const;

    void add(
      const string& fname,
      const vector<string>& casesMissing,
      const vector<string>& infoMissing);

    void write(const string& fnameDD = "tableaux.log") const;
};

#endif

