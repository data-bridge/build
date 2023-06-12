/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>
#include <sstream>
#include <fstream>

#include "../Group.h"

#include "../formats/fileLIN.h"
#include "../formats/filePBN.h"
#include "../formats/fileRBN.h"
#include "../formats/fileTXT.h"
#include "../formats/fileEML.h"
#include "../formats/fileREC.h"

#include "../parse.h"
#include "../Bexcept.h"


using namespace std;

typedef void (*SegPtr)(string&, const Segment&, const Format);
typedef void (*BoardPtr)(string&, const Segment&, const Board&, 
  WriteInfo&, const Format);

static SegPtr segPtr[BRIDGE_FORMAT_LABELS_SIZE];
static BoardPtr boardPtr[BRIDGE_FORMAT_LABELS_SIZE];


static void writeDummySegmentLevel(
  [[maybe_unused]] string& st,
  [[maybe_unused]] const Segment& segment,
  [[maybe_unused]] const Format format)
{
}


void setWriteTables()
{
  segPtr[BRIDGE_FORMAT_LIN] = &writeLINSegmentLevel;
  segPtr[BRIDGE_FORMAT_LIN_RP] = &writeLINSegmentLevel;
  segPtr[BRIDGE_FORMAT_LIN_VG] = &writeLINSegmentLevel;
  segPtr[BRIDGE_FORMAT_LIN_TRN] = &writeLINSegmentLevel;
  segPtr[BRIDGE_FORMAT_PBN] = &writeDummySegmentLevel;
  segPtr[BRIDGE_FORMAT_RBN] = &writeRBNSegmentLevel;
  segPtr[BRIDGE_FORMAT_RBX] = &writeRBNSegmentLevel;
  segPtr[BRIDGE_FORMAT_TXT] = &writeTXTSegmentLevel;
  segPtr[BRIDGE_FORMAT_EML] = &writeDummySegmentLevel;
  segPtr[BRIDGE_FORMAT_REC] = &writeDummySegmentLevel;

  boardPtr[BRIDGE_FORMAT_LIN] = &writeLINBoardLevel;
  boardPtr[BRIDGE_FORMAT_LIN_RP] = &writeLINBoardLevel;
  boardPtr[BRIDGE_FORMAT_LIN_VG] = &writeLINBoardLevel;
  boardPtr[BRIDGE_FORMAT_LIN_TRN] = &writeLINBoardLevel;
  boardPtr[BRIDGE_FORMAT_PBN] = &writePBNBoardLevel;
  boardPtr[BRIDGE_FORMAT_RBN] = &writeRBNBoardLevel;
  boardPtr[BRIDGE_FORMAT_RBX] = &writeRBNBoardLevel;
  boardPtr[BRIDGE_FORMAT_TXT] = &writeTXTBoardLevel;
  boardPtr[BRIDGE_FORMAT_EML] = &writeEMLBoardLevel;
  boardPtr[BRIDGE_FORMAT_REC] = &writeRECBoardLevel;
}


static void writeHeader(
  const Format format,
  const Group& group,
  string& text)
{
  text = "";
  const string g = guessOriginalLine(group.name(), group.count());
  if (g == "")
    return;

  if (format == BRIDGE_FORMAT_RBX)
  {
    text += "%{RBX " + g + "}";
    text += "%{www.rpbridge.net Richard Pavlicek}";
  }
  else if (format == BRIDGE_FORMAT_LIN ||
      format == BRIDGE_FORMAT_LIN_TRN)
  {
    // Nothing.
  }
  else
  {
    text += "% " + FORMAT_EXTENSIONS[format] + " " + g + "\n";
    text += "% www.rpbridge.net Richard Pavlicek\n";
  }
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


static void writeFormattedFile(
  const string& fname,
  const Format format,
  const BoardOrder order,
  const Group& group,
  string& text)
{
  WriteInfo writeInfo;

  writeHeader(format, group, text);

  for (auto &segment: group)
  {
    if (segment.size() == 0)
      continue;

    (* segPtr[format])(text, segment, format);

    writeInfo.namesOld[0] = "";
    writeInfo.namesOld[1] = "";
    writeInfo.score1 = 0;
    writeInfo.score2 = 0;
    writeInfo.numBoards = segment.size();
    writeInfo.first = true;
    writeInfo.last = false;
    const unsigned lastRealNo = segment.lastRealBoardNumber();

    if (order == ORDER_COCO)
    {
      // c1, o1, c2, o2, ...
      for (auto &bpair: segment)
      {
        const Board& board = bpair.board;
        if (bpair.extNo == lastRealNo)
          writeInfo.last = true;

        writeInfo.bno = bpair.extNo;
        writeInfo.numInst = board.countAll();
        writeInfo.numInstActive = board.count();

        for (unsigned i = 0, j = writeInfo.numInst-1; 
            i < writeInfo.numInst; i++, j--)
        {
          writeInfo.instNo = j;
          writeInfo.ino = i;
          (* boardPtr[format])(text, segment, board, writeInfo, format);
          writeInfo.first = false;
        }
      }
    }
    else if (order == ORDER_OOCC)
    {
      // o1, o2, ..., c1, c2, ...
      for (unsigned i = 0; i < 2; i++)
      {
        for (auto &bpair: segment)
        {
          const Board& board = bpair.board;
          if (bpair.extNo == lastRealNo)
            writeInfo.last = true;

          writeInfo.bno = bpair.extNo;
          writeInfo.numInst = board.countAll();
          writeInfo.numInstActive = board.count();
          if (writeInfo.numInst > 2)
            THROW("Too many instances for OOCC output order");

          writeInfo.instNo = i;
          writeInfo.ino = i;
          (* boardPtr[format])(text, segment, board, writeInfo, format);
          writeInfo.first = false;
        }
      }
    }
    else
    {
      // o1, c1, o2, c2, ...
      for (auto &bpair: segment)
      {
        const Board& board = bpair.board;
        if (bpair.extNo == lastRealNo)
          writeInfo.last = true;

        writeInfo.bno = bpair.extNo;
        writeInfo.numInst = board.countAll();
        writeInfo.numInstActive = board.count();

        for (unsigned i = 0; i < writeInfo.numInst; i++)
        {
          writeInfo.instNo = i;
          writeInfo.ino = i;
          (* boardPtr[format])(text, segment, board, writeInfo, format);
          writeInfo.first = false;
        }
      }
    }
  }

  if (fname != "")
    writeFast(fname, text);
}


void dispatchWrite(
  const string& fname,
  const Format format,
  const BoardOrder order,
  const Group& group,
  string& text,
  ostream& flog)
{
  try
  {
    text = "";
    writeFormattedFile(fname, format, order, group, text);
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
  }
}

