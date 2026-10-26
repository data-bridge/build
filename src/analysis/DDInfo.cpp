/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <mutex>

#include "DDInfo.h"

#include "../records/SDPoints.h"
#include "../util/parse.h"
#include "../handling/Bexcept.h"

static mutex mtx;


DDInfo::DDInfo()
{
  DDInfo::reset();
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

  string path = filepath(resName);
  if (! path.empty() && path.back() == '/')
    path.pop_back();

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


void DDInfo::readLeads(const string& resName)
{
  ifstream resstr(resName.c_str());
  if (! resstr.is_open())
    return; // File not found

  string path = filepath(resName);
  if (! path.empty() && path.back() == '/')
    path.pop_back();

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

  string boardNo;
  string boardData;

  while (getline(resstr, line))
  {
    if (! line.empty() && line.front() == '%')
      continue;
    
    if (line.back()  == ':')
    {
      if (! boardNo.empty())
      {
        (*bres)[boardNo] = boardData;
        boardNo.clear();
        boardData.clear();
      }

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

    if (line.empty())
    {
      if (! boardNo.empty())
      {
        (*bres)[boardNo] = boardData;
        boardNo.clear();
        boardData.clear();
      }
      continue;
    }

    if (boardNo.empty())
    {
      boardNo = line;
      while (!boardNo.empty() && boardNo.back() == ' ')
        boardNo.pop_back();
    }
    else
    {
      if (! boardData.empty())
        boardData += "\n";
      boardData += line;
    }
  }

  if (! boardNo.empty())
    (*bres)[boardNo] = boardData;

  resstr.close();
}


bool DDInfo::haveResults(
  const string& fname,
  const vector<string>& casesIn,
  CaseResults& infoSeen,
  vector<string>& casesMissing) const
{
  string path = filepath(fname);
  if (! path.empty() && path.back() == '/')
    path.pop_back();

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
        ", " + to_string(i) + ", " + casesMissing[i]);
  }
}


void DDInfo::addSingleDummyPoints(
  const string& fname,
  const string& boardName,
  const vector<SDPoints>& points)
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
  auto itCase = bres.find(boardName);
  if (itCase == bres.end())
  {
    bres[boardName] = 
      "0: " + points[0].str() + " " +
      "0: " + points[1].str() + " " +
      "0: " + points[2].str() + " " +
      "0: " + points[3].str();
  }
  else
    THROW("Attempting to reset an existing DD value: " + fname +
      ", " + boardName);
}


// To make the board numbers come out in the expected order.

struct Key
{
    char prefix;  // 'o', 'c', or 0 for plain numbers
    int num;
};


Key parseKey(const string& s)
{
  if (!s.empty() && (s[0] == 'o' || s[0] == 'c'))
    return {s[0], std::stoi(s.substr(1))};

  return {'\0', std::stoi(s)};
}


bool lessKey(
  const pair<string, string>& a,
  const pair<string, string>& b)
{
  Key ka = parseKey(a.first);
  Key kb = parseKey(b.first);

  // Plain numeric case
  if (ka.prefix == '\0' && kb.prefix == '\0')
    return ka.num < kb.num;

  // o/c case
  if (ka.num != kb.num)
    return ka.num < kb.num;

  return ka.prefix > kb.prefix;  // 'o' > 'c', so o1 before c1
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

      vector<pair<string, string>> v(bres.begin(), bres.end());
      sort(v.begin(), v.end(), lessKey);

      // for (auto const &itCase: bres)
      for (auto const &itCase: v)
        dd << itCase.first << " " << itCase.second << "\n";
      dd << "\n";
    }
    dd.close();
  }
}

