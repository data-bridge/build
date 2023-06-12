/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iomanip>
#include <sstream>
#include <fstream>

#include "../Group.h"
#include "../Chunk.h"
#include "../RefLines.h"

#include "../fileLIN.h"
#include "../filePBN.h"
#include "../fileRBN.h"
#include "../fileTXT.h"
#include "../fileEML.h"
#include "../fileREC.h"

#include "../OrderCounts.h"

#include "../parse.h"
#include "../Bexcept.h"

#include "funcRead.h"

using namespace std;

typedef void (* ReadPtr)(Buffer&, Chunk&, bool&);
typedef void (Segment::*SegPtr)(const string& s, const Format format);
typedef void (Board::*BoardPtr)(const string& s, const Format format);
typedef void (Instance::*InstPtr)(const string& s, const Format format);

static ReadPtr readChunk[BRIDGE_FORMAT_LABELS_SIZE];
static SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
static BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];
static InstPtr instPtr[BRIDGE_FORMAT_LABELS_SIZE];


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
  segPtr[BRIDGE_FORMAT_SCORES_LIST] = &Segment::setScoresList;
  segPtr[BRIDGE_FORMAT_BOARDS_LIST] = &Segment::setBoardsList;

  segPtr[BRIDGE_FORMAT_BOARD_NO] = &Segment::setNumber;
  segPtr[BRIDGE_FORMAT_ROOM] = &Segment::setRoom;

  boardPtr[BRIDGE_FORMAT_DEAL] = &Board::setDeal;
  boardPtr[BRIDGE_FORMAT_DEALER] = &Board::setDealer;
  boardPtr[BRIDGE_FORMAT_VULNERABLE] = &Board::setVul;
  boardPtr[BRIDGE_FORMAT_SCORE_IMP] = &Board::setScoreIMP;
  boardPtr[BRIDGE_FORMAT_SCORE_MP] = &Board::setScoreMP;
  boardPtr[BRIDGE_FORMAT_DOUBLE_DUMMY] = &Board::setTableau;

  instPtr[BRIDGE_FORMAT_PLAYERS] = &Instance::setPlayers;
  instPtr[BRIDGE_FORMAT_WEST] = &Instance::setWest;
  instPtr[BRIDGE_FORMAT_NORTH] = &Instance::setNorth;
  instPtr[BRIDGE_FORMAT_EAST] = &Instance::setEast;
  instPtr[BRIDGE_FORMAT_SOUTH] = &Instance::setSouth;
  instPtr[BRIDGE_FORMAT_AUCTION] = &Instance::setAuction;
  instPtr[BRIDGE_FORMAT_DECLARER] = &Instance::setDeclarer;
  instPtr[BRIDGE_FORMAT_CONTRACT] = &Instance::setContract;
  instPtr[BRIDGE_FORMAT_PLAY] = &Instance::setPlays;
  instPtr[BRIDGE_FORMAT_RESULT] = &Instance::setResult;
  instPtr[BRIDGE_FORMAT_SCORE] = &Instance::setScore;
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
  Instance * instance,
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

    for (i = BRIDGE_FORMAT_DEAL; i <= BRIDGE_FORMAT_DOUBLE_DUMMY; i++)
    {
      const string text = chunk.get(i);
      if (text != "")
        (board->*boardPtr[i])(text, format);
    }

    for (i = BRIDGE_FORMAT_PLAYERS; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
    {
      const string text = chunk.get(i);
      if (text != "")
        (instance->*instPtr[i])(text, format);
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
    cout << instance->strContract(BRIDGE_FORMAT_TXT) << endl;

    if (options.verboseBatch)
      cout << chunk.str();
    return false;
  }

  if (! instance->auctionIsOver() && instance->lengthAuction() > 0)
  {
    printCounts(fname, chunk, counts);
    cout << board->strDeal(BRIDGE_FORMAT_TXT) << endl;
    cout << instance->strContract(BRIDGE_FORMAT_TXT) << endl;
    cout << instance->strAuction(BRIDGE_FORMAT_TXT) << endl;
    cout << instance->strPlay(BRIDGE_FORMAT_TXT) << endl;
    cout << "Error: Auction incomplete\n";
    return false;
  }

  return true;
}


bool dispatchReadBuffer(
  const Format format,
  const Options& options,
  Buffer& buffer,
  Group& group,
  BoardOrder& order,
  ostream& flog)
{
  group.setFormat(format);
  Chunk chunk, prevChunk;
  Segment * segment = nullptr;
  Board * board = nullptr;
  Instance * instance = nullptr;
  bool newSegFlag = false;
  Counts counts = {0, 0, 0, true};
  Counts countsPrev = {0, 0, 0, true};
  OrderCounts orderCounts;

  while (true)
  {
    if (format == BRIDGE_FORMAT_PBN)
      prevChunk.copyFrom(chunk, CHUNK_HEADER);

    chunk.reset();
    try
    {
      (* readChunk[format])(buffer, chunk, newSegFlag);
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

    if (chunk.seemsEmpty())
      break;

    // In PBN, the header may just be repeated.
    if (newSegFlag && format == BRIDGE_FORMAT_PBN)
      newSegFlag = chunk.differsFrom(prevChunk, CHUNK_HEADER);

    if (segment == nullptr || newSegFlag)
    {
      if (board != nullptr)
        board->calculateScore();

      segment = group.make();
      newSegFlag = false;
      counts.segno++;
      countsPrev.bno = 0;

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
    orderCounts.incr(counts, countsPrev);
    countsPrev = counts;

    if (board == nullptr || counts.bno != counts.prevno)
    {
      if (board != nullptr)
        board->calculateScore();

      board = segment->acquireBoard(counts.bno);
      counts.prevno = counts.bno;
    }

    const unsigned instNo = (counts.openFlag ? 0u : 1u);
    instance = board->acquireInstance(instNo);
    board->markUsed(instNo);

    if (! storeChunk(group.name(), format, options, counts,
        segment, board, instance, chunk, flog))
      return false;
  }

  if (board != nullptr)
    board->calculateScore();

  order = orderCounts.classify();
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

    // There is no information in the TXT, EML and REC files
    // to deduce the order.
    if (refLines.orderCOCO() &&
       (format == BRIDGE_FORMAT_TXT ||
        format == BRIDGE_FORMAT_EML ||
        format == BRIDGE_FORMAT_REC))
      group.setCOCO(format);

    BoardOrder orderSeen;
    bool b = dispatchReadBuffer(format, options, buffer, 
      group, orderSeen, flog);

    if (format != BRIDGE_FORMAT_TXT &&
        format != BRIDGE_FORMAT_EML &&
        format != BRIDGE_FORMAT_REC)
    {
      // For (some of) the other formats, we have to swap teams.
      if (refLines.validate())
        refLines.setOrder(orderSeen);
      if (orderSeen == ORDER_COCO)
      {
        group.setCOCO(format);
      }
    }

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

