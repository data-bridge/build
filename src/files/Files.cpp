/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>
#include <mutex>

#include "Files.h"

#include "../parse.h"
#include "../control/Options.h"

static mutex mtx;


Files::Files()
{
  Files::reset();
}


void Files::reset()
{
  nextNo = 0;
  fileTasks.clear();
  infoDD.clear();
}


void Files::rewind()
{
  nextNo = 0;
}


bool Files::fillEntry(
  const string& text,
  FileEntry& entry) const
{
  regex re("([^./]+)\\.(\\w+)$");
  smatch match;
  if (regex_search(text, match, re) && match.size() >= 2)
  {
    entry.fullName = text;
    entry.base = match.str(1);
    entry.format = ext2format(match.str(2));

    if (entry.format == BRIDGE_FORMAT_LIN)
      entry.format = Files::guessLINFormat(entry.base);

    if (entry.format == BRIDGE_FORMAT_SIZE)
      return false;
    
    return true;
  }
  else
    return false;
}


void Files::buildFileList(
  const string& dirName,
  vector<FileEntry>& fileList,
  const Format formatOnly)
{
  FileEntry entry;
  const filesystem::path dir(dirName);

  if (! filesystem::exists(dir) || ! filesystem::is_directory(dir))
    return;
  
  for (const auto& elem: filesystem::directory_iterator(dir))
  {
    if (elem.is_directory())
    {
      Files::buildFileList(elem.path().string(), fileList, formatOnly);
    }
    else if (Files::fillEntry(elem.path().string(), entry))
    {
      if (formatOnly == BRIDGE_FORMAT_SIZE ||
          entry.format == formatOnly)
        fileList.push_back(entry);
    }
  }
}


void Files::list2map(
  const vector<FileEntry>& fileList,
  map<string, vector<FileEntry>>& refMap)
{
  // Index the file list by basename.
  for (auto &e: fileList)
    refMap[e.base].push_back(e);
}


Format Files::guessLINFormat(const string& base) const
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


void Files::extendTaskList(
  const FileEntry& in,
  const vector<FileEntry>& out,
  const bool keepFlag,
  const map<string, vector<FileEntry>>& refMap)
{
  FileTask t;

  t.fileInput = in.fullName;
  t.formatInput = in.format;
  t.removeOutputFlag = ! keepFlag;

  for (auto &e: out)
  {
    FileOutputTask o;
    o.fileOutput = e.fullName;
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


void Files::readDDInfoFile(
  const string& dir,
  const Options& options)
{
  if (options.solveFlag)
  {
    const string resName = dir + "/" + DDInfoNames[BRIDGE_DD_INFO_SOLVE];
    infoDD[BRIDGE_DD_INFO_SOLVE].read(resName);
  }

  if (options.traceFlag)
  {
    const string resName = dir + "/" + DDInfoNames[BRIDGE_DD_INFO_TRACE];
    infoDD[BRIDGE_DD_INFO_TRACE].read(resName);
  }
}


void Files::set(const Options& options)
{
  vector<FileEntry> inputList, refList, outputList;
  map<string, vector<FileEntry>> refMap;

  if (options.solveFlag || options.traceFlag)
    infoDD.resize(BRIDGE_DD_INFO_SIZE);

  // Set inputList
  if (options.fileInput.setFlag)
  {
    FileEntry e;
    if (Files::fillEntry(options.fileInput.name, e))
    {
      inputList.push_back(e);
      const string dir = filepath(options.fileInput.name);
      Files::readDDInfoFile(dir, options);
    }
  }
  else
  {
    dirList.clear();
    Files::buildFileList(options.dirInput.name, inputList, 
      BRIDGE_FORMAT_SIZE);

    if (options.solveFlag || options.traceFlag)
    {
      for (auto &s: dirList)
        Files::readDDInfoFile(s, options);
    }
  }

  // Set refMap, a map of reference files indexed by basename
  if (options.fileRef.setFlag)
  {
    FileEntry e;
    if (Files::fillEntry(options.fileRef.name, e))
      refList.push_back(e);
  }
  else if (options.dirRef.setFlag)
    Files::buildFileList(options.dirRef.name, refList, options.format);

  Files::list2map(refList, refMap);

  // Set output list and list of output formats
  vector<Format> flist;
  FileEntry oe;
  bool keepFlag;

  if (options.fileOutput.setFlag)
  {
    keepFlag = true;
    if (! Files::fillEntry(options.fileOutput.name, oe))
      return;

    outputList.push_back(oe);
    flist.push_back(oe.format);

    // Only one entry in the inputList.
    for (auto &i: inputList)
      Files::extendTaskList(i, outputList, keepFlag, refMap);
  }
  else
  {
    keepFlag = options.dirOutput.setFlag;

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
      const string iprefix = options.dirOutput.name + "/" +
        FORMAT_NAMES[i.format] + "/" + i.base + ".";

      for (auto &f: flist)
      {
        if (keepFlag)
        {
          string oname = iprefix + FORMAT_EXTENSIONS[f];
          Files::fillEntry(oname, oe);
        }
        else
        {
          oe.fullName = "";
          oe.base = "";
          if (f == BRIDGE_FORMAT_LIN)
            oe.format = Files::guessLINFormat(i.base);
          else
            oe.format = f;
        }

        outputList.push_back(oe);
      }

      Files::extendTaskList(i, outputList, keepFlag, refMap);
    }
  }
}


bool Files::next(FileTask& ftask) 
{
  if (nextNo >= fileTasks.size())
    return false;

  mtx.lock();
  ftask = fileTasks[nextNo++];
  mtx.unlock();
  return true;
}


void Files::print() const
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


bool Files::haveResults(
  const DDInfoType infoNo,
  const string& fname,
  const vector<string>& casesIn,
  CaseResults& infoSeen,
  vector<string>& casesMissing) const
{
  return infoDD[infoNo].haveResults(fname, casesIn, infoSeen, 
    casesMissing);
}


void Files::addDDInfo(
  const DDInfoType infoNo,
  const string& fname,
  const vector<string>& casesMissing,
  const vector<string>& infoMissing)
{
  infoDD[infoNo].add(fname, casesMissing, infoMissing);
}


void Files::writeDDInfo(const DDInfoType infoNo) const
{
  infoDD[infoNo].write(DDInfoNames[infoNo]);
}

