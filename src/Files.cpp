/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <map>
#include <regex>
#include <thread>
#include <mutex>
#include <stdlib.h>
#include <assert.h>
#include "dirent.h"

#include <windows.h>
#include <iostream>
#include "Shlwapi.h"

#include "Files.h"
#include "Debug.h"
#include "portab.h"
#include "parse.h"

extern Debug debug;

mutex mtx;


Files::Files()
{
  Files::Reset();
}


Files::~Files()
{
}


void Files::Reset()
{
  nextNo = 0;
  fileTasks.clear();
}


void Files::Rewind()
{
  nextNo = 0;
}


bool Files::FillEntry(
  const string& s,
  FileEntryType& entry) const
{
  regex re("([^./]+)\\.(\\w+)$");
  smatch match;
  if (regex_search(s, match, re) && match.size() >= 2)
  {
    entry.fullName = s;
    entry.base = match.str(1);
    entry.format = ExtToFormat(match.str(2));
    return (entry.format != BRIDGE_FORMAT_SIZE);
  }
  else
    return false;
}


void Files::BuildFileList(
  const string& dirName,
  vector<FileEntryType>& fileList,
  const formatType formatOnly)
{
  DIR *dir;
  struct dirent *ent;
  FileEntryType entry;

  if ((dir = opendir(dirName.c_str())) == nullptr) 
    return;

  while ((ent = readdir(dir)) != nullptr) 
  {
    string s = dirName + "/" + string(ent->d_name);
    switch(ent->d_type)
    {
      case DT_REG:
        if (Files::FillEntry(s, entry))
        {
          if (formatOnly == BRIDGE_FORMAT_SIZE || 
              entry.format == formatOnly)
          fileList.push_back(entry);
        }
        break;

      case DT_DIR:
        if (strcmp(ent->d_name, ".") != 0 &&
            strcmp(ent->d_name, "..") != 0)
        {
          BuildFileList(s, fileList, formatOnly);
        }

      default:
        break;
    }
  }
  closedir(dir);
}


void Files::ListToMap(
  const vector<FileEntryType>& fileList,
  map<string, vector<FileEntryType>>& refMap)
{
  // Index the file list by basename.
  for (auto &e: fileList)
    refMap[e.base].push_back(e);
}


formatType Files::GuessLINFormat(const string& base) const
{
 // Guess the specific LIN output dialect from the filename.

  regex re1("^(\\d+)$");
  regex re2("^(Tournament)");
  regex re3("^([SUVW]\\d\\d\\w\\w\\w)$");
  smatch match;

  if (regex_search(base, match, re1))
    return BRIDGE_FORMAT_LIN_VG;
  else if (regex_search(base, match, re2))
    return BRIDGE_FORMAT_LIN_TRN;
  else if (regex_search(base, match, re3))
    return BRIDGE_FORMAT_LIN_RP;
  else
    return BRIDGE_FORMAT_LIN;
}


void Files::ExtendTaskList(
  const FileEntryType& in,
  const vector<FileEntryType>& out,
  const bool keepFlag,
  const map<string, vector<FileEntryType>>& refMap)
{
  FileTaskType t;

  t.fileInput = in.fullName;
  t.formatInput = in.format;
  t.removeOutputFlag = ! keepFlag;

  for (auto &e: out)
  {
    FileOutputTaskType o;
    o.fileOutput = e.fullName;

    if (e.format == BRIDGE_FORMAT_LIN)
      o.formatOutput = Files::GuessLINFormat(in.base);
    else
      o.formatOutput = e.format;

    o.refFlag = false;
    auto it = refMap.find(in.base);
    if (it != refMap.end())
    {
      for (auto &f: it->second)
      {
        if (f.format == e.format)
        {
          o.refFlag = true;
          o.fileRef = f.fullName;
          break;
        }
      }
    }

    t.taskList.push_back(o);
  }

  fileTasks.push_back(t);
}


void Files::Set(const OptionsType& options)
{
  vector<FileEntryType> inputList, refList, outputList;
  map<string, vector<FileEntryType>> refMap;

  // Set inputList
  if (options.fileInput.setFlag)
  {
    FileEntryType e;
    Files::FillEntry(options.fileInput.name, e);
    inputList.push_back(e);
  }
  else
    Files::BuildFileList(options.dirInput.name, inputList);

  // Set refMap, a map of reference files indexed by basename
  if (options.fileRef.setFlag)
  {
    FileEntryType e;
    Files::FillEntry(options.fileRef.name, e);
    refList.push_back(e);
  }
  else if (options.dirRef.setFlag)
    Files::BuildFileList(options.dirRef.name, refList, options.format);

  Files::ListToMap(refList, refMap);

  // Set output list and list of output formats
  vector<formatType> flist;
  FileEntryType oe;
  bool keepFlag;
  string prefix;

  if (options.fileOutput.setFlag)
  {
    keepFlag = true;
    prefix = "";
    Files::FillEntry(options.fileOutput.name, oe);

    outputList.push_back(oe);
    flist.push_back(oe.format);

    // Only one entry in the inputList.
    for (auto &i: inputList)
      Files::ExtendTaskList(i, outputList, keepFlag, refMap);
  }
  else
  {
    keepFlag = options.dirOutput.setFlag;
    prefix = (keepFlag ? options.dirOutput.name + "/" : "tmp");

    if (options.formatSetFlag)
      flist.push_back(options.format);
    else
    {
      flist.push_back(BRIDGE_FORMAT_LIN);
      flist.push_back(BRIDGE_FORMAT_PBN);
      flist.push_back(BRIDGE_FORMAT_RBN);
      flist.push_back(BRIDGE_FORMAT_RBX);
      flist.push_back(BRIDGE_FORMAT_TXT);
      flist.push_back(BRIDGE_FORMAT_EML);
      flist.push_back(BRIDGE_FORMAT_REC);
    }

    // Connect the dots
    for (auto &i: inputList)
    {
      outputList.clear();

      for (auto &f: flist)
      {
        string oname = prefix + i.base + "." + FORMAT_NAMES[f];
        Files::FillEntry(oname, oe);
        outputList.push_back(oe);
      }

      Files::ExtendTaskList(i, outputList, keepFlag, refMap);
    }
  }
}


bool Files::GetNextTask(FileTaskType& ftask) 
{
  if (nextNo >= fileTasks.size())
    return false;

  mtx.lock();
  ftask = fileTasks[nextNo++];
  mtx.unlock();
  return true;
}


void Files::PrintTasks() const
{
  for (auto &i: fileTasks)
  {
    cout << "fileInput    " << i.fileInput << endl;
    cout << "formatInput  " << FORMAT_NAMES[i.formatInput] << endl;
    cout << "removeOutput " << (i.removeOutputFlag ? "true" : "false")
        << endl;
    
    for (auto &t: i.taskList)
    {
      cout << "  fileOutput   " << t.fileOutput << endl;
      cout << "  formatOutput " << FORMAT_NAMES[t.formatOutput] << endl;
      if (t.refFlag)
        cout << "  fileRef      " << t.fileRef << endl;
      else
        cout << "  no ref" << endl;
    }
    cout << endl;
  }
}

