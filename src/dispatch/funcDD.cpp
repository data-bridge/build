/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iomanip>
#include <array>
#include <sstream>
#include <fstream>
#pragma warning(pop)

#include "../Group.h"
#include "../Files.h"
#include "../parse.h"

#include "../dll.h"
#include "../Bexcept.h"


using namespace std;


void makeTableau(
  ddTableDealsPBN * tablePBN,
  array<Board *, MAXNOOFTABLES> bpMissing,
  vector<string>& infoMissing)
{
  ddTablesRes resDDS;
  tableauDD(tablePBN, &resDDS);

  for (int i = 0; i < tablePBN->noOfTables; i++)
  {
    const size_t u = static_cast<unsigned>(i);
    bpMissing[u]->setTableauDDS(resDDS.results[i].resTable);

    const string s = bpMissing[u]->strTableau(BRIDGE_FORMAT_RBN);
    infoMissing.push_back(s.substr(4, s.length()-6));
  }

  tablePBN->noOfTables = 0;
}


void str2Board(
  const string& st,
  Segment * segment,
  Board ** bd)
{
  unsigned extNo;
  if (! str2unsigned(st, extNo))
    THROW("Not a number: " + st);

  if (segment->getBoard(extNo) == nullptr)
    THROW("Bad board");
  * bd = segment->acquireBoard(extNo);
}


void makeDD(
  Group& group,
  Files& files,
  const string& fname)
{
  // It is a bug that multiple segments in an input file which use
  // overlapping board numbers will fail.  This doesn't happen in
  // the input files I use, so I haven't fixed DDInfo.

  ddTableDealsPBN tablePBN;
  tablePBN.noOfTables = 0;

  unsigned segNo = 0;
  for (auto segment = group.mbegin(); segment != group.mend(); segment++)
  {
    if (segment->size() == 0)
      continue;
    segNo++;

    vector<string> boardsIn;
    for (auto &bp: * segment)
      boardsIn.push_back(to_string(bp.extNo));
    
    vector<string> boardsMissAll, boardsMissing, infoMissing;
    CaseResults infoSeen;
    array<Board *, MAXNOOFTABLES> bpMissing;
    files.haveResults(BRIDGE_DD_INFO_SOLVE, fname, 
        boardsIn, infoSeen, boardsMissAll);
    
    Segment * segptr = &*segment;
    for (auto& itSeen: infoSeen)
    {
      // Fill out the instance traces from file memory.
      Board * bd;
      str2Board(itSeen.first, segptr, &bd);
      bd->setTableau("::" + itSeen.second, BRIDGE_FORMAT_RBN);
    }

    for (string extStr: boardsMissAll)
    {
      Board * bd;
      str2Board(extStr, segptr, &bd);

      string tmp = bd->strDeal(BRIDGE_WEST,BRIDGE_FORMAT_PBN);
      const size_t l = tmp.length();
      if (l < 40)
        THROW("Not PBN: " + tmp);

      // Skip "Deal [" and the trailing stuff.
      copy(tmp.begin()+7, tmp.end()-3, 
        tablePBN.deals[tablePBN.noOfTables].cards);
      tablePBN.deals[tablePBN.noOfTables].cards[l-10] = '\0';

      boardsMissing.push_back(extStr);
      bpMissing[static_cast<unsigned>(tablePBN.noOfTables)] = bd;

      tablePBN.noOfTables++;

      if (tablePBN.noOfTables == MAXNOOFTABLES)
      {
        makeTableau(&tablePBN, bpMissing, infoMissing);
        files.addDDInfo(BRIDGE_DD_INFO_SOLVE, fname, 
          boardsMissing, infoMissing);
        tablePBN.noOfTables = 0;
        boardsMissing.clear();
        infoMissing.clear();
      }
    }

    if (tablePBN.noOfTables > 0)
    {
      // Stragglers.
      makeTableau(&tablePBN, bpMissing, infoMissing);
      files.addDDInfo(BRIDGE_DD_INFO_SOLVE, fname, 
        boardsMissing, infoMissing);
    }
  }
}


void dispatchDD(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog)
{
  try
  {
    makeDD(group, files, fname);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

