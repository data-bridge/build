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

