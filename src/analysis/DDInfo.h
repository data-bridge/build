/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DDINFO_H
#define BRIDGE_DDINFO_H

#include <string>
#include <vector>
#include <list>
#include <map>

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

