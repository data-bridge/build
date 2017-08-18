/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "Group.h"
#include "Files.h"

#include "dll.h"
#include "Bexcept.h"


using namespace std;


string DDS2RBN(const int res[5][4])
{
  Tableau tableau;
  tableau.setDDS(res);
  return tableau.str(BRIDGE_FORMAT_RBN);
}


void makeTableau(
  ddTableDealsPBN * tablePBN,
  const unsigned numThreads,
  vector<string>& infoMissing)
{
  ddTablesRes resDDS;
  tableauDD(tablePBN, &resDDS, numThreads);

  for (int i = 0; i < tablePBN->noOfTables; i++)
    infoMissing.push_back(DDS2RBN(resDDS.results[i].resTable));

  tablePBN->noOfTables = 0;
}


void makeDD(
  const DDInfoType infoNo,
  const Group& group,
  Files& files,
  const string& fname,
  const Options& options)
{
  ddTableDealsPBN tablePBN;
  if (infoNo == BRIDGE_DD_INFO_TRACE)
    THROW("Trace not yet implemented");
  tablePBN.noOfTables = 0;

  unsigned segNo = 0;
  for (auto &segment: group)
  {
    if (segment.size() == 0)
      continue;
    segNo++;

    vector<unsigned> boardsIn;
    for (auto &bp: segment)
      boardsIn.push_back(bp.extNo);
    
    vector<unsigned> boardsMissing;
    vector<string> infoMissing;
    if (files.boardsHaveResults(infoNo, fname, boardsIn, boardsMissing))
      continue;
    
    for (unsigned extNo: boardsMissing)
    {
      const Board * bd = segment.getBoard(extNo);
      if (bd == nullptr)
        THROW("Bad board");

      string tmp = bd->strDeal(BRIDGE_WEST,BRIDGE_FORMAT_PBN);
      tmp.copy(tablePBN.deals[tablePBN.noOfTables++].cards, 
        tmp.length()+1);

      if (tablePBN.noOfTables == MAXNOOFTABLES)
      {
        makeTableau(&tablePBN, options.numThreads, infoMissing);
        files.addDDInfo(infoNo, fname, boardsMissing, infoMissing);
        infoMissing.clear();
      }
    }

    if (tablePBN.noOfTables > 0)
    {
      // Stragglers.
      makeTableau(&tablePBN, options.numThreads, infoMissing);
      files.addDDInfo(infoNo, fname, boardsMissing, infoMissing);
    }
  }
}


void dispatchDD(
  const DDInfoType infoNo,
  const Group& group,
  Files& files,
  const string& fname,
  const Options& options,
  ostream& flog)
{
  try
  {
    makeDD(infoNo, group, files, fname, options);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

