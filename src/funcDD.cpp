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
  vector<string>& infoMissing)
{
  ddTablesRes resDDS;
  tableauDD(tablePBN, &resDDS);

  for (int i = 0; i < tablePBN->noOfTables; i++)
  {
    const string s = DDS2RBN(resDDS.results[i].resTable);
    infoMissing.push_back(s.substr(4, s.length()-6));
  }

  tablePBN->noOfTables = 0;
}


void makeDD(
  const DDInfoType infoNo,
  const Group& group,
  Files& files,
  const string& fname)
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
    
    vector<unsigned> boardsMissAll, boardsMissing;
    vector<string> infoMissing;
    if (files.boardsHaveResults(infoNo, fname, boardsIn, boardsMissAll))
      continue;
    
    for (unsigned extNo: boardsMissAll)
    {
      const Board * bd = segment.getBoard(extNo);
      if (bd == nullptr)
        THROW("Bad board");

      string tmp = bd->strDeal(BRIDGE_WEST,BRIDGE_FORMAT_PBN);
      const size_t l = tmp.length();
      if (l < 40)
        THROW("Not PBN: " + tmp);

      // Skip "Deal [" and the trailing stuff.
      copy(tmp.begin()+7, tmp.end()-3, 
        tablePBN.deals[tablePBN.noOfTables].cards);
      tablePBN.deals[tablePBN.noOfTables].cards[l-10] = '\0';
      tablePBN.noOfTables++;

      boardsMissing.push_back(extNo);

      if (tablePBN.noOfTables == MAXNOOFTABLES)
      {
        makeTableau(&tablePBN, infoMissing);
        files.addDDInfo(infoNo, fname, boardsMissing, infoMissing);
        tablePBN.noOfTables = 0;
        boardsMissing.clear();
        infoMissing.clear();
      }
    }

    if (tablePBN.noOfTables > 0)
    {
      // Stragglers.
      makeTableau(&tablePBN, infoMissing);
      files.addDDInfo(infoNo, fname, boardsMissing, infoMissing);
    }
  }
}


void dispatchDD(
  const DDInfoType infoNo,
  const Group& group,
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

