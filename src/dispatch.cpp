/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iomanip>
#include <sstream>
#include <fstream>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "dispatch.h"

#include "funcDigest.h"
#include "funcIMPSheet.h"
#include "funcRead.h"
#include "funcRefStats.h"
#include "funcTextStats.h"
#include "funcValidate.h"
#include "funcWrite.h"

#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


using namespace std;


static void checkPlayerCompletion(
  Group& group,
  ostream& flog);

static bool readFormattedFile(
  const string& fname,
  const Format format,
  Group& group,
  const Options& options,
  RefLines& refLines,
  ostream& flog);


void setTables()
{
  setReadTables();
  setWriteTables();
  setValidateTables();
}


static bool dispatchRead(
  const FileTask& task,
  Group& group,
  const Options& options,
  RefLines& refLines,
  ostream& flog)
{
  try
  {
    bool b = readFormattedFile(task.fileInput,
      task.formatInput, group, options, refLines, flog);
    if (refLines.skip())
      return true;

    return b;
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    return false;
  }
}


static void dispatchCompare(
  const Options& options,
  const string& fname,
  const Format format,
  Group& group,
  const string& text,
  CompStats& cstats,
  ostream& flog)
{
  try
  {
    Group groupNew;
    if (group.isCOCO())
      groupNew.setCOCO();

    Buffer buffer;
    buffer.split(text, format);

    dispatchReadBuffer(format, options, buffer, groupNew, flog);

    group == groupNew;
    cstats.add(true, format);
  }
  catch (Bdiff& bdiff)
  {
    cout << "Difference: " << fname << ", format " <<
      FORMAT_NAMES[format] << "\n";

    bdiff.print(flog);
    cstats.add(false, format);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    cstats.add(false, format);
  }
}


void dispatch(
  const int thrNo,
  Files& files,
  const Options& options,
  AllStats& allStats)
{
  ofstream freal;
  if (options.fileLog.setFlag)
    freal.open(options.fileLog.name + (thrNo == 0 ? "" : STR(thrNo)));
  ostream& flog = (options.fileLog.setFlag ? freal : cout);

  FileTask task;
  string text;
  text.reserve(100000);

  RefLines refLines;

  while (files.next(task))
  {
    if (options.verboseIO)
      flog << "Input " << task.fileInput << endl;

    Group group;

    // TODO: Better dispatch description language.
    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
      goto DIGEST;

    refLines.reset();
    allStats.timers.start(BRIDGE_TIMER_READ, task.formatInput);
    bool b = dispatchRead(task, group, options, refLines, flog);
    allStats.timers.stop(BRIDGE_TIMER_READ, task.formatInput);
    if (! b)
    {
      flog << "Failed to read " << task.fileInput << endl;
      continue;
    }

    if (options.playersFlag)
      checkPlayerCompletion(group, flog);

    if (options.quoteFlag)
    {
      if (options.verboseIO)
        flog << "Ref file for " << task.fileInput << endl;
    
      allStats.timers.start(BRIDGE_TIMER_REF_STATS, task.formatInput);
      dispatchRefStats(refLines, allStats.refstats, flog);
      allStats.timers.stop(BRIDGE_TIMER_REF_STATS, task.formatInput);
    }

    if (refLines.skip())
      continue;

    if (options.statsFlag)
    {
      if (options.verboseIO)
        flog << "Input " << task.fileInput << endl;
    
      allStats.timers.start(BRIDGE_TIMER_STATS, task.formatInput);
      dispatchTextStats(task, group, allStats.tstats, flog);
      allStats.timers.stop(BRIDGE_TIMER_STATS, task.formatInput);
    }

    for (auto &t: task.taskList)
    {
      if (options.verboseIO && t.fileOutput != "")
        flog << "Output " << t.fileOutput << endl;

      allStats.timers.start(BRIDGE_TIMER_WRITE, t.formatOutput);
      dispatchWrite(t.fileOutput, t.formatOutput, refLines.order(), 
        group, text, flog);
      allStats.timers.stop(BRIDGE_TIMER_WRITE, t.formatOutput);

      if (t.refFlag && refLines.validate())
      {
        if (options.verboseIO)
          flog << "Validating " << t.fileOutput <<
              " against " << t.fileRef << endl;

        allStats.timers.start(BRIDGE_TIMER_VALIDATE, t.formatOutput);
        dispatchValidate(task, t, options, text, allStats.vstats, flog);
        allStats.timers.stop(BRIDGE_TIMER_VALIDATE, t.formatOutput);
      }

      if (options.compareFlag && task.formatInput == t.formatOutput)
      {
        if (options.verboseIO)
          flog << "Comparing " << t.fileOutput <<
              " against " << task.fileInput << endl;

        allStats.timers.start(BRIDGE_TIMER_COMPARE, t.formatOutput);
        dispatchCompare(options, task.fileInput, task.formatInput, 
          group, text, allStats.cstats, flog);
        allStats.timers.stop(BRIDGE_TIMER_COMPARE, t.formatOutput);
      }

      if (task.removeOutputFlag)
        remove(t.fileOutput.c_str());
    }

    DIGEST:
    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
    {
      if (options.verboseIO)
        flog << "Digest input " << task.fileInput << endl;
    
      allStats.timers.start(BRIDGE_TIMER_DIGEST, task.formatInput);
      dispatchDigest(task, options, flog);
      allStats.timers.stop(BRIDGE_TIMER_DIGEST, task.formatInput);
    }

    // TODO: Use an option to control
    // dispatchIMPSheet(group, flog);
  }
}


static void checkPlayerCompletion(
  Group& group,
  ostream& flog)
{
  unsigned numSegs = 0;
  unsigned numCombos = 0;
  bool overlaps = false;
  bool nonFull = false;
  vector<unsigned> completions(65);
  completions.clear();

  for (auto &segment: group)
  {
    numSegs++;
    for (auto &bpair: segment)
    {
      Board& board = bpair.board;
      unsigned c = 0;
      for (unsigned i = 0; i < board.countAll(); i++)
      {
        board.setInstance(i);
        c *= 8;
        if (! board.skipped())
          c += board.missingPlayers();
      }
      if (c > 65)
        THROW("Bad combo: " + STR(c));
      if (c > 0)
        nonFull = true;
      if (completions[c] == 0)
        numCombos++;

      completions[c]++;

      if (board.overlappingPlayers())
        overlaps = true;
    }
  }

  if (numSegs > 1)
  {
    flog << "Input: " << group.name() << "\n";
    flog << "Number of segments: " << numSegs << "\n\n";
  }

  if (overlaps || nonFull)
  {
    if (overlaps)
      flog << "File " << group.name() << ": Overlaps exist\n\n";
    if (nonFull)
      flog << "File " << group.name() << ": Incomplete players exist\n\n";
    for (auto &segment: group)
    {
      flog << segment.strTitle(BRIDGE_FORMAT_LIN_VG);
      flog << segment.strPlayers(BRIDGE_FORMAT_LIN_VG) << "\n";
    }
  }

  if (numCombos <= 1)
    return;

  flog << "Input: " << group.name() << "\n";

  for (auto &segment: group)
  {
    flog << segment.strTitle(BRIDGE_FORMAT_LIN_VG);
    flog << segment.strPlayers(BRIDGE_FORMAT_LIN_VG);
  }

  for (unsigned i = 0; i < 65; i++)
  {
    if (completions[i] == 0)
      continue;
    flog << i/8 << ", " << (i % 8) << ": " << completions[i] << "\n";
  }
  flog << "\n";
}


static bool readFormattedFile(
  const string& fname,
  const Format format,
  Group& group,
  const Options& options,
  RefLines& refLines,
  ostream& flog)
{
  Buffer buffer;
  buffer.read(fname, format, refLines);
  if (refLines.skip() && options.quoteFlag)
  {
    // Ugh.  Poor man's counter of hands and boards.
    buffer.reset();
    buffer.readForce(fname,format);

    unsigned numLines, numHands, numBoards;
    numLines = buffer.lengthOrig();
    vector<string> lines;
    for (unsigned i = 1; i <= numLines; i++)
      lines.push_back(buffer.getLine(i));

    RefLine rl;
    rl.countHands(lines, FORMAT_INPUT_MAP[format], numHands, numBoards);
    refLines.setFileData(numLines, numHands, numBoards);
    refLines.checkHeader();
    return true;
  }

  if (refLines.skip())
    return true;

  group.setName(fname);
  if (refLines.orderCOCO())
    group.setCOCO();

  bool b = dispatchReadBuffer(format, options, buffer, group, flog);

  refLines.setFileData(buffer.lengthOrig(), 
    group.count(), group.countBoards());
  if (options.quoteFlag)
    refLines.checkHeader();

  return b;
}

