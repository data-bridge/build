/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FILES_H
#define BRIDGE_FILES_H

#include <string>
#include <vector>
#include <map>

#include "DDInfo.h"
#include "bconst.h"

using namespace std;


struct FileEntry
{
  string fullName;
  string base;
  Format format;
};


class Files
{
  private:

    vector<FileTask> fileTasks;
    unsigned nextNo;

    vector<DDInfo> infoDD;
    vector<string> dirList; // Sloppy to keep this

    bool fillEntry(
      const string& text,
      FileEntry& entry) const;

    void list2map(
      const vector<FileEntry>& fileList,
      map<string, vector<FileEntry>>& refMap);

    void buildFileList(
      const string& dirName,
      vector<FileEntry>& fileList,
      const Format formatOnly = BRIDGE_FORMAT_SIZE);

    Format guessLINFormat(const string& base) const;

    void extendTaskList(
      const FileEntry& in,
      const vector<FileEntry>& out,
      const bool keepFlag,
      const map<string, vector<FileEntry>>& refMap);

    void readDDInfoFile(
      const string& dir,
      const Options& options);


  public:

    Files();

    ~Files();

    void reset();

    void rewind();

    void set(const Options& options);

    bool next(FileTask& ftask);

    void print() const;

    bool haveResults(
      const DDInfoType infoNo,
      const string& fname,
      const vector<string>& casesIn,
      CaseResults& infoSeen,
      vector<string>& casesMissing) const;
      
    void addDDInfo(
      const DDInfoType infoNo,
      const string& fname,
      const vector<string>& casesMissing,
      const vector<string>& infoMissing);

    void writeDDInfo(const DDInfoType infoNo) const;
};

#endif

