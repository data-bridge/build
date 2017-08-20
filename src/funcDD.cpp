/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <array>
#include <sstream>
#include <fstream>

#include "Group.h"
#include "Files.h"

#include "dll.h"
#include "Bexcept.h"


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


void makeDD(
  const DDInfoType infoNo,
  Group& group,
  Files& files,
  const string& fname)
{
  ddTableDealsPBN tablePBN;
  if (infoNo == BRIDGE_DD_INFO_TRACE)
    THROW("Trace not yet implemented");
  tablePBN.noOfTables = 0;

  unsigned segNo = 0;
  for (auto segment = group.mbegin(); segment != group.mend(); segment++)
  {
    if (segment->size() == 0)
      continue;
    segNo++;

    vector<unsigned> boardsIn;
    for (auto &bp: * segment)
      boardsIn.push_back(bp.extNo);
    
    vector<unsigned> boardsMissAll, boardsMissing;
    array<Board *, MAXNOOFTABLES> bpMissing;
    vector<string> infoMissing;
    if (files.boardsHaveResults(infoNo, fname, boardsIn, boardsMissAll))
      continue;
    
    for (unsigned extNo: boardsMissAll)
    {
      if (segment->getBoard(extNo) == nullptr)
        THROW("Bad board");
      Board * bd = segment->acquireBoard(extNo);

      string tmp = bd->strDeal(BRIDGE_WEST,BRIDGE_FORMAT_PBN);
      const size_t l = tmp.length();
      if (l < 40)
        THROW("Not PBN: " + tmp);

      // Skip "Deal [" and the trailing stuff.
      copy(tmp.begin()+7, tmp.end()-3, 
        tablePBN.deals[tablePBN.noOfTables].cards);
      tablePBN.deals[tablePBN.noOfTables].cards[l-10] = '\0';

      boardsMissing.push_back(extNo);
      bpMissing[static_cast<unsigned>(tablePBN.noOfTables)] = bd;

      tablePBN.noOfTables++;

      if (tablePBN.noOfTables == MAXNOOFTABLES)
      {
        makeTableau(&tablePBN, bpMissing, infoMissing);
        files.addDDInfo(infoNo, fname, boardsMissing, infoMissing);
        tablePBN.noOfTables = 0;
        boardsMissing.clear();
        infoMissing.clear();
      }
    }

    if (tablePBN.noOfTables > 0)
    {
      // Stragglers.
      makeTableau(&tablePBN, bpMissing, infoMissing);
      files.addDDInfo(infoNo, fname, boardsMissing, infoMissing);
    }
  }
}


void dispatchDD(
  const DDInfoType infoNo,
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog)
{
  try
  {
    makeDD(infoNo, group, files, fname);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

