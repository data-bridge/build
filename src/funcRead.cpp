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
#include "Chunk.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileTXT.h"
#include "fileEML.h"
#include "fileREC.h"

#include "funcRead.h"

#include "parse.h"
#include "Bexcept.h"


using namespace std;

typedef void (* ReadPtr)(Buffer&, Chunk&, bool&);
typedef void (Segment::*SegPtr)(const string& s, const Format format);
typedef void (Board::*BoardPtr)(const string& s, const Format format);

static ReadPtr readChunk[BRIDGE_FORMAT_LABELS_SIZE];
static SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
static BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];


void setReadTables()
{
  setLINTables();
  setPBNTables();
  setRBNTables();
  setTXTTables();
  setEMLTables();

  readChunk[BRIDGE_FORMAT_LIN] = &readLINChunk;
  readChunk[BRIDGE_FORMAT_LIN_RP] = &readLINChunk;
  readChunk[BRIDGE_FORMAT_LIN_VG] = &readLINChunk;
  readChunk[BRIDGE_FORMAT_LIN_TRN] = &readLINChunk;
  readChunk[BRIDGE_FORMAT_PBN] = &readPBNChunk;
  readChunk[BRIDGE_FORMAT_RBN] = &readRBNChunk;
  readChunk[BRIDGE_FORMAT_RBX] = &readRBNChunk; // !
  readChunk[BRIDGE_FORMAT_TXT] = &readTXTChunk;
  readChunk[BRIDGE_FORMAT_EML] = &readEMLChunk;
  readChunk[BRIDGE_FORMAT_REC] = &readRECChunk;

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


static void printCounts(
  const string& fname,
  const Chunk& chunk,
  const Counts& counts)
{
  cout << "Input file:   " << fname << endl;
  cout << "Segment:      " << counts.segno << endl;
  cout << "Board:        " << counts.bno << endl;
  cout << "Room:         " << (counts.openFlag ? "Open" : "Closed") << endl;
  cout << chunk.strRange();
}


static bool storeLINHeader(
  const string& fname,
  const Format format,
  const Options& options,
  const Counts& counts,
  Segment * segment,
  Chunk& chunk,
  ostream& flog)
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
  catch (Bexcept& bex)
  {
    if (options.verboseThrow)
      printCounts(fname, chunk, counts);

    bex.print(flog);

    if (options.verboseBatch)
      cout << chunk.str();
    return false;
  }

  return true;
}


static bool storeChunk(
  const string& fname,
  const Format format,
  const Options& options,
  const Counts& counts,
  Segment * segment,
  Board * board,
  Chunk& chunk,
  ostream& flog)
{
  if (((format == BRIDGE_FORMAT_RBN || format == BRIDGE_FORMAT_RBX) &&
      chunk.isEmpty(BRIDGE_FORMAT_AUCTION)) ||
      (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN &&
      chunk.isEmpty(BRIDGE_FORMAT_VULNERABLE)))
  {
    // Guess dealer and vul from the board number.
    chunk.guessDealerAndVul(counts.bno, format);
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
      printCounts(fname, chunk, counts);
      cout << chunk.str(static_cast<Label>(i));
    }

    bex.print(flog);

    cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
    cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;

    if (options.verboseBatch)
      cout << chunk.str();
    return false;
  }

  if (! board->skipped() && 
      ! board->auctionIsOver() &&
      board->lengthAuction() > 0)
  {
    printCounts(fname, chunk, counts);
    cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
    cout << board->strContract(BRIDGE_FORMAT_TXT) << endl;
    cout << board->strAuction(BRIDGE_FORMAT_TXT) << endl;
    cout << board->strPlay(BRIDGE_FORMAT_TXT) << endl;
    cout << "Error: Auction incomplete\n";
    return false;
  }

  board->spreadBasics(); // TODO: Automatic in Board?
  return true;
}


bool dispatchReadBuffer(
  const Format format,
  const Options& options,
  Buffer& buffer,
  Group& group,
  ostream& flog)
{
  group.setFormat(format);

  Chunk chunk, prevChunk;
  Segment * segment = nullptr;
  bool newSegFlag = false;

  Board * board = nullptr;

  Counts counts;
  counts.segno = 0;
  counts.bno = 0;
  counts.prevno = 0;

  while (true)
  {
    try
    {
      if (format == BRIDGE_FORMAT_PBN)
        prevChunk.copyFrom(chunk, CHUNK_HEADER);

      newSegFlag = false;
      chunk.reset();

      (* readChunk[format])(buffer, chunk, newSegFlag);

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

    if (newSegFlag && format == BRIDGE_FORMAT_PBN)
    {
      // May not really be a new segment.
      newSegFlag = chunk.differsFrom(prevChunk, CHUNK_HEADER);
    }

    if (newSegFlag || segment == nullptr)
    {
      segment = group.make();
      counts.segno++;

      if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
      {
        // Need to store the LIN header first, as it contains default
        // information for boards.
        if (! storeLINHeader(group.name(), format, options, counts,
            segment, chunk, flog))
          return false;
      }
      else if (format == BRIDGE_FORMAT_TXT ||
          format == BRIDGE_FORMAT_EML ||
          format == BRIDGE_FORMAT_REC)
      {
        // If COCO, then we start with open room here, so that the
        // first inversion is closed, and vice versa.
        counts.openFlag = segment->getCOCO();
      }
    }

    chunk.getCounts(format, counts);

    if (board == nullptr || counts.bno != counts.prevno)
    {
      board = segment->acquireBoard(counts.bno);
      counts.prevno = counts.bno;
    }

    board->acquireInstance(counts.openFlag ? 0u : 1u);
    board->unmarkInstanceSkip();

    if (! storeChunk(group.name(), format, options, counts,
        segment, board, chunk, flog))
      return false;
  }

  return true;
}


bool dispatchReadFile(
  const string& fname,
  const Format format,
  const Options& options,
  Group& group,
  RefLines& refLines,
  ostream& flog)
{
  try
  {
    Buffer buffer;
    buffer.read(fname, format, refLines);
    if (refLines.skip())
      return true;

    group.setName(fname);
    if (refLines.orderCOCO())
      group.setCOCO();

    bool b = dispatchReadBuffer(format, options, buffer, group, flog);
    refLines.setFileData(buffer.lengthOrig(), 
      group.count(), group.countBoards());

    return b;
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    return false;
  }
}

