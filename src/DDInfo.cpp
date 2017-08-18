/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

#include "DDInfo.h"
#include "parse.h"
#include "Bexcept.h"


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
  boardResults.clear();
}


void DDInfo::read(const string& resName)
{
  ifstream resstr(resName.c_str());
  if (! resstr.is_open())
    THROW("File " + resName + " cannot be opened");

  const string path = filepath(resName);
  auto itDir = dirResults.find(path);
  if (itDir != dirResults.end())
    THROW("Already looked in directory " + path);

  fileResults.emplace_back(FileResults());
  FileResults& fres = fileResults.back();

  itDir->second.fileRes = &fres;
  itDir->second.fnameDD = basefile(resName);
  itDir->second.dirtyFlag = false;

  string line;
  BoardResults * bres = nullptr;

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
      
      boardResults.emplace_back(BoardResults());
      bres = &boardResults.back();

      itFile->second = bres;
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


bool DDInfo::boardsHaveResults(
  const string& fname,
  const vector<string>& boardsIn,
  vector<string>& boardsMissing) const
{
  const string path = filepath(fname);
  const string base = basefile(fname);

  auto itDir = dirResults.find(path);
  if (itDir == dirResults.end())
  {
    boardsMissing = boardsIn;
    return false;
  }

  const FileResults& fres = *(itDir->second.fileRes);
  auto itFile = fres.find(base);
  if (itFile == fres.end())
  {
    boardsMissing = boardsIn;
    return false;
  }

  const BoardResults& bres = *(itFile->second);
  boardsMissing.clear();
  for (auto &it: boardsIn)
  {
    auto itBoard = bres.find(it);
    if (itBoard == bres.end())
      boardsMissing.push_back(it);
  }

  return (boardsMissing.size() == 0);
}


void DDInfo::add(
  const string& fname,
  const vector<string>& boardsMissing,
  const vector<string>& infoMissing)
{
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
    fresp = itDir->second.fileRes;

  FileResults& fres = * fresp;
  const string base = basefile(fname);

  BoardResults * bresp;
  auto itFile = fres.find(base);
  if (itFile == fres.end())
  {
    boardResults.emplace_back(BoardResults());
    bresp = &boardResults.back();
    fres[base] = bresp;
  }
  else
    bresp = itFile->second;
  
  BoardResults& bres = * bresp;
  for (unsigned i = 0; i < boardsMissing.size(); i++)
  {
    auto itBoard = bres.find(boardsMissing[i]);
    if (itBoard == bres.end())
      bres[boardsMissing[i]] = infoMissing[i];
    else
      THROW("Attempting to reset an existing DD value");
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
      BoardResults& bres = *(itFile.second);

      dd << inFile << ":\n";
      for (auto const &itBoard: bres)
        dd << itBoard.first << " " << itBoard.second << "\n";
      dd << "\n";
    }
    dd.close();
  }
}
