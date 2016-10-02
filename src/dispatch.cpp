/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <assert.h>

#include "Group.h"
#include "Segment.h"
#include "Board.h"
#include "dispatch.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileRBX.h"
#include "fileEML.h"
#include "fileTXT.h"
#include "fileREC.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileRBX.h"
#include "fileTXT.h"
#include "fileEML.h"
#include "fileREC.h"

#include "bconst.h"
#include "parse.h"
#include "debug.h"
#include "Bexcept.h"
#include "portab.h"

extern Debug debug;



using namespace std;

struct FormatFunctionsType
{
  bool (* write)(Group&, const string&);
  void (* writeSeg)(ofstream&, Segment *, formatType);
  void (* writeBoard)(ofstream&, Segment *, Board *, 
    const writeInfoType&, const formatType);
  bool (* readChunk)(ifstream&, unsigned&, vector<string>&, bool&);
};

FormatFunctionsType formatFncs[BRIDGE_FORMAT_SIZE];

typedef bool (Segment::*SegPtr)(const string& s, const formatType f);
typedef bool (Board::*BoardPtr)(const string& s, const formatType f);

SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];


static bool readFormattedFile(
  const string& fname,
  const formatType f,
  Group& group);

static bool tryFormatMethod(
  const formatType f,
  const string& text,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr);

static void writeHeader(
  ofstream& fstr,
  Group& group,
  const formatType f);

static bool writeFormattedFile(
  Group& group,
  const string& fname,
  const formatType f);

static bool dummyWrite(
  Group& group,
  const string& fname)
{
  UNUSED(group);
  UNUSED(fname);
  return true;
}


void setTables()
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

  formatFncs[BRIDGE_FORMAT_LIN_EXT].write = &dummyWrite;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].readChunk = &readLINChunk;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].writeSeg = &writeLINSegmentLevel;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].writeBoard = &writeLINBoardLevel;

  formatFncs[BRIDGE_FORMAT_PBN].readChunk = &readPBNChunk;
  formatFncs[BRIDGE_FORMAT_PBN].writeSeg = &writePBNSegmentLevel;
  formatFncs[BRIDGE_FORMAT_PBN].writeBoard = &writePBNBoardLevel;

  formatFncs[BRIDGE_FORMAT_RBN].write = &writeRBN;
  formatFncs[BRIDGE_FORMAT_RBN].readChunk = &readRBNChunk;
  formatFncs[BRIDGE_FORMAT_RBN].writeSeg = &writeLINSegmentLevel; // TODO
  formatFncs[BRIDGE_FORMAT_RBN].writeBoard = &writeLINBoardLevel; // TODO

  formatFncs[BRIDGE_FORMAT_RBX].write = &writeRBX;
  formatFncs[BRIDGE_FORMAT_RBX].readChunk = &readRBXChunk;
  formatFncs[BRIDGE_FORMAT_RBX].writeSeg = &writeLINSegmentLevel; // TODO
  formatFncs[BRIDGE_FORMAT_RBX].writeBoard = &writeLINBoardLevel; // TODO

  formatFncs[BRIDGE_FORMAT_TXT].write = &writeTXT;
  formatFncs[BRIDGE_FORMAT_TXT].readChunk = &readTXTChunk;
  formatFncs[BRIDGE_FORMAT_TXT].writeSeg = &writeLINSegmentLevel; // TODO
  formatFncs[BRIDGE_FORMAT_TXT].writeBoard = &writeLINBoardLevel; // TODO

  formatFncs[BRIDGE_FORMAT_EML].write = &writeEML;
  formatFncs[BRIDGE_FORMAT_EML].readChunk = &readEMLChunk;
  formatFncs[BRIDGE_FORMAT_EML].writeSeg = &writeLINSegmentLevel; // TODO
  formatFncs[BRIDGE_FORMAT_EML].writeBoard = &writeLINBoardLevel; // TODO

  formatFncs[BRIDGE_FORMAT_REC].write = &writeREC;
  formatFncs[BRIDGE_FORMAT_REC].readChunk = &readRECChunk;
  formatFncs[BRIDGE_FORMAT_REC].writeSeg = &writeLINSegmentLevel; // TODO
  formatFncs[BRIDGE_FORMAT_REC].writeBoard = &writeLINBoardLevel; // TODO

  formatFncs[BRIDGE_FORMAT_PAR].write = &dummyWrite;

  setLINTables();
  setPBNTables();
  setRBNTables();
  setRBXTables();
  setTXTTables();
  setEMLTables();

  segPtr[BRIDGE_FORMAT_TITLE] = &Segment::SetTitle;
  segPtr[BRIDGE_FORMAT_DATE] = &Segment::SetDate;
  segPtr[BRIDGE_FORMAT_LOCATION] = &Segment::SetLocation;
  segPtr[BRIDGE_FORMAT_EVENT] = &Segment::SetEvent;
  segPtr[BRIDGE_FORMAT_SESSION] = &Segment::SetSession;
  segPtr[BRIDGE_FORMAT_SCORING] = &Segment::SetScoring;
  segPtr[BRIDGE_FORMAT_TEAMS] = &Segment::SetTeams;
  segPtr[BRIDGE_FORMAT_HOMETEAM] = &Segment::SetFirstTeam;
  segPtr[BRIDGE_FORMAT_VISITTEAM] = &Segment::SetSecondTeam;

  segPtr[BRIDGE_FORMAT_RESULTS_LIST] = &Segment::SetResultsList;
  segPtr[BRIDGE_FORMAT_PLAYERS_LIST] = &Segment::SetPlayersList;
  segPtr[BRIDGE_FORMAT_PLAYERS_HEADER] = &Segment::SetPlayersHeader;
  segPtr[BRIDGE_FORMAT_WEST] = &Segment::SetWest;
  segPtr[BRIDGE_FORMAT_NORTH] = &Segment::SetNorth;
  segPtr[BRIDGE_FORMAT_EAST] = &Segment::SetEast;
  segPtr[BRIDGE_FORMAT_SOUTH] = &Segment::SetSouth;
  segPtr[BRIDGE_FORMAT_SCORES_LIST] = &Segment::SetScoresList;
  segPtr[BRIDGE_FORMAT_BOARDS_LIST] = &Segment::SetBoardsList;

  segPtr[BRIDGE_FORMAT_BOARD_NO] = &Segment::SetNumber;
  segPtr[BRIDGE_FORMAT_PLAYERS_BOARD] = &Segment::SetPlayers;
  segPtr[BRIDGE_FORMAT_ROOM] = &Segment::SetRoom;

  boardPtr[BRIDGE_FORMAT_DEAL] = &Board::SetDeal;
  boardPtr[BRIDGE_FORMAT_DEALER] = &Board::SetDealer;
  boardPtr[BRIDGE_FORMAT_VULNERABLE] = &Board::SetVul;
  boardPtr[BRIDGE_FORMAT_AUCTION] = &Board::SetAuction;
  boardPtr[BRIDGE_FORMAT_DECLARER] = &Board::SetDeclarer;
  boardPtr[BRIDGE_FORMAT_CONTRACT] = &Board::SetContract;
  boardPtr[BRIDGE_FORMAT_PLAY] = &Board::SetPlays;
  boardPtr[BRIDGE_FORMAT_RESULT] = &Board::SetResult;
  boardPtr[BRIDGE_FORMAT_SCORE] = &Board::SetScore;
  boardPtr[BRIDGE_FORMAT_SCORE_IMP] = &Board::SetScoreIMP;
  boardPtr[BRIDGE_FORMAT_SCORE_MP] = &Board::SetScoreMP;
  boardPtr[BRIDGE_FORMAT_DOUBLE_DUMMY] = &Board::SetTableau;
}


void dispatch(
  const int thrNo,
  Files& files)
{
  UNUSED(thrNo);

  FileTaskType task;
  if (! files.GetNextTask(task))
    return;

  Group group;

  try
  {
    if (! readFormattedFile(task.fileInput, task.formatInput, group))
      THROW("something blew up");
  }
  catch(Bexcept& bex)
  {
    bex.Print();
    assert(false);
  }

  for (auto &t: task.taskList)
  {
    if (t.formatOutput == BRIDGE_FORMAT_LIN ||
        t.formatOutput == BRIDGE_FORMAT_LIN_RP ||
        t.formatOutput == BRIDGE_FORMAT_LIN_VG ||
        t.formatOutput == BRIDGE_FORMAT_LIN_TRN ||
        t.formatOutput == BRIDGE_FORMAT_PBN)
    {
      if (! writeFormattedFile(group, t.fileOutput, t.formatOutput))
      {
        debug.Print();
        assert(false);
      }
    }
    else
    {
    if (! (* formatFncs[t.formatOutput].write)(group, t.fileOutput))
    {
      debug.Print();
      assert(false);
    }
    }

    if (t.refFlag)
    {
      // Compare magic on t.fileOutput and t.fileRef: TODO
    }

    if (task.removeOutputFlag)
    {
      // delete t.fileOutput: TODO
    }
  }

}

static bool readFormattedFile(
  const string& fname,
  const formatType f,
  Group& group)
{
  ifstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("No such file: " + fname);
    return false;
  }

  group.SetFileName(fname);
  group.SetInputFormat(f);

  vector<string> chunk(BRIDGE_FORMAT_LABELS_SIZE);

  Segment * segment = nullptr;
  unsigned segno = 0;
  bool newSegFlag = false;

  Board * board = nullptr;
  unsigned bno = 0;
  string lastBoard = "";

  unsigned lno = 0;

  while ((* formatFncs[f].readChunk)(fstr, lno, chunk, newSegFlag))
  {
    if (newSegFlag || segment == nullptr)
    {
      if (! group.MakeSegment(segno))
      {
        LOG("Cannot make segment " + STR(segno));
        fstr.close();
        return false;
      }

      segment = group.GetSegment(segno);
      segno++;
      bno = 0;
    }

    if (chunk[BRIDGE_FORMAT_BOARD_NO] != "" &&
        chunk[BRIDGE_FORMAT_BOARD_NO] != lastBoard &&
        chunk[BRIDGE_FORMAT_BOARD_NO].at(0) != 'c')
    {
      // New board.
      lastBoard = chunk[BRIDGE_FORMAT_BOARD_NO];
      board = segment->AcquireBoard(bno);
      bno++;

      if (board == nullptr)
      {
        LOG("Unknown error");
        fstr.close();
        return false;
      }
    }

    board->NewInstance();
    segment->CopyPlayers();

    for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      if (chunk[i] != "" &&
          ! tryFormatMethod(f, chunk[i], segment, board, i, fstr))
        return false;
    }

    // Have to wait until after the methods with this.
    // Only applies to LIN.
    segment->TransferHeader(bno-1, board->GetInstance());

    if (fstr.eof())
      break;
  }

  fstr.close();
  return true;
}


static bool tryFormatMethod(
  const formatType f,
  const string& text,
  Segment * segment,
  Board * board,
  const unsigned label,
  ifstream& fstr)
{
  if (label <= BRIDGE_FORMAT_ROOM)
  {
    if ((segment->*segPtr[label])(text, f))
      return true;
    else
    {
      LOG("Cannot add " + formatLabelNames[label] + " line " + text + "'");
      fstr.close();
      return false;
    }
  }
  else  if ((board->*boardPtr[label])(text, f))
    return true;
  else
  {
    LOG("Cannot add " + formatLabelNames[label] + " line " + text + "'");
    fstr.close();
    return false;
  }
}


static void writeHeader(
  ofstream& fstr,
  Group& group,
  const formatType f)
{
  const string g = GuessOriginalLine(group.GetFileName(), group.GetCount());
  if (g == "")
    return;

  fstr << "% " << FORMAT_EXTENSIONS[f] << " " << g << "\n";
  fstr << "% www.rpbridge.net Richard Pavlicek\n";
}


static bool writeFormattedFile(
  Group& group,
  const string& fname,
  const formatType f)
{
  ofstream fstr(fname.c_str());
  if (! fstr.is_open())
  {
    LOG("Cannot write to: " + fname);
    return false;
  }

  writeInfoType writeInfo;
  writeHeader(fstr, group, f);

  for (unsigned g = 0; g < group.GetLength(); g++)
  {
    Segment * segment = group.GetSegment(g);

    (* formatFncs[f].writeSeg)(fstr, segment, f);

    writeInfo.numBoards = segment->GetLength();
    for (unsigned b = 0; b < writeInfo.numBoards; b++)
    {
      Board * board = segment->GetBoard(b);
      if (board == nullptr)
      {
        LOG("Invalid board");
        fstr.close();
        return false;
      }

      writeInfo.bno = b;
      writeInfo.numInst = board->GetLength();

      for (unsigned i = 0; i < writeInfo.numInst; i++)
      {
        if (! board->SetInstance(i))
        {
          LOG("Invalid instance");
          fstr.close();
          return false;
        }

        writeInfo.ino = i;
        (* formatFncs[f].writeBoard)(fstr, segment, board, writeInfo, f);
      }
    }
  }

  fstr.close();
  return true;
}

