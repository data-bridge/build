/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>
#include <assert.h>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "Chunk.h"
#include "dispatch.h"
#include "ddsIF.h"
#include "validate.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileTXT.h"
#include "fileEML.h"
#include "fileREC.h"

#include "funcDigest.h"
#include "funcIMPSheet.h"
#include "funcTextStats.h"
#include "funcWrite.h"

#include "AllStats.h"

#include "parse.h"
#include "Sheet.h"
#include "Bexcept.h"
#include "Bdiff.h"


using namespace std;

struct FormatFunctions
{
  void (* readChunk)(Buffer&, Chunk&, bool&);
};

static FormatFunctions formatFncs[BRIDGE_FORMAT_SIZE];

typedef void (Segment::*SegPtr)(const string& s, const Format format);
typedef void (Board::*BoardPtr)(const string& s, const Format format);

static SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
static BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];


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

static bool readFormattedFile(
  Buffer& buffer,
  const Format format,
  Group& group,
  const Options& options,
  ostream& flog);


static void setFormatTables()
{
  setLINTables();
  setPBNTables();
  setRBNTables();
  setTXTTables();
  setEMLTables();
}


static void setIO()
{
  setWriteTables();

  formatFncs[BRIDGE_FORMAT_LIN].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_RP].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_VG].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_TRN].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_PBN].readChunk = &readPBNChunk;
  formatFncs[BRIDGE_FORMAT_RBN].readChunk = &readRBNChunk;
  formatFncs[BRIDGE_FORMAT_RBX].readChunk = &readRBNChunk; // !
  formatFncs[BRIDGE_FORMAT_TXT].readChunk = &readTXTChunk;
  formatFncs[BRIDGE_FORMAT_EML].readChunk = &readEMLChunk;
  formatFncs[BRIDGE_FORMAT_REC].readChunk = &readRECChunk;
}


static void setInterface()
{
  segPtr[BRIDGE_FORMAT_TITLE] = &Segment::setTitle;
  segPtr[BRIDGE_FORMAT_DATE] = &Segment::setDate;
  segPtr[BRIDGE_FORMAT_LOCATION] = &Segment::setLocation;
  segPtr[BRIDGE_FORMAT_EVENT] = &Segment::setEvent;
  segPtr[BRIDGE_FORMAT_SESSION] = &Segment::setSession;
  segPtr[BRIDGE_FORMAT_SCORING] = &Segment::setScoring;
  segPtr[BRIDGE_FORMAT_TEAMS] = &Segment::setTeams;
  segPtr[BRIDGE_FORMAT_HOMETEAM] = &Segment::setFirstTeam;
  segPtr[BRIDGE_FORMAT_VISITTEAM] = &Segment::setSecondTeam;

  segPtr[BRIDGE_FORMAT_RESULTS_LIST] = &Segment::setResultsList;
  segPtr[BRIDGE_FORMAT_PLAYERS_LIST] = &Segment::setPlayersList;
  segPtr[BRIDGE_FORMAT_PLAYERS_HEADER] = &Segment::setPlayersHeader;
  segPtr[BRIDGE_FORMAT_WEST] = &Segment::setWest;
  segPtr[BRIDGE_FORMAT_NORTH] = &Segment::setNorth;
  segPtr[BRIDGE_FORMAT_EAST] = &Segment::setEast;
  segPtr[BRIDGE_FORMAT_SOUTH] = &Segment::setSouth;
  segPtr[BRIDGE_FORMAT_SCORES_LIST] = &Segment::setScoresList;
  segPtr[BRIDGE_FORMAT_BOARDS_LIST] = &Segment::setBoardsList;

  segPtr[BRIDGE_FORMAT_BOARD_NO] = &Segment::setNumber;
  segPtr[BRIDGE_FORMAT_PLAYERS_BOARD] = &Segment::setPlayers;
  segPtr[BRIDGE_FORMAT_ROOM] = &Segment::setRoom;

  boardPtr[BRIDGE_FORMAT_DEAL] = &Board::setDeal;
  boardPtr[BRIDGE_FORMAT_DEALER] = &Board::setDealer;
  boardPtr[BRIDGE_FORMAT_VULNERABLE] = &Board::setVul;
  boardPtr[BRIDGE_FORMAT_AUCTION] = &Board::setAuction;
  boardPtr[BRIDGE_FORMAT_DECLARER] = &Board::setDeclarer;
  boardPtr[BRIDGE_FORMAT_CONTRACT] = &Board::setContract;
  boardPtr[BRIDGE_FORMAT_PLAY] = &Board::setPlays;
  boardPtr[BRIDGE_FORMAT_RESULT] = &Board::setResult;
  boardPtr[BRIDGE_FORMAT_SCORE] = &Board::setScore;
  boardPtr[BRIDGE_FORMAT_SCORE_IMP] = &Board::setScoreIMP;
  boardPtr[BRIDGE_FORMAT_SCORE_MP] = &Board::setScoreMP;
  boardPtr[BRIDGE_FORMAT_DOUBLE_DUMMY] = &Board::setTableau;
}


void setTables()
{
  setFormatTables();
  setIO();
  setInterface();
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

    if (options.playersFlag && b)
      checkPlayerCompletion(group, flog);

    return b;
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    return false;
  }
}


static void dispatchRefStats(
  const RefLines& refLines,
  RefStats& refstats,
  ostream& flog)
{
  try
  {
    CommentType cat;
    RefEntry re;

    if (refLines.getHeaderEntry(cat, re))
      refstats.logFile(re);
    else
      THROW("No header in refLines");

    if (! refLines.hasComments())
      return;

    refstats.logRefFile();

    if (refLines.skip())
    {
      refstats.logSkip(cat, re);
      return;
    }
    else if (refLines.orderCOCO())
      refstats.logOrder(ERR_LIN_QX_ORDER_COCO, re);
    else if (refLines.orderOOCC())
      refstats.logOrder(ERR_LIN_QX_ORDER_OOCC, re);
    else if (! refLines.validate())
      refstats.logNoval(cat, re);

    for (auto &rl: refLines)
    {
      rl.getEntry(cat, re);
      refstats.logRef(cat, re);
    }
    
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}


static void dispatchValidate(
  const FileTask& task,
  const FileOutputTask& otask,
  const Options& options,
  const string& text,
  ValStats& vstats,
  ostream& flog)
{
  try
  {
    validate(text, otask.fileOutput, otask.fileRef,
      task.formatInput, otask.formatOutput, options, vstats);
  }
  catch (Bexcept& bex)
  {
    flog << "Files " << task.fileInput << " -> " <<
      otask.fileOutput << endl;
    bex.print(flog);
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

    readFormattedFile(buffer, format, groupNew, options, flog);

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


static void printCounts(
  const string& fname,
  const Chunk& chunk,
  const Counts& counts)
{
  cout << "Input file:   " << fname << endl;
  cout << "Segment:      " << counts.segno << endl;
  cout << "Board:        " << counts.bno << endl;
  cout << chunk.strRange();
}


static void str2board(
  const Chunk& chunk,
  const Format format,
  Counts& counts)
{
  // TODO: Could go into chunk
  const string bno = chunk.get(BRIDGE_FORMAT_BOARD_NO);

  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    if (bno == "")
    {
      counts.curr.no = 0;
      return;
    }

    const string st = bno.substr(1);
    if (! str2upos(st, counts.curr.no))
      THROW("Not a board number");
    if (bno.at(0) == 'o')
      counts.curr.roomFlag = true;
    else if (bno.at(0) == 'c')
      counts.curr.roomFlag = false;
    else
      THROW("Not a room");
  }
  else if (format == BRIDGE_FORMAT_PBN)
  {
    if (bno != "")
    {
      // Otherwise reuse the value in counts.curr.no
      if (! str2upos(bno, counts.curr.no))
        THROW("Not a board number");
    }

    const string r = chunk.get(BRIDGE_FORMAT_ROOM);
    if (r == "" || r == "Open")
      counts.curr.roomFlag = true;
    else if (r == "Closed")
      counts.curr.roomFlag = false;
    else
      THROW("Unknown room: " + r);
  }
  else if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
  {
    if (bno != "")
    {
      // Otherwise reuse the value in counts.curr.no
      if (! str2upos(bno, counts.curr.no))
        THROW("Not a board number");
    }
      
    const string sn = chunk.get(BRIDGE_FORMAT_PLAYERS_BOARD);
    const unsigned sl = sn.length();
    if (sn != "" && sl >= 2)
    {
      if (sn.substr(sl-2) == ":O")
        counts.curr.roomFlag = true;
      else if (sn.substr(sl-2) == ":C")
        counts.curr.roomFlag = false;
      else
        counts.curr.roomFlag = ! counts.curr.roomFlag;
    }
    else
      counts.curr.roomFlag = ! counts.curr.roomFlag;
  }
  else if (format == BRIDGE_FORMAT_TXT ||
      format == BRIDGE_FORMAT_EML ||
      format == BRIDGE_FORMAT_REC)
  {
    if (bno != "")
    {
      // Otherwise reuse the value in counts.curr.no
      if (! str2upos(bno, counts.curr.no))
        THROW("Not a board number");
    }

    counts.curr.roomFlag = ! counts.curr.roomFlag;
  }
  else
  {
    if (! str2upos(bno, counts.curr.no))
      THROW("Not a board number: " + bno);
  }
}


static bool storeChunk(
  Group& group,
  Segment * segment,
  Board * board,
  Chunk& chunk,
  const Counts& counts,
  const Format format,
  const Options& options,
  ostream& flog)
{
    if (chunk.isEmpty(BRIDGE_FORMAT_AUCTION) ||
        (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN &&
         chunk.isEmpty(BRIDGE_FORMAT_VULNERABLE)))
    {
      // Guess dealer and vul from the board number.
      if (chunk.isEmpty(BRIDGE_FORMAT_BOARD_NO))
      {
        chunk.guessDealerAndVul(counts.curr.no, format);
      }
      else if (format == BRIDGE_FORMAT_LIN_VG && 
          board->hasDealerVul())
      {
        chunk.set(BRIDGE_FORMAT_VULNERABLE, 
          board->strVul(BRIDGE_FORMAT_PAR));
      }
      else if (FORMAT_INPUT_MAP[format] != BRIDGE_FORMAT_LIN ||
          chunk.isEmpty(BRIDGE_FORMAT_VULNERABLE))
      {
        chunk.guessDealerAndVul(format);
      }
    }

  unsigned i;
  try
  {
    for (i = 0; i <= BRIDGE_FORMAT_ROOM; i++)
    {
      const string text = chunk.get(i);
      if (text != "")
        (segment->*segPtr[i])(text, format);
    }

    for (i = BRIDGE_FORMAT_DEAL; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      const string text = chunk.get(i);
      if (text != "")
        (board->*boardPtr[i])(text, format);
    }
  }
  catch (Bexcept& bex)
  {
    if (options.verboseThrow)
    {
      printCounts(group.name(), chunk, counts);
      cout << chunk.str(static_cast<Label>(i));
    }

    bex.print(flog);

    cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
    cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;

    if (options.verboseBatch)
      cout << chunk.str();
    return false;
  }

  if (! board->skipped() && ! board->auctionIsOver())
  {
    if (board->lengthAuction() > 0)
    {
      printCounts(group.name(), chunk, counts);
      cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
      cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;
      cout << board->strAuction(BRIDGE_FORMAT_TXT) << endl;
      cout << board->strPlay(BRIDGE_FORMAT_TXT) << endl;

      cout << "Error: Auction incomplete\n";
      return false;
    }
    else
    {
      // If we skip here, we get a validation "error".
      // board->markInstanceSkip();
    }
  }

  board->spreadBasics();
  return true;
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

  bool b = readFormattedFile(buffer, format, group, options, flog);

  refLines.setFileData(buffer.lengthOrig(), 
    group.count(), group.countBoards());
  if (options.quoteFlag)
    refLines.checkHeader();

  return b;
}


static bool readFormattedFile(
  Buffer& buffer,
  const Format format,
  Group& group,
  const Options& options,
  ostream& flog)
{
  group.setFormat(format);

  Chunk chunk, prevChunk;
  Segment * segment = nullptr;
  bool newSegFlag = false;

  Board * board = nullptr;
  boardIDLIN lastBoard = {0, false};

  Counts counts;
  counts.segno = 0;
  counts.chunkno = 0;
  counts.bno = 0;

  while (true)
  {
    try
    {
      if (format == BRIDGE_FORMAT_PBN)
        prevChunk.copyFrom(chunk, CHUNK_HEADER);

      newSegFlag = false;
      chunk.reset();

      (* formatFncs[format].readChunk)(buffer, chunk, newSegFlag);

      if (chunk.isEmpty(BRIDGE_FORMAT_BOARD_NO) &&
          chunk.isEmpty(BRIDGE_FORMAT_RESULT) &&
          chunk.isEmpty(BRIDGE_FORMAT_AUCTION))
        break;
    }
    catch (Bexcept& bex)
    {
      if (options.verboseThrow)
        printCounts(group.name(), chunk, counts);

      bex.print(flog);

      if (options.verboseBatch)
        cout << chunk.str();
      return false;
    }

    if (format == BRIDGE_FORMAT_RBN || 
        format == BRIDGE_FORMAT_RBX ||
        format == BRIDGE_FORMAT_TXT)
    {
      // TODO: This is not necessary to get the deal as such,
      // but otherwise Board::setDeal() does not get called,
      // and the deal info does not get into play and auction.
      // Should probably be detected in Board?
      if (chunk.isEmpty(BRIDGE_FORMAT_DEAL))
        chunk.copyFrom(prevChunk, CHUNK_DEAL);
      else
        prevChunk.copyFrom(chunk, CHUNK_DEAL);

      if (format == BRIDGE_FORMAT_TXT)
      {
        // TODO: Ditto.
        if (chunk.isEmpty(BRIDGE_FORMAT_VULNERABLE))
          chunk.set(BRIDGE_FORMAT_VULNERABLE, 
            prevChunk.get(BRIDGE_FORMAT_VULNERABLE));
        else
          prevChunk.set(BRIDGE_FORMAT_VULNERABLE, 
            chunk.get(BRIDGE_FORMAT_VULNERABLE));

        if (chunk.isEmpty(BRIDGE_FORMAT_DEALER))
          chunk.set(BRIDGE_FORMAT_DEALER, 
            prevChunk.get(BRIDGE_FORMAT_DEALER));
        else
          prevChunk.set(BRIDGE_FORMAT_DEALER, 
            chunk.get(BRIDGE_FORMAT_DEALER));
      }
    }

    counts.chunkno++;

    if (newSegFlag && format == BRIDGE_FORMAT_PBN)
    {
      // May not really be a new segment.
      newSegFlag = chunk.differsFrom(prevChunk, CHUNK_HEADER);
    }

    if (newSegFlag || segment == nullptr)
    {
      segment = group.make();
      counts.segno++;
      counts.bno = 0;
      if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
      {
        try
        {
          // Easier to get the key header information in first.
          for (unsigned i = BRIDGE_FORMAT_TITLE; 
              i <= BRIDGE_FORMAT_BOARDS_LIST; i++)
          {
            const string text = chunk.get(i);
            if (text != "")
            {
              (segment->*segPtr[i])(text, format);
              chunk.set(static_cast<Label>(i), "");
            }
          }
        }
        catch(Bexcept& bex)
        {
          if (options.verboseThrow)
            printCounts(group.name(), chunk, counts);

          bex.print(flog);

          if (options.verboseBatch)
            cout << chunk.str();
          return false;
        }
      }
      else if (format == BRIDGE_FORMAT_TXT ||
          format == BRIDGE_FORMAT_EML ||
          format == BRIDGE_FORMAT_REC)
      {
        // If COCO, then we start with open room here, so that the
        // first inversion is closed, and vice versa.
        counts.curr.roomFlag = segment->getCOCO();
      }
    }

    str2board(chunk, format, counts);

    if (counts.curr.no != 0)
    {
      if (counts.curr.no != lastBoard.no)
      {
        // New board.
        board = segment->acquireBoard(counts.curr.no);
        counts.bno++;
      }
    }

    lastBoard = counts.curr;

    board->acquireInstance(counts.curr.roomFlag ? 0u : 1u);
    board->unmarkInstanceSkip();

    if (! storeChunk(group, segment, board, chunk,
        counts, format, options, flog))
      return false;
  }

  return true;
}

