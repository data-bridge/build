/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FILES_H
#define BRIDGE_FILES_H

#include "bconst.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <vector>
#include <map>

using namespace std;


struct FileEntryType
{
  string fullName;
  string base;
  formatType format;
};


class Files
{
  private:

    vector<FileTaskType> fileTasks;
    unsigned nextNo;

    bool FillEntry(
      const string& s,
      FileEntryType& entry) const;

    void ListToMap(
      const vector<FileEntryType>& fileList,
      map<string, vector<FileEntryType>>& refMap);

    void BuildFileList(
      const string& dirName,
      vector<FileEntryType>& fileList,
      const formatType formatOnly = BRIDGE_FORMAT_SIZE);

    formatType GuessLINFormat(const string& base) const;

    void ExtendTaskList(
      const FileEntryType& in,
      const vector<FileEntryType>& out,
      const bool keepFlag,
      const map<string, vector<FileEntryType>>& refMap);


  public:

    Files();

    ~Files();

    void Reset();

    void Set(const OptionsType& options);

    bool GetNextTask(FileTaskType& ftask);

    void PrintTasks() const;

};

#endif

