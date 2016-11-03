/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>
#include <assert.h>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "dispatch.h"
#include "validate.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileTXT.h"
#include "fileEML.h"
#include "fileREC.h"

#include "parse.h"
#include "portab.h"
#include "Bexcept.h"
#include "Bdiff.h"


// Modulo 4, so West for Board "0" (4, 8, ...) etc.
//
static const playerType BOARD_TO_DEALER[4] = 
{
  BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST, BRIDGE_SOUTH
};

// Modulo 16, so EW for Board "0" (16, 32, ...) etc.

static const vulType BOARD_TO_VUL[16] =
{
  BRIDGE_VUL_EAST_WEST, 
  BRIDGE_VUL_NONE, 
  BRIDGE_VUL_NORTH_SOUTH, 
  BRIDGE_VUL_EAST_WEST,

  BRIDGE_VUL_BOTH,
  BRIDGE_VUL_NORTH_SOUTH,
  BRIDGE_VUL_EAST_WEST,
  BRIDGE_VUL_BOTH,

  BRIDGE_VUL_NONE, 
  BRIDGE_VUL_EAST_WEST,
  BRIDGE_VUL_BOTH,
  BRIDGE_VUL_NONE, 

  BRIDGE_VUL_NORTH_SOUTH,
  BRIDGE_VUL_BOTH,
  BRIDGE_VUL_NONE, 
  BRIDGE_VUL_NORTH_SOUTH
};



using namespace std;

struct FormatFunctionsType
{
  bool (* readChunk)(ifstream&, unsigned&, vector<string>&, bool&);
  void (* writeSeg)(ofstream&, Segment&, Format);
  void (* writeBoard)(ofstream&, Segment&, Board&, 
    writeInfoType&, const Format);
};

FormatFunctionsType formatFncs[BRIDGE_FORMAT_SIZE];

typedef void (Segment::*SegPtr)(const string& s, const Format format);
typedef void (Board::*BoardPtr)(const string& s, const Format format);

SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];

struct Fix
{
  unsigned no;
  formatLabelType field;
  string value;
};


void writeDummySegmentLevel(
  ofstream& fstr,
  Segment& segment,
  const Format format);

void GuessDealerAndVul(
  vector<string>& chunk, 
  const unsigned b,
  const Format format);

void GuessDealerAndVul(
  vector<string>& chunk, 
  const string& s,
  const Format format);

static void readFix(
  const string& fname,
  vector<Fix>& fix);

static bool readFormattedFile(
  const string& fname,
  const Format format,
  Group& group,
  const OptionsType& options);

static void tryFormatMethod(
  const Format format,
  const string& text,
  Segment * segment,
  Board * board,
  const unsigned label);

static void writeHeader(
  ofstream& fstr,
  Group& group,
  const Format format);

static bool writeFormattedFile(
  Group& group,
  const string& fname,
  const Format format);

static void setFormatTables();

static void SetIO();

static void SetInterface();


void writeDummySegmentLevel(
  ofstream& fstr,
  Segment& segment,
  const Format format)
{
  UNUSED(fstr);
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

  formatFncs[BRIDGE_FORMAT_LIN_EXT].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].writeSeg = &writeLINSegmentLevel;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].writeBoard = &writeLINBoardLevel;

  formatFncs[BRIDGE_FORMAT_PBN].readChunk = &readPBNChunk;
  formatFncs[BRIDGE_FORMAT_PBN].writeSeg = &writeDummySegmentLevel;
  formatFncs[BRIDGE_FORMAT_PBN].writeBoard = &writePBNBoardLevel;

  formatFncs[BRIDGE_FORMAT_RBN].readChunk = &readRBNChunk;
  formatFncs[BRIDGE_FORMAT_RBN].writeSeg = &writeRBNSegmentLevel;
  formatFncs[BRIDGE_FORMAT_RBN].writeBoard = &writeRBNBoardLevel;

  formatFncs[BRIDGE_FORMAT_RBX].readChunk = &readRBXChunk;
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
}


void dispatch(
  const int thrNo,
  Files& files,
  const OptionsType& options,
  ValStats& vstats,
  Timers& timers)
{
  ofstream freal;
  if (options.fileLog.setFlag)
    freal.open(options.fileLog.name + (thrNo == 0 ? "" : STR(thrNo)));
  ostream& flog = (options.fileLog.setFlag ? freal : cout);

  FileTask task;
  while (files.next(task))
  {
    if (options.verboseIO)
      flog << "Input " << task.fileInput << endl;

    Group group;
    timers.start(BRIDGE_TIMER_READ, task.formatInput);
    try
    {
      if (! readFormattedFile(task.fileInput, task.formatInput, 
          group, options))
      {
        THROW("dispatch: read failed");
      }
    }
    catch (Bexcept& bex)
    {
      bex.print();
      continue;
    }
    timers.stop(BRIDGE_TIMER_READ, task.formatInput);

    for (auto &t: task.taskList)
    {
      if (options.verboseIO)
        flog << "Output " << t.fileOutput << endl;

      timers.start(BRIDGE_TIMER_WRITE, task.formatInput);
      try
      {
        if (! writeFormattedFile(group, t.fileOutput, t.formatOutput))
          THROW("something blew up");
      }
      catch (Bexcept& bex)
      {
        bex.print();
        continue;
      }
      timers.stop(BRIDGE_TIMER_WRITE, task.formatInput);

      if (t.refFlag)
      {
        if (options.verboseIO)
          flog << "Validating " << t.fileOutput <<
              " against " << t.fileRef << endl;

        timers.start(BRIDGE_TIMER_VALIDATE, task.formatInput);
        try
        {
          validate(t.fileOutput, t.fileRef,
            task.formatInput, t.formatOutput, options, vstats);
        }
        catch(Bexcept& bex)
        {
          bex.print();
        }
        timers.stop(BRIDGE_TIMER_VALIDATE, task.formatInput);
      }

      if (task.removeOutputFlag)
      {
        // delete t.fileOutput: TODO
      }
    }
  }
}


void GuessDealerAndVul(
  vector<string>& chunk, 
  const unsigned b,
  const Format format)
{
  // This is not quite fool-proof, as there are LIN files where
  // the board numbers don't match...

  if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
  {
    chunk[BRIDGE_FORMAT_DEALER] = PLAYER_NAMES_SHORT[BOARD_TO_DEALER[b % 4]];
    chunk[BRIDGE_FORMAT_VULNERABLE] = VUL_NAMES_PBN[BOARD_TO_VUL[b % 16]];
  }
}


void GuessDealerAndVul(
  vector<string>& chunk, 
  const string& s,
  const Format format)
{
  unsigned u;
  if (! StringToNonzeroUnsigned(s, u))
    return;

  GuessDealerAndVul(chunk, u, format);
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

    if (! StringToUnsigned(match.str(1), newFix.no))
      THROW("Fix file " + fixName + ": Bad number in '" + line + "'");

    bool found = false;
    unsigned i;
    for (i = 0; i <= BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      if (formatLabelNames[i] == match.str(2))
      {
        found = true;
        break;
      }
    }

    if (! found)
      THROW("Fix file " + fixName + ": Unknown label in '" + line + "'");

    newFix.field = static_cast<formatLabelType>(i);
    newFix.value = match.str(3);

    fix.push_back(newFix);
  }

  fixstr.close();
}


static void fixChunk(
  vector<string>& chunk,
  bool& newSegFlag,
  vector<Fix>& fix)
{
  if (fix[0].field == BRIDGE_FORMAT_LABELS_SIZE)
    newSegFlag = (fix[0].value != "0");
  else
    chunk[fix[0].field] = fix[0].value;

  fix.erase(fix.begin());
}


static bool readFormattedFile(
  const string& fname,
  const Format format,
  Group& group,
  const OptionsType& options)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    cout << "No such file: " << fname << endl;
    return false;
  }

  vector<Fix> fix;
  readFix(fname, fix);

  group.setName(fname);
  group.setFormat(format);

  vector<string> chunk(BRIDGE_FORMAT_LABELS_SIZE);

  Segment * segment = nullptr;
  unsigned segno = 0;
  bool newSegFlag = false;

  Board * board = nullptr;
  unsigned bno = 0;
  string lastBoard = "";
  unsigned chunkNo = 0;

  unsigned lno = 0;
  unsigned lnoOld;

  while (true)
  {
    lnoOld = lno;
    try
    {
      if (! (* formatFncs[format].readChunk)(fstr, lno, chunk, newSegFlag))
      {
        if (fstr.eof())
          break;
        else
          THROW("Early end");
      }
    }
    catch (Bexcept& bex)
    {
      if (options.verboseThrow)
      {
        cout << "Input file " << fname << endl;
        if (lnoOld+1 > lno-1)
          cout << "Line number " << lnoOld << endl;
        else
          cout << "Line numbers " << lnoOld+2 << " to " << lno-1 << 
              ", chunk " << chunkNo << endl << endl;
      }

      bex.print();

      if (options.verboseBatch)
      {
        cout << endl;
        for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
        {
          if (chunk[i] != "")
          {
            cout << setw(15) << formatLabelNames[i] <<
                " (" << setw(2) << i << "), '" <<
                chunk[i] << "'" << endl;
          }
        }
      }

      cout << endl;
      fstr.close();
      return false;
    }

    while (fix.size() > 0 && fix[0].no == chunkNo)
      fixChunk(chunk, newSegFlag, fix);

    chunkNo++;

    if (newSegFlag || segment == nullptr)
    {
      // segment = group.make(segno);
      segment = group.make();
      segno++;
      bno = 0;
    }

    if (chunk[BRIDGE_FORMAT_BOARD_NO] != "" &&
        chunk[BRIDGE_FORMAT_BOARD_NO] != lastBoard &&
        (format != BRIDGE_FORMAT_LIN ||
        lastBoard == "" ||
        chunk[BRIDGE_FORMAT_BOARD_NO].substr(1) != lastBoard.substr(1)))
    {
      // New board.
      lastBoard = chunk[BRIDGE_FORMAT_BOARD_NO];
      board = segment->acquireBoard(bno);
      bno++;

      if (board == nullptr)
      {
        fstr.close();
        THROW("Unknown error");
      }
    }

    board->newInstance();
    segment->copyPlayers();

    if (chunk[BRIDGE_FORMAT_AUCTION] == "")
    {
      // Guess dealer and vul from the board number.
      if (chunk[BRIDGE_FORMAT_BOARD_NO] == "")
        GuessDealerAndVul(chunk, segment->getActiveExtBoardNo(), format);
      else
        GuessDealerAndVul(chunk, chunk[BRIDGE_FORMAT_BOARD_NO], format);
    }

    unsigned i;
    try
    {
      for (i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
      {
        if (chunk[i] == "")
        {
          if (i == BRIDGE_FORMAT_CONTRACT && format == BRIDGE_FORMAT_LIN)
            segment->loadSpecificsFromHeader(chunk[BRIDGE_FORMAT_BOARD_NO]);

          continue;
        }

        tryFormatMethod(format, chunk[i], segment, board, i);
      }
    }
    catch (Bexcept& bex)
    {
      if (options.verboseThrow)
      {
        cout << "Input file " << fname << endl;
        if (lnoOld+1 > lno-1)
          cout << "Line number " << lnoOld;
        else
          cout << "Line numbers " << lnoOld+2 << " to " << lno-1;
        cout << ", chunk " << chunkNo-1 << ", bno " << bno-1 << endl;
        cout << "label " << formatLabelNames[i] << 
            " (" << i << "), '" <<
            chunk[i] << "'" << endl << endl;
      }

      bex.print();

      if (options.verboseBatch)
      {
        cout << endl;
        for (i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
        {
          if (chunk[i] != "")
          {
            cout << setw(15) << formatLabelNames[i] <<
                " (" << setw(2) << i << "), '" <<
                chunk[i] << "'" << endl;
          }
        }
      }

      cout << endl;
      fstr.close();
      return false;
    }


    // Have to wait until after the methods with this.
    // Only applies to LIN.
    // segment->TransferHeader(bno-1, board->GetInstance());

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


static void tryFormatMethod(
  const Format format,
  const string& text,
  Segment * segment,
  Board * board,
  const unsigned label)
{
  if (label <= BRIDGE_FORMAT_ROOM)
  {
    (segment->*segPtr[label])(text, format);
  }
  else 
    (board->*boardPtr[label])(text, format);
}


static void writeHeader(
  ofstream& fstr,
  Group& group,
  const Format format)
{
  const string g = GuessOriginalLine(group.name(), group.count());
  if (g == "")
    return;

  if (format == BRIDGE_FORMAT_RBX)
  {
    fstr << "%{RBX " << g << "}";
    fstr << "%{www.rpbridge.net Richard Pavlicek}";
  }
  else
  {
    fstr << "% " << FORMAT_EXTENSIONS[format] << " " << g << "\n";
    fstr << "% www.rpbridge.net Richard Pavlicek\n";
  }
}


static bool writeFormattedFile(
  Group& group,
  const string& fname,
  const Format format)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
    THROW("Cannot write to: " + fname);

  writeInfoType writeInfo;
  writeInfo.namesOld[0] = "";
  writeInfo.namesOld[1] = "";
  writeInfo.score1 = 0;
  writeInfo.score2 = 0;

  writeHeader(fstr, group, format);

  for (auto &segment: group)
  {
    (* formatFncs[format].writeSeg)(fstr, segment, format);

    writeInfo.numBoards = segment.size();
    // for (unsigned b = 0; b < writeInfo.numBoards; b++)
    for (auto &bpair: segment)
    {
      // Board * board = segment.getBoard(b);
      Board& board = bpair.board;
      segment.setBoard(bpair.no);

      // if (board == nullptr)
      // {
        // fstr.close();
        // THROW("Invalid board");
      // }

      // writeInfo.bno = b;
      writeInfo.bno = bpair.no;
      // writeInfo.numInst = board->count();
      writeInfo.numInst = board.count();

      for (unsigned i = 0; i < writeInfo.numInst; i++)
      {
        // board->setInstance(i);
        board.setInstance(i);
        writeInfo.ino = i;
        (* formatFncs[format].writeBoard)
          (fstr, segment, board, writeInfo, format);
      }
    }
  }

  fstr.close();
  return true;
}


#include <cstdio>

void mergeResults(
  vector<ValStats>& vstats,
  vector<Timers>& timers,
  const OptionsType& options)
{
  if (options.numThreads == 1)
    return;

  for (unsigned i = 1; i < options.numThreads; i++)
  {
    vstats[0] += vstats[i];
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

