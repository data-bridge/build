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

#include "../Files.h"
#include "../parse.h"

#include "../records/Group.h"

#include "../dll.h"

#include "../handling/Bexcept.h"


using namespace std;


void makeTraceDDS(
  boardsPBN * bopPBN,
  playTracesPBN * plpPBN,
  array<Instance *, MAXNOOFBOARDS> instpMissing,
  vector<string>& infoMissing)
{
  solvedPlays resDDS;
  traceDD(bopPBN, plpPBN, &resDDS);

  for (int i = 0; i < bopPBN->noOfBoards; i++)
  {
    const size_t u = static_cast<unsigned>(i);
    instpMissing[u]->setTrace(resDDS.solved[i].number, 
      resDDS.solved[i].tricks);

    infoMissing.push_back(instpMissing[u]->strTraceCompact());
  }

  bopPBN->noOfBoards = 0;
  plpPBN->noOfBoards = 0;
}


void makeInstanceList(
  Segment& segment,
  vector<string>& instancesIn)
{
  unsigned firstBno = segment.firstBoardNumber();
  unsigned lastBno = segment.lastRealBoardNumber();
  if (firstBno == BIGNUM || lastBno == BIGNUM)
    THROW("Bad segment board range: " + to_string(firstBno) +
      " to " + to_string(lastBno));

  for (unsigned bno = firstBno; bno <= lastBno; bno++)
  {
    if (segment.getBoard(bno) == nullptr)
      THROW("Bad board");
    Board * bd = segment.acquireBoard(bno);

    for (unsigned i = 0; i < bd->countAll(); i++)
    {
      if (bd->skipped(i))
        continue;

      const Instance& inst = bd->getInstance(i);
      const string st = inst.strRoom(bno, BRIDGE_FORMAT_LIN_RP);
      if (st.size() < 5)
        THROW("Bad room string: " + st);
      instancesIn.push_back(st.substr(3, st.size()-4));
    }
  }
}


void str2BoardInstance(
  const string& st, 
  Segment * segment,
  Board ** bd, 
  Instance ** inst)
{
  if (st.length() < 2)
    THROW("Bad board: " + st);

  const string room = st.substr(0, 1);
  if (room != "o" && room != "c")
    THROW("Not a room: " + room);

  const string rest = st.substr(1);
  unsigned extNo;
  if (! str2unsigned(rest, extNo))
    THROW("Not a number: " + rest);

  if (segment->getBoard(extNo) == nullptr)
    THROW("Bad board");
  * bd = segment->acquireBoard(extNo);
  * inst = (* bd)->acquireInstance(room == "o" ? 0u : 1u);
}


void makeTrace(
  Group& group,
  Files& files,
  const string& fname)
{
  // It is a bug that multiple segments in an input file which use
  // overlapping board numbers will fail.  This doesn't happen in
  // the input files I use, so I haven't fixed DDInfo.

  boardsPBN bPBN;
  playTracesPBN pPBN;
  bPBN.noOfBoards = 0;
  pPBN.noOfBoards = 0;

  unsigned segNo = 0;
  for (auto segment = group.mbegin(); segment != group.mend(); segment++)
  {
    if (segment->size() == 0)
      continue;
    segNo++;

    vector<string> instancesIn;
    makeInstanceList(* segment, instancesIn);

    vector<string> instMissAll, instMissing, infoMissing;
    CaseResults infoSeen;
    array<Instance *, MAXNOOFBOARDS> instpMissing;
    files.haveResults(BRIDGE_DD_INFO_TRACE, fname, 
        instancesIn, infoSeen, instMissAll);
    
    Segment * segptr = &*segment;
    for (auto& itSeen: infoSeen)
    {
      // Fill out the instance traces from file memory.
      Board * bd;
      Instance * inst;
      str2BoardInstance(itSeen.first, segptr, &bd, &inst);
      inst->setTrace(itSeen.second);
    }

    for (string extStr: instMissAll)
    {
      Board * bd;
      Instance * inst;
      str2BoardInstance(extStr, segptr, &bd, &inst);

      if (inst->isPassedOut() || ! inst->contractIsSet())
        continue;

      string tmp = bd->strDeal(BRIDGE_WEST, BRIDGE_FORMAT_PBN);
      const size_t l = tmp.length();
      if (l < 40)
        THROW("Not PBN: " + tmp);

      // Skip "Deal [" and the trailing stuff.
      const unsigned u = static_cast<unsigned>(bPBN.noOfBoards);
      copy(tmp.begin()+7, tmp.end()-3, bPBN.deals[u].remainCards);
      bPBN.deals[u].remainCards[l-10] = '\0';

      bPBN.deals[u].trump = inst->getDenom();
      bPBN.deals[u].first = inst->getLeader();
      bPBN.deals[u].currentTrickRank[0] = 0;
      bPBN.deals[u].currentTrickRank[1] = 0;
      bPBN.deals[u].currentTrickRank[2] = 0;

      const string stPlay = inst->strPlay(BRIDGE_FORMAT_PAR);
      pPBN.plays[u].number = static_cast<int>(stPlay.length()) >> 1;
      copy(stPlay.begin(), stPlay.end(), pPBN.plays[u].cards);
      pPBN.plays[u].cards[stPlay.length()] = '\0';

      bPBN.noOfBoards++;
      pPBN.noOfBoards++;

      instMissing.push_back(extStr);
      instpMissing[u] = inst;

      if (bPBN.noOfBoards == MAXNOOFBOARDS)
      {
        makeTraceDDS(&bPBN, &pPBN, instpMissing, infoMissing);
        files.addDDInfo(BRIDGE_DD_INFO_TRACE, fname, 
          instMissing, infoMissing);
        bPBN.noOfBoards = 0;
        pPBN.noOfBoards = 0;
        instMissing.clear();
        infoMissing.clear();
      }
    }

    if (bPBN.noOfBoards > 0)
    {
      // Stragglers.
      makeTraceDDS(&bPBN, &pPBN, instpMissing, infoMissing);
      files.addDDInfo(BRIDGE_DD_INFO_TRACE, fname, 
        instMissing, infoMissing);
    }
  }
}


void dispatchTrace(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog)
{
  try
  {
    makeTrace(group, files, fname);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

