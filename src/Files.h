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

    bool fillEntry(
      const string& text,
      FileEntry& entry,
      const bool checkSkip = false) const;

    void list2map(
      const vector<FileEntry>& fileList,
      map<string, vector<FileEntry>>& refMap);

    void buildFileList(
      const string& dirName,
      vector<FileEntry>& fileList,
      const Format formatOnly = BRIDGE_FORMAT_SIZE,
      const bool checkSkip = false);

    Format guessLINFormat(const string& base) const;

    void extendTaskList(
      const FileEntry& in,
      const vector<FileEntry>& out,
      const bool keepFlag,
      const map<string, vector<FileEntry>>& refMap);


  public:

    Files();

    ~Files();

    void reset();

    void rewind();

    void set(const Options& options);

    bool next(FileTask& ftask);

    void print() const;

};

#endif

