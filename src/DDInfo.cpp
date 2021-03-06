/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#if defined(_WIN32) && defined(__MINGW32__)
  #include "mingw.thread.h"
  #include "mingw.mutex.h"
#else
  #include <thread>
  #include <mutex>
#endif

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "DDInfo.h"
#include "parse.h"
#include "Bexcept.h"

static mutex mtx;


DDInfo::DDInfo()
{
  DDInfo::reset();
}


DDInfo::~DDInfo()
{
}


void DDInfo::reset()
{
  dirResults.clear();
  fileResults.clear();
  caseResults.clear();
}


void DDInfo::read(const string& resName)
{
  ifstream resstr(resName.c_str());
  if (! resstr.is_open())
    return; // File not found

  const string path = filepath(resName);
  auto itDir = dirResults.find(path);
  if (itDir != dirResults.end())
    THROW("Already looked in directory " + path);

  fileResults.emplace_back(FileResults());
  FileResults& fres = fileResults.back();

  dirResults[path].fileRes = &fres;
  dirResults[path].fnameDD = basefile(resName);
  dirResults[path].dirtyFlag = false;

  string line;
  CaseResults * bres = nullptr;

  while (getline(resstr, line))
  {
    if (line.empty() || line.front() == '%')
      continue;
    
    if (line.back()  == ':')
    {
      const string fname = line.substr(0, line.size()-1);
      auto itFile = fres.find(fname);
      if (itFile != fres.end())
        THROW("In directory " + path + ", file " + fname + " already seen");
      
      caseResults.emplace_back(CaseResults());
      bres = &caseResults.back();
      fres[fname] = bres;
      continue;
    }

    if (bres == nullptr)
      THROW("In directory " + path + ", no file is entered in " + resName);
    
    const size_t c = countDelimiters(line, " ");
    if (c != 1)
      THROW("Need exactly one space in line " + line);

    vector<string> tokens(2);
    tokens.clear();
    tokenize(line, tokens, " ");

    (*bres)[tokens[0]] = tokens[1];
  }

  resstr.close();
}


bool DDInfo::haveResults(
  const string& fname,
  const vector<string>& casesIn,
  CaseResults& infoSeen,
  vector<string>& casesMissing) const
{
  const string path = filepath(fname);
  const string base = basefile(fname);

  auto itDir = dirResults.find(path);
  if (itDir == dirResults.end())
  {
    infoSeen.clear();
    casesMissing = casesIn;
    return false;
  }

  const FileResults& fres = *(itDir->second.fileRes);
  auto itFile = fres.find(base);
  if (itFile == fres.end())
  {
    infoSeen.clear();
    casesMissing = casesIn;
    return false;
  }

  const CaseResults& bres = *(itFile->second);
  casesMissing.clear();
  for (auto &it: casesIn)
  {
    auto itCase = bres.find(it);
    if (itCase == bres.end())
      casesMissing.push_back(it);
    else
      infoSeen[itCase->first] = itCase->second;
  }

  return (casesMissing.size() == 0);
}


void DDInfo::add(
  const string& fname,
  const vector<string>& casesMissing,
  const vector<string>& infoMissing)
{
  mtx.lock();

  FileResults * fresp;
  const string path = filepath(fname);
  auto itDir = dirResults.find(path);
  if (itDir == dirResults.end())
  {
    fileResults.emplace_back(FileResults());
    fresp = &fileResults.back();
    dirResults[path] = {fresp, "", true};
  }
  else
  {
    fresp = itDir->second.fileRes;
    dirResults[path].dirtyFlag = true;
  }

  FileResults& fres = * fresp;
  const string base = basefile(fname);

  CaseResults * bresp;
  auto itFile = fres.find(base);
  if (itFile == fres.end())
  {
    caseResults.emplace_back(CaseResults());
    bresp = &caseResults.back();
    fres[base] = bresp;
  }
  else
    bresp = itFile->second;
  
  mtx.unlock();

  CaseResults& bres = * bresp;
  for (unsigned i = 0; i < casesMissing.size(); i++)
  {
    auto itCase = bres.find(casesMissing[i]);
    if (itCase == bres.end())
      bres[casesMissing[i]] = infoMissing[i];
    else
      THROW("Attempting to reset an existing DD value: " + fname +
        ", " + STR(i) + ", " + STR(casesMissing[i]));
  }
}


void DDInfo::write(const string& resName) const
{
  for (auto const &itDir: dirResults)
  {
    const DirEntry& dirEntry = itDir.second;
    if (! dirEntry.dirtyFlag)
      continue;

    const string fullName = itDir.first + 
      (resName == "" ? dirEntry.fnameDD : resName);

    ofstream dd;
    dd.open(fullName);

    FileResults& fres = *(dirEntry.fileRes);
    for (auto const &itFile: fres)
    {
      const string inFile = itFile.first;
      CaseResults& bres = *(itFile.second);

      dd << inFile << ":\n";
      for (auto const &itCase: bres)
        dd << itCase.first << " " << itCase.second << "\n";
      dd << "\n";
    }
    dd.close();
  }
}

