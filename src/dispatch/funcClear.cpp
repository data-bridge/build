/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <array>
#include <sstream>
#include <fstream>

#include "../files/Files.h"
#include "../records/Group.h"
#include "../records/SDPoints.h"
#include "../analysis/dll.h"
#include "../util/parse.h"
#include "../handling/Bexcept.h"


using namespace std;


void makeInstanceList(
  Segment& segment,
  vector<string>& instancesIn)
{
  unsigned firstBno = segment.firstBoardNumber();
  unsigned lastBno = segment.lastRealBoardNumber();
  if (firstBno == numeric_limits<unsigned>::max() || 
      lastBno == numeric_limits<unsigned>::max())
    THROW("Bad segment board range: " + to_string(firstBno) +
      " to " + to_string(lastBno));

  for (unsigned bno = firstBno; bno <= lastBno; bno++)
  {
    if (segment.getBoard(bno) == nullptr)
      continue;

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


void makeClear(
  Group& group,
  Files& files,
  const string& fname)
{
  // It is a bug that multiple segments in an input file which use
  // overlapping board numbers will fail.  This doesn't happen in
  // the input files I use, so I haven't fixed DDInfo.

  unsigned segNo = 0;
  for (auto segment = group.mbegin(); segment != group.mend(); segment++)
  {
    if (segment->size() == 0)
      continue;
    segNo++;

    vector<string> instancesIn;
    makeInstanceList(* segment, instancesIn);

    Segment * segptr = &*segment;
    for (string extStr: instancesIn)
    {
      Board * bd;
      Instance * inst;
      str2BoardInstance(extStr, segptr, &bd, &inst);

      if (inst->isPassedOut() || ! inst->contractIsSet())
        continue;

      vector<SDPoints> points;
      inst->getSingleDummyPoints(points);

      files.addSingleDummyPoints(fname, extStr, points);
    }
  }
}


void dispatchClear(
  Group& group,
  Files& files,
  const string& fname,
  ostream& flog)
{
  try
  {
    makeClear(group, files, fname);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

