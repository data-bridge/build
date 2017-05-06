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
#include "heurfix.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileTXT.h"
#include "fileEML.h"
#include "fileREC.h"

#include "parse.h"
#include "Sheet.h"
#include "Bexcept.h"
#include "Bdiff.h"


using namespace std;

struct FormatFunctions
{
  void (* readChunk)(Buffer&, vector<unsigned>&, Chunk&, bool&);
  void (* writeSeg)(string&, Segment&, Format);
  void (* writeBoard)(string&, Segment&, Board&, 
    WriteInfo&, const Format);
};

FormatFunctions formatFncs[BRIDGE_FORMAT_SIZE];

typedef void (Segment::*SegPtr)(const string& s, const Format format);
typedef void (Board::*BoardPtr)(const string& s, const Format format);

SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];

struct Fix
{
  unsigned no;
  Label label;
  string value;
};


static void checkPlayerCompletion(
  Group& group,
  ostream& flog);

static bool readFormattedFile(
  const string& fname,
  const Format format,
  Group& group,
  const Options& options,
  RefControl& refControl,
  ostream& flog);

static bool readFormattedFile(
  Buffer& buffer,
  vector<Fix>& fix,
  const Format format,
  Group& group,
  const Options& options,
  ostream& flog);

static void logLengths(
  Group& group,
  TextStats& tstats,
  const string& fname,
  const Format format);

static void readFix(
  const string& fname,
  vector<Fix>& fix);

static void writeFormattedFile(
  Group& group,
  const string& fname,
  const RefControl refControl,
  string& text,
  const Format format);

static void writeFast(
  const string& fname,
  const string& text);


static void writeDummySegmentLevel(
  string& st,
  Segment& segment,
  const Format format)
{
  UNUSED(st);
  UNUSED(segment);
  UNUSED(format);
}


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
  formatFncs[BRIDGE_FORMAT_LIN].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN].writeSeg = &writeLINSegmentLevel;
  formatFncs[BRIDGE_FORMAT_LIN].writeBoard = &writeLINBoardLevel;

  formatFncs[BRIDGE_FORMAT_LIN_RP].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_RP].writeSeg = &writeLINSegmentLevel;
  formatFncs[BRIDGE_FORMAT_LIN_RP].writeBoard = &writeLINBoardLevel;

  formatFncs[BRIDGE_FORMAT_LIN_VG].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_VG].writeSeg = &writeLINSegmentLevel;
  formatFncs[BRIDGE_FORMAT_LIN_VG].writeBoard = &writeLINBoardLevel;

  formatFncs[BRIDGE_FORMAT_LIN_TRN].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_TRN].writeSeg = &writeLINSegmentLevel;
  formatFncs[BRIDGE_FORMAT_LIN_TRN].writeBoard = &writeLINBoardLevel;

  formatFncs[BRIDGE_FORMAT_PBN].readChunk = &readPBNChunk;
  formatFncs[BRIDGE_FORMAT_PBN].writeSeg = &writeDummySegmentLevel;
  formatFncs[BRIDGE_FORMAT_PBN].writeBoard = &writePBNBoardLevel;

  formatFncs[BRIDGE_FORMAT_RBN].readChunk = &readRBNChunk;
  formatFncs[BRIDGE_FORMAT_RBN].writeSeg = &writeRBNSegmentLevel;
  formatFncs[BRIDGE_FORMAT_RBN].writeBoard = &writeRBNBoardLevel;

  formatFncs[BRIDGE_FORMAT_RBX].readChunk = &readRBNChunk; // !
  formatFncs[BRIDGE_FORMAT_RBX].writeSeg = &writeRBNSegmentLevel;
  formatFncs[BRIDGE_FORMAT_RBX].writeBoard = &writeRBNBoardLevel;

  formatFncs[BRIDGE_FORMAT_TXT].readChunk = &readTXTChunk;
  formatFncs[BRIDGE_FORMAT_TXT].writeSeg = &writeTXTSegmentLevel;
  formatFncs[BRIDGE_FORMAT_TXT].writeBoard = &writeTXTBoardLevel;

  formatFncs[BRIDGE_FORMAT_EML].readChunk = &readEMLChunk;
  formatFncs[BRIDGE_FORMAT_EML].writeSeg = &writeDummySegmentLevel;
  formatFncs[BRIDGE_FORMAT_EML].writeBoard = &writeEMLBoardLevel;

  formatFncs[BRIDGE_FORMAT_REC].readChunk = &readRECChunk;
  formatFncs[BRIDGE_FORMAT_REC].writeSeg = &writeDummySegmentLevel;
  formatFncs[BRIDGE_FORMAT_REC].writeBoard = &writeRECBoardLevel;
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
  RefControl& refControl,
  ostream& flog)
{
  try
  {
    bool b = readFormattedFile(task.fileInput,
      task.formatInput, group, options, refControl, flog);
    if (refControl == ERR_REF_SKIP)
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


static void dispatchStats(
  const FileTask& task,
  Group& group,
  TextStats& tstats,
  ostream& flog)
{
  try
  {
    logLengths(group, tstats, task.fileInput, task.formatInput);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}


static void dispatchWrite(
  const FileOutputTask& otask,
  const RefControl refControl,
  Group& group,
  string& text,
  ostream& flog)
{
  try
  {
    text = "";
    writeFormattedFile(group, otask.fileOutput, refControl, 
      text, otask.formatOutput);
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
    Buffer buffer;
    vector<Fix> fix;

    buffer.split(text, format);
    fix.clear();
    readFix(fname, fix);

    readFormattedFile(buffer, fix, format, groupNew, options, flog);

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


static void dispatchDigest(
  const Options& options,
  const FileTask& task,
  ostream& flog)
{
  if (FORMAT_INPUT_MAP[task.formatInput] != BRIDGE_FORMAT_LIN)
    return;

  try
  {
    Sheet sheet;
    sheet.read(task.fileInput);
    const string st = sheet.str();
    if (st != "")
    {
      ofstream dlog;
      if (options.fileDigest.setFlag)
        dlog.open(options.fileDigest.name);
      else
      {
        const string base = basefile(task.fileInput);
	const string dout = options.dirDigest.name + "/" +
	  changeExt(base, ".sht");
        dlog.open(dout);
      }
      dlog << st;
      dlog.close();
    }
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    cout << "Came from " << task.fileInput << "\n";
  }
}


void dispatch(
  const int thrNo,
  Files& files,
  const Options& options,
  ValStats& vstats,
  TextStats& tstats,
  CompStats& cstats,
  Timers& timers)
{
  ofstream freal;
  if (options.fileLog.setFlag)
    freal.open(options.fileLog.name + (thrNo == 0 ? "" : STR(thrNo)));
  ostream& flog = (options.fileLog.setFlag ? freal : cout);

  FileTask task;
  string text;
  text.reserve(100000);

  RefControl refControl = ERR_REF_STANDARD;

  while (files.next(task))
  {
    if (options.verboseIO)
      flog << "Input " << task.fileInput << endl;

    Group group;

    // TODO: Better dispatch description language.
    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
      goto DIGEST;

    timers.start(BRIDGE_TIMER_READ, task.formatInput);
    bool b = dispatchRead(task, group, options, refControl, flog);
    timers.stop(BRIDGE_TIMER_READ, task.formatInput);
    if (! b)
    {
      flog << "Failed to read " << task.fileInput << endl;
      continue;
    }
    if (refControl == ERR_REF_SKIP)
      continue;

    if (options.statsFlag)
    {
      if (options.verboseIO)
        flog << "Input " << task.fileInput << endl;
    
      timers.start(BRIDGE_TIMER_STATS, task.formatInput);
      dispatchStats(task, group, tstats, flog);
      timers.stop(BRIDGE_TIMER_STATS, task.formatInput);
    }

    for (auto &t: task.taskList)
    {
      if (options.verboseIO && t.fileOutput != "")
        flog << "Output " << t.fileOutput << endl;

      timers.start(BRIDGE_TIMER_WRITE, t.formatOutput);
      dispatchWrite(t, refControl, group, text, flog);
      timers.stop(BRIDGE_TIMER_WRITE, t.formatOutput);

      if (t.refFlag && refControl != ERR_REF_NOVAL)
      {
        if (options.verboseIO)
          flog << "Validating " << t.fileOutput <<
              " against " << t.fileRef << endl;

        timers.start(BRIDGE_TIMER_VALIDATE, t.formatOutput);
        dispatchValidate(task, t, options, text, vstats, flog);
        timers.stop(BRIDGE_TIMER_VALIDATE, t.formatOutput);
      }

      if (options.compareFlag && task.formatInput == t.formatOutput)
      {
        if (options.verboseIO)
          flog << "Comparing " << t.fileOutput <<
              " against " << task.fileInput << endl;

        timers.start(BRIDGE_TIMER_COMPARE, t.formatOutput);
        dispatchCompare(options, task.fileInput, task.formatInput, 
          group, text, cstats, flog);
        timers.stop(BRIDGE_TIMER_COMPARE, t.formatOutput);
      }

      if (task.removeOutputFlag)
        remove(t.fileOutput.c_str());
    }

    DIGEST:
    if (options.fileDigest.setFlag || options.dirDigest.setFlag)
    {
      if (options.verboseIO)
        flog << "Digest input " << task.fileInput << endl;
    
      timers.start(BRIDGE_TIMER_DIGEST, task.formatInput);
      dispatchDigest(options, task, flog);
      timers.stop(BRIDGE_TIMER_DIGEST, task.formatInput);
    }
  }
}


static void readFix(
  const string& fname,
  vector<Fix>& fix)
{
  regex re("\\.\\w+$");
  string fixName = regex_replace(fname, re, string(".fix"));
  
  // There might not be a fix file (not an error).
  ifstream fixstr(fixName.c_str());
  if (! fixstr.is_open())
    return;

  string line;
  regex rep("^(\\d+)\\s+\"([^\"]*)\"\\s+\"([^\"]*)\"\\s*$");
  smatch match;
  Fix newFix;
  while (getline(fixstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    if (! regex_search(line, match, rep) || match.size() < 3)
      THROW("Fix file " + fixName + ": Syntax error in '" + line + "'");

    if (! str2unsigned(match.str(1), newFix.no))
      THROW("Fix file " + fixName + ": Bad number in '" + line + "'");

    bool found = false;
    unsigned i;
    for (i = 0; i <= BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      if (LABEL_NAMES[i] == match.str(2))
      {
        found = true;
        break;
      }
    }

    if (! found)
      THROW("Fix file " + fixName + ": Unknown label in '" + line + "'");

    newFix.label = static_cast<Label>(i);
    newFix.value = match.str(3);

    // Replace backslash-n with literal newline.
    regex renl("\\\\n");
    newFix.value = regex_replace(newFix.value, renl, string("\n"));

    fix.push_back(newFix);
  }

  fixstr.close();
}


static void fixChunk(
  Chunk& chunk,
  bool& newSegFlag,
  vector<Fix>& fix)
{
  if (fix[0].label == BRIDGE_FORMAT_LABELS_SIZE)
    newSegFlag = (fix[0].value != "0");
  else
    chunk.set(fix[0].label, fix[0].value);

  fix.erase(fix.begin());
}


static void printCounts(
  const string& fname,
  const Counts& counts)
{
  cout << "Input file:   " << fname << endl;
  cout << "Segment:      " << counts.segno << endl;
  cout << "Board:        " << counts.bno << endl;

  unsigned lo = BIGNUM;
  unsigned hi = 0;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    if (counts.lno[i] == BIGNUM)
      continue;
    if (counts.lno[i] > hi)
      hi = counts.lno[i];
    if (counts.lno[i] < lo)
      lo = counts.lno[i];
  }

  if (lo == hi)
    cout << "Line number:  " << lo << endl;
  else
    cout << "Line numbers: " << lo << " to " << hi << endl << endl;
}


static void str2board(
  const string bno,
  const Format format,
  Counts& counts)
{
  if (bno == "")
    counts.curr.no = 0;
  else if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
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
  else
  {
    if (! str2upos(bno, counts.curr.no))
      THROW("Not a board number");
  }
}


static void advance(
  boardIDLIN& expectBoard,
  const Counts& counts)
{
  if (expectBoard.no == 0)
  {
    expectBoard.no = counts.bExtmin;
    expectBoard.roomFlag = true;
  }
  else if (expectBoard.roomFlag)
    expectBoard.roomFlag = false;
  else
  {
    expectBoard.no++;
    expectBoard.roomFlag = true;
  }
}


static bool operator < (
  const boardIDLIN& lhs,
  const boardIDLIN& rhs)
{
  return (lhs.no < rhs.no ||
      (lhs.no == rhs.no && ! rhs.roomFlag && lhs.roomFlag));
}


static bool operator == (
  const boardIDLIN& lhs,
  const boardIDLIN& rhs)
{
  return (lhs.no == rhs.no && rhs.roomFlag == lhs.roomFlag);
}


static bool operator > (
  const boardIDLIN& lhs,
  const boardIDLIN& rhs)
{
  return (lhs.no > rhs.no ||
      (lhs.no == rhs.no && rhs.roomFlag && ! lhs.roomFlag));
}


static void tryFormatMethod(
  const Format format,
  const string& text,
  Segment * segment,
  Board * board,
  const unsigned label)
{
  if (label <= BRIDGE_FORMAT_ROOM)
    (segment->*segPtr[label])(text, format);
  else 
    (board->*boardPtr[label])(text, format);
}


static bool storeChunk(
  Group& group,
  Segment * segment,
  Board * board,
  const Buffer& buffer,
  Chunk& chunk,
  const Counts& counts,
  const Format format,
  const Options& options,
  ostream& flog,
  const bool useDefaultsFlag = true)
{
  if (useDefaultsFlag)
  {
    segment->copyPlayers();

    if (chunk.isEmpty(BRIDGE_FORMAT_AUCTION) ||
        ((format == BRIDGE_FORMAT_LIN ||
          format == BRIDGE_FORMAT_LIN_VG ||
          format == BRIDGE_FORMAT_LIN_RP ||
          format == BRIDGE_FORMAT_LIN_TRN) &&
         chunk.isEmpty(BRIDGE_FORMAT_VULNERABLE)))
    {
      // Guess dealer and vul from the board number.
      if (chunk.isEmpty(BRIDGE_FORMAT_BOARD_NO))
      {
        chunk.guessDealerAndVul(segment->getActiveExtBoardNo(), format);
      }
      else if (format == BRIDGE_FORMAT_LIN_VG && board->hasDealerVul())
      {
        chunk.set(BRIDGE_FORMAT_VULNERABLE, board->strVul(BRIDGE_FORMAT_PAR));
      }
      else if ((format != BRIDGE_FORMAT_LIN &&
          format != BRIDGE_FORMAT_LIN_VG &&
          format != BRIDGE_FORMAT_LIN_RP &&
          format != BRIDGE_FORMAT_LIN_TRN) ||
          chunk.isEmpty(BRIDGE_FORMAT_VULNERABLE))
      {
        chunk.guessDealerAndVul(format);
      }
    }
  }

  unsigned i;
  try
  {
    for (i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      if (chunk.isEmpty(static_cast<Label>(i)))
      {
        if (useDefaultsFlag &&
            i == BRIDGE_FORMAT_CONTRACT && 
            FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
        {
          segment->loadSpecificsFromHeader(
            chunk.get(BRIDGE_FORMAT_BOARD_NO), format);
        }

        continue;
      }

      tryFormatMethod(format, chunk.get(i), segment, board, i);
    }
  }
  catch (Bexcept& bex)
  {
    if (options.verboseThrow)
    {
      printCounts(group.name(), counts);
      chunk.str(static_cast<Label>(i));
    }

    bex.print(flog);

    if (bex.isTricks() && FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
    {
      heurFixTricks(group, segment, board, buffer, chunk, counts, options);
    }
    else if (bex.isPlay())
    {
      cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
      cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;
    }
    else if (bex.isClaim())
    {
      heurFixTricks(group, segment, board, buffer, chunk, counts, options);
    }
    else if (bex.isPlayDD() && 
        FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
    {
      heurFixPlayDD(group, segment, board, buffer, chunk, options);
    }

    if (options.verboseBatch)
      cout << chunk.str();
    return false;
  }

  if (! board->skipped() && ! board->auctionIsOver())
  {
    if (board->lengthAuction() > 0)
    {
      printCounts(group.name(), counts);
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

  return true;
}


static bool fillBoards(
  Group& group, 
  Segment * segment, 
  Board *& board, 
  const Buffer& buffer,
  Chunk& chunk, 
  Counts& counts,
  boardIDLIN& lastBoard,
  const Format format,
  const Options& options,
  ostream& flog)
{
  boardIDLIN expectBoard = lastBoard;
  advance(expectBoard, counts);

  if (counts.curr == expectBoard)
    return true;

  if (counts.curr < expectBoard)
  {
    // Boards are not in order.
    if (counts.curr.no < counts.bExtmin)
      THROW("Not a valid board: " + STR(counts.curr.no));
    const unsigned intNo = counts.curr.no - counts.bExtmin;

    board = segment->getBoard(intNo);
    if (board == nullptr)
      THROW("Not a board: " + STR(counts.curr.no));

    if (counts.curr.roomFlag)
      board->setInstance(0);
    else
      board->setInstance(1);

    board->unmarkInstanceSkip();

    return storeChunk(group, segment, board, buffer, chunk,
        counts, format, options, flog, false);
  }

  Chunk chunkSynth;

  // Need the header for the very first synthetic board.
  if (chunk.isSet(BRIDGE_FORMAT_TITLE))
  {
    chunkSynth.copyFrom(chunk, CHUNK_BOARD);
    chunk.reset(CHUNK_BOARD);
  }

  do
  {
    chunkSynth.set(BRIDGE_FORMAT_BOARD_NO, 
      (expectBoard.roomFlag ? 'o' : 'c') + STR(expectBoard.no));

    if (expectBoard.no != lastBoard.no)
    {
      board = segment->acquireBoard(counts.bno);
      counts.bno++;
    }

    if ((format == BRIDGE_FORMAT_LIN_VG ||
         format == BRIDGE_FORMAT_LIN_RP) &&
        expectBoard.no == counts.curr.no)
    {
      chunkSynth.copyFrom(chunk, CHUNK_DVD);
    }

    lastBoard = expectBoard;
    board->newInstance();
    board->markInstanceSkip();
    if (! storeChunk(group, segment, board, buffer, chunkSynth,
        counts, format, options, flog))
      return false;
    advance(expectBoard, counts);

    if (chunkSynth.isSet(BRIDGE_FORMAT_TITLE))
    {
      chunkSynth.reset(CHUNK_BOARD);
    }
  }
  while (counts.curr > expectBoard);
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
  RefControl& refControl,
  ostream& flog)
{
  Buffer buffer;
  buffer.read(fname, format, refControl);
  if (refControl == ERR_REF_SKIP)
    return true;

  vector<Fix> fix;
  fix.clear();
  readFix(fname, fix);

  group.setName(fname);

  return readFormattedFile(buffer, fix, format, group, options, flog);
}


static bool readFormattedFile(
  Buffer& buffer,
  vector<Fix>& fix,
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
  counts.lno.resize(BRIDGE_FORMAT_LABELS_SIZE);

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
      for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
        counts.lno[i] = BIGNUM;


      (* formatFncs[format].readChunk)
        (buffer, counts.lno, chunk, newSegFlag);

      if (chunk.isEmpty(BRIDGE_FORMAT_BOARD_NO) &&
          chunk.isEmpty(BRIDGE_FORMAT_RESULT) &&
          chunk.isEmpty(BRIDGE_FORMAT_AUCTION))
        break;
    }
    catch (Bexcept& bex)
    {
      if ((format == BRIDGE_FORMAT_LIN ||
           format == BRIDGE_FORMAT_LIN_TRN) &&
          bex.isNoCards() &&
          chunk.isEmpty(BRIDGE_FORMAT_TITLE))
      {
        // Empty hand.
        continue;
      }
          
      if (options.verboseThrow)
        printCounts(group.name(), counts);

      bex.print(flog);

      if (options.verboseBatch)
        cout << chunk.str();
      return false;
    }

    while (fix.size() > 0 && fix[0].no == counts.chunkno)
      fixChunk(chunk, newSegFlag, fix);

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
          // format != BRIDGE_FORMAT_LIN_RP)
      {
        try
        {
          chunk.getRange(counts);
        }
        catch(Bexcept& bex)
        {
          if (options.verboseThrow)
            printCounts(group.name(), counts);

          bex.print(flog);

          if (options.verboseBatch)
            cout << chunk.str();
          return false;
        }
      }
    }

    str2board(chunk.get(BRIDGE_FORMAT_BOARD_NO), format, counts);

    if (format == BRIDGE_FORMAT_LIN_VG ||
        format == BRIDGE_FORMAT_LIN_RP ||
        format == BRIDGE_FORMAT_LIN_TRN)
    {
      if (! fillBoards(group, segment, board, buffer, chunk, counts,
          lastBoard, format, options, flog))
        return false;
      if (counts.curr < lastBoard)
      {
        // This was a backfill operation.
        continue;
      }
    }

    if (counts.curr.no != 0)
    {
      if (counts.curr.no != lastBoard.no)
      {
        // New board.
        board = segment->acquireBoard(counts.bno);
        counts.bno++;
      }
      else if ((format == BRIDGE_FORMAT_LIN_VG ||
          format == BRIDGE_FORMAT_LIN_RP ||
          format == BRIDGE_FORMAT_LIN_TRN) &&
          counts.bExtmin + counts.bno != counts.curr.no)
      {
        // Usually not needed, but sometimes we moved the board
        // pointer while backfilling.
        const unsigned intNo = counts.curr.no - counts.bExtmin;
        board = segment->getBoard(intNo);
      }
    }

    lastBoard = counts.curr;

    board->newInstance();
    if (! storeChunk(group, segment, board, buffer, chunk,
        counts, format, options, flog))
      return false;
  }

  if ((format == BRIDGE_FORMAT_LIN_VG ||
       format == BRIDGE_FORMAT_LIN_RP) && 
       board != nullptr)
  {
    // Fill out trailing skips.
    counts.curr.no = counts.bExtmax+1;
    counts.curr.roomFlag = true;

    if (! fillBoards(group, segment, board, buffer, chunk, counts,
        lastBoard, format, options, flog))
      return false;
  }

  return true;
}


static void logLengths(
  Group& group,
  TextStats& tstats,
  const string& fname,
  const Format format)
{
  for (auto &segment: group)
  {
    tstats.add(segment.strTitle(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_TITLE, format);

    tstats.add(segment.strLocation(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_LOCATION, format);

    tstats.add(segment.strEvent(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_EVENT, format);

    tstats.add(segment.strSession(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_SESSION, format);

    tstats.add(segment.strFirstTeam(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_TEAMS, format);

    tstats.add(segment.strSecondTeam(BRIDGE_FORMAT_PAR),
      fname, BRIDGE_FORMAT_TEAMS, format);

    for (auto &bpair: segment)
    {
      Board& board = bpair.board;

      for (unsigned i = 0; i < board.countAll(); i++)
      {
        board.setInstance(i);

        tstats.add(board.lengthAuction(),
          fname, BRIDGE_FORMAT_AUCTION, format);

        for (unsigned p = 0; p < BRIDGE_PLAYERS; p++)
        {
          tstats.add(
            board.strPlayer(static_cast<Player>(p), BRIDGE_FORMAT_PAR),
            fname, BRIDGE_FORMAT_PLAYERS_BOARD, format);
        }
      }
    }
  }
}


static void writeHeader(
  string& st,
  Group& group,
  const Format format)
{
  st = "";
  string tmp;
  const string g = guessOriginalLine(group.name(), group.count());
  if (g == "")
    return;

  if (format == BRIDGE_FORMAT_RBX)
  {
    st += "%{RBX " + g + "}";
    st += "%{www.rpbridge.net Richard Pavlicek}";
  }
  else if (format == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_LIN_TRN)
  {
    // Nothing.
    // TODO: Skip except when coming from a Pavlicek file.
  }
  else
  {
    st += "% " + FORMAT_EXTENSIONS[format] + " " + g + "\n";
    st += "% www.rpbridge.net Richard Pavlicek\n";
  }
}


static void writeFormattedFile(
  Group& group,
  const string& fname,
  const RefControl refControl,
  string &text,
  const Format format)
{
  WriteInfo writeInfo;

  writeHeader(text, group, format);

  for (auto &segment: group)
  {
    if (segment.size() == 0)
      continue;

    (* formatFncs[format].writeSeg)(text, segment, format);

    writeInfo.namesOld[0] = "";
    writeInfo.namesOld[1] = "";
    writeInfo.score1 = 0;
    writeInfo.score2 = 0;
    writeInfo.numBoards = segment.size();

    if (refControl == ERR_REF_OUT_COCO)
    {
      // c1, o1, c2, o2, ...
      for (auto &bpair: segment)
      {
        Board& board = bpair.board;
        segment.setBoard(bpair.no);

        writeInfo.bno = bpair.no;
        writeInfo.numInst = board.countAll();

        for (unsigned i = 0, j = writeInfo.numInst-1; 
            i < writeInfo.numInst; i++, j--)
        {
          board.setInstance(j);
          if (board.skipped())
            continue;

          writeInfo.ino = j;
          (* formatFncs[format].writeBoard)
            (text, segment, board, writeInfo, format);
        }
      }
    }
    else if (refControl == ERR_REF_OUT_OOCC)
    {
      // o1, o2, ..., c1, c2, ...
      for (unsigned i = 0; i < 2; i++)
      {
        for (auto &bpair: segment)
        {
          Board& board = bpair.board;
          segment.setBoard(bpair.no);

          writeInfo.bno = bpair.no;
          writeInfo.numInst = board.countAll();
          if (writeInfo.numInst > 2)
            THROW("Too many instances for OOCC output order");

          board.setInstance(i);
          if (board.skipped())
            continue;

          writeInfo.ino = i;
          (* formatFncs[format].writeBoard)
            (text, segment, board, writeInfo, format);
        }
      }
    }
    else
    {
      // o1, c1, o2, c2, ...
      for (auto &bpair: segment)
      {
        Board& board = bpair.board;
        segment.setBoard(bpair.no);

        writeInfo.bno = bpair.no;
        writeInfo.numInst = board.countAll();

        for (unsigned i = 0; i < writeInfo.numInst; i++)
        {
          board.setInstance(i);
          if (board.skipped())
            continue;

          writeInfo.ino = i;
          (* formatFncs[format].writeBoard)
            (text, segment, board, writeInfo, format);
        }
      }
    }
  }

  if (fname != "")
    writeFast(fname, text);
}


void mergeResults(
  vector<ValStats>& vstats,
  vector<TextStats>& tstats,
  vector<CompStats>& cstats,
  vector<Timers>& timers,
  const Options& options)
{
  if (options.numThreads == 1)
    return;

  for (unsigned i = 1; i < options.numThreads; i++)
  {
    vstats[0] += vstats[i];
    tstats[0] += tstats[i];
    cstats[0] += cstats[i];
    timers[0] += timers[i];
  }

  if (! options.fileLog.setFlag)
    return;

  ofstream fbase(options.fileLog.name, std::ofstream::app);
  if (! fbase.is_open())
    return;

  string line;
  for (unsigned i = 1; i < options.numThreads; i++)
  {
    const string fname = options.fileLog.name + STR(i);
    ifstream flog(fname);
    if (! flog.is_open())
      continue;

    fbase << "From thread " << i << ":\n";
    fbase << "-------------\n\n";

    while (getline(flog, line))
      fbase << line;

    flog.close();
    remove(fname.c_str());
  }
  fbase.close();
}


// http://stackoverflow.com/questions/11563963/
//   writing-a-binary-file-in-c-very-fast

static void writeFast(
  const string& fname,
  const string& text)
{
  FILE* pFile;
  pFile = fopen(fname.c_str(), "wb");
  fwrite(text.c_str(), 1, text.length(), pFile);
  fclose(pFile);
}

