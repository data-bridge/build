/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "Chunk.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"

//
// Modulo 4, so West for Board "0" (4, 8, ...) etc.

static const Player BOARD_TO_DEALER[4] = 
{
  BRIDGE_WEST, BRIDGE_NORTH, BRIDGE_EAST, BRIDGE_SOUTH
};

// Modulo 16, so EW for Board "0" (16, 32, ...) etc.

static const Vul BOARD_TO_VUL[16] =
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

const vector<Label> PBNFields =
{
  BRIDGE_FORMAT_TITLE,
  BRIDGE_FORMAT_EVENT,
  BRIDGE_FORMAT_DATE,
  BRIDGE_FORMAT_LOCATION,
  BRIDGE_FORMAT_EVENT,
  BRIDGE_FORMAT_SESSION,
  BRIDGE_FORMAT_SCORING,
  BRIDGE_FORMAT_HOMETEAM,
  BRIDGE_FORMAT_VISITTEAM,
  BRIDGE_FORMAT_WEST,
  BRIDGE_FORMAT_NORTH,
  BRIDGE_FORMAT_EAST,
  BRIDGE_FORMAT_SOUTH,
  BRIDGE_FORMAT_BOARD_NO,
  BRIDGE_FORMAT_ROOM
};


Chunk::Chunk()
{
  Chunk::reset();
}


Chunk::~Chunk()
{
}


void Chunk::reset()
{
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    chunk[i].reserve(128);
    chunk[i] = "";
    lineno[i] = BIGNUM;
  }
}


void Chunk::reset(const ChunkRange range)
{
  unsigned end;
  if (range == CHUNK_HEADER)
    end = BRIDGE_FORMAT_RESULTS_LIST;
  else if (range == CHUNK_BOARD)
    end = BRIDGE_FORMAT_BOARD_NO;
  else if (range == CHUNK_ALL)
    end = BRIDGE_FORMAT_LABELS_SIZE;
  else
    THROW("Bad range");

  for (unsigned i = 0; i < end; i++)
  {
    chunk[i] = "";
    lineno[i] = BIGNUM;
  }
}  


bool Chunk::isSet(const Label label) const
{
  return (chunk[label] != "");
}


bool Chunk::isEmpty(const Label label) const
{
  return (chunk[label] == "");
}


bool Chunk::seemsEmpty() const
{
  return (chunk[BRIDGE_FORMAT_BOARD_NO] == "" &&
      chunk[BRIDGE_FORMAT_RESULT] == "" &&
      chunk[BRIDGE_FORMAT_AUCTION] == "");
}


void Chunk::set(
  const Label label,
  const string& value,
  const unsigned lno)
{
  chunk[label] = value;
  lineno[label] = lno;
}


void Chunk::append(
  const Label label,
  const string& value)
{
  chunk[label] += value;
}


string Chunk::get(const Label label) const
{
  return chunk[label];
}


string Chunk::get(const unsigned label) const
{
  if (label >= BRIDGE_FORMAT_LABELS_SIZE)
    THROW("label out of range");
  return chunk[label];
}


void Chunk::getCounts(
  const Format format,
  Counts& counts) const
{
  string bno = chunk[BRIDGE_FORMAT_BOARD_NO];

  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    // Reuse the value in counts.bno
    if (bno == "")
      return;

    const string r = bno.substr(0, 1);
    bno = bno.substr(1);

    if (r == "o")
      counts.openFlag = true;
    else if (r == "c")
      counts.openFlag = false;
    else
      THROW("Not a room");
  }
  else if (format == BRIDGE_FORMAT_PBN)
  {
    const string r = chunk[BRIDGE_FORMAT_ROOM];
    if (r == "" || r == "Open")
      counts.openFlag = true;
    else if (r == "Closed")
      counts.openFlag = false;
    else
      THROW("Unknown room: " + r);
  }
  else if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
  {
    const string sn = chunk[BRIDGE_FORMAT_PLAYERS];
    const unsigned sl = static_cast<unsigned>(sn.length());
    if (sn != "" && sl >= 2)
    {
      const string r = sn.substr(sl-2);
      if (r == ":O")
        counts.openFlag = true;
      else if (r == ":C")
        counts.openFlag = false;
      else
        counts.openFlag = ! counts.openFlag;
    }
    else
      counts.openFlag = ! counts.openFlag;
  }
  else if (format == BRIDGE_FORMAT_TXT ||
      format == BRIDGE_FORMAT_EML ||
      format == BRIDGE_FORMAT_REC)
  {
    counts.openFlag = ! counts.openFlag;
  }
  else
    THROW("Bad format");

  if (bno != "")
  {
    // If bno is empty, reuse the value in counts.bno for now (will fail)
    if (! str2upos(bno, counts.bno))
      THROW("Not a board number");
  } 
}


void Chunk::guessDealerAndVul(
  const unsigned bno,
  const Format format)
{
  if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
  {
    chunk[BRIDGE_FORMAT_DEALER] = 
      PLAYER_NAMES_SHORT[BOARD_TO_DEALER[bno % 4]];
    chunk[BRIDGE_FORMAT_VULNERABLE] = VUL_NAMES_PBN[BOARD_TO_VUL[bno % 16]];
    return;
  }

  const string st = chunk[BRIDGE_FORMAT_BOARD_NO];
  unsigned u;
  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    if (st.length() <= 1 || ! str2upos(st.substr(1), u))
      return;

    chunk[BRIDGE_FORMAT_DEALER] = 
      to_string(PLAYER_DDS_TO_LIN_DEALER[BOARD_TO_DEALER[u % 4]]);
    chunk[BRIDGE_FORMAT_VULNERABLE] = 
      VUL_NAMES_LIN[BOARD_TO_VUL[u % 16]];
  }
  else
  {
    THROW("Should not be happening");
  }
}


void Chunk::copyFrom(
  const Chunk& chunk2,
  const Label label)
{
  chunk[label] = chunk2.chunk[label];
  lineno[label] = chunk2.lineno[label];
}


void Chunk::copyFrom(
  const Chunk& chunk2,
  const ChunkRange range)
{
  unsigned end;
  if (range == CHUNK_HEADER)
    end = BRIDGE_FORMAT_RESULTS_LIST;
  else if (range == CHUNK_BOARD)
    end = BRIDGE_FORMAT_BOARD_NO;
  else if (range == CHUNK_ALL)
    end = BRIDGE_FORMAT_LABELS_SIZE;
  else if (range == CHUNK_DVD)
  {
    Chunk::copyFrom(chunk2, BRIDGE_FORMAT_VULNERABLE);
    Chunk::copyFrom(chunk2, BRIDGE_FORMAT_DEALER);
    Chunk::copyFrom(chunk2, BRIDGE_FORMAT_DEAL);
    return;
  }
  else if (range == CHUNK_DEAL)
  {
    Chunk::copyFrom(chunk2, BRIDGE_FORMAT_DEAL);
    return;
  }
  else if (range == CHUNK_PBN)
  {
    for (auto &i: PBNFields)
      Chunk::copyFrom(chunk2, i);
    return;
  }
  else if (range == CHUNK_PBN_SOFTLY)
  {
    for (auto &i: PBNFields)
    {
      if (chunk[i] == "")
        Chunk::copyFrom(chunk2, i);
    }
    return;
  }
  else
    THROW("Bad range");

  for (unsigned i = 0; i < end; i++)
    Chunk::copyFrom(chunk2, static_cast<Label>(i));
}


bool Chunk::differsFrom(
  const Chunk& chunk2,
  const Label label) const
{
  return (chunk[label] != chunk2.chunk[label]);
}


bool Chunk::differsFrom(
  const Chunk& chunk2,
  const ChunkRange range) const
{
  unsigned end;
  if (range == CHUNK_HEADER)
    end = BRIDGE_FORMAT_RESULTS_LIST;
  else if (range == CHUNK_BOARD)
    end = BRIDGE_FORMAT_BOARD_NO;
  else if (range == CHUNK_ALL)
    end = BRIDGE_FORMAT_LABELS_SIZE;
  else
    THROW("Bad range");

  for (unsigned i = 0; i < end; i++)
  {
    if (chunk[i] != chunk2.chunk[i] && chunk[i] != "")
      return true;
  }
  return false;
}


string Chunk::str(const Label label) const
{
  stringstream ss;
  ss << "label " << LABEL_NAMES[label] << " (" << label << "), '" <<
    chunk[label] << "'" << endl << endl;
  return ss.str();
}


string Chunk::str() const
{
  stringstream ss;
  ss << endl;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    if (chunk[i] != "")
    {
      ss << setw(15) << LABEL_NAMES[i] <<
          " (" << setw(2) << i << "), '" <<
          chunk[i] << "'" << endl;
    }
  }
  return ss.str();
}


string Chunk::strRange() const
{
  unsigned lo = BIGNUM;
  unsigned hi = 0;
  for (unsigned i = 0; i < BRIDGE_FORMAT_LABELS_SIZE; i++)
  {
    if (lineno[i] == BIGNUM)
      continue;
    if (lineno[i] > hi)
      hi = lineno[i];
    if (lineno[i] < lo)
      lo = lineno[i];
  }

  if (lo == hi)
    return "Line number:  " + to_string(lo) + "\n\n";
  else
    return "Line numbers: " + to_string(lo) + " to " + 
      to_string(hi) + "\n\n";
}

