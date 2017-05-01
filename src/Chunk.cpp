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
    chunk[i] = "";
}  


bool Chunk::isSet(const Label label) const
{
  return (chunk[label] != "");
}


bool Chunk::isEmpty(const Label label) const
{
  return (chunk[label] == "");
}


void Chunk::set(
  const Label label,
  const string& value)
{
  chunk[label] = value;
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


void Chunk::getRange(Counts& counts) const
{
  const string title = chunk[BRIDGE_FORMAT_TITLE];
  if (title == "")
    return;

  if (count(title.begin(), title.end(), ',') != 8)
    THROW("LIN vg need exactly 8 commas");

  vector<string> v(9);
  v.clear();
  tokenize(title, v, ",");
  
  if (v[3] == "" || v[4] == "")
    return;
  
  if (! str2upos(v[3], counts.bExtmin))
    THROW("Not a board number");
  if (! str2upos(v[4], counts.bExtmax))
    THROW("Not a board number");

  const string res = chunk[BRIDGE_FORMAT_RESULTS_LIST];
  if (res == "")
    return;

  const unsigned l = static_cast<unsigned>(res.length());
  unsigned p = 0;
  while (p+1 < l && res.substr(p, 2) == ",,")
  {
    p += 2;
    counts.bExtmin++;
  }

  if (counts.bExtmax < counts.bExtmin)
    THROW("Bad board range");
}


void Chunk::guessDealerAndVul(
  const unsigned bno,
  const Format format)
{
  // This is not quite fool-proof, as there are LIN files where
  // the board numbers don't match...

  if (format == BRIDGE_FORMAT_RBN ||
      format == BRIDGE_FORMAT_RBX)
  {
    chunk[BRIDGE_FORMAT_DEALER] = PLAYER_NAMES_SHORT[BOARD_TO_DEALER[bno % 4]];
    chunk[BRIDGE_FORMAT_VULNERABLE] = VUL_NAMES_PBN[BOARD_TO_VUL[bno % 16]];
  }
}


void Chunk::guessDealerAndVul(const Format format)
{
  const string st = chunk[BRIDGE_FORMAT_BOARD_NO];
  unsigned u;
  if (format == BRIDGE_FORMAT_LIN || 
      format == BRIDGE_FORMAT_LIN_VG ||
      format == BRIDGE_FORMAT_LIN_TRN)
  {
    if (st.length() <= 1 || ! str2upos(st.substr(1), u))
      return;

    chunk[BRIDGE_FORMAT_DEALER] = 
      STR(PLAYER_DDS_TO_LIN_DEALER[BOARD_TO_DEALER[u % 4]]);
    chunk[BRIDGE_FORMAT_VULNERABLE] = 
      VUL_NAMES_LIN[BOARD_TO_VUL[u % 16]];
  }
  else
  {
    if (! str2upos(st, u))
      return;
    Chunk::guessDealerAndVul(u, format);
  }
}


void Chunk::copyFrom(
  const Chunk& chunk2,
  const Label label)
{
  chunk[label] = chunk2.chunk[label];
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
    chunk[BRIDGE_FORMAT_VULNERABLE] = chunk2.chunk[BRIDGE_FORMAT_VULNERABLE];
    chunk[BRIDGE_FORMAT_DEALER] = chunk2.chunk[BRIDGE_FORMAT_DEALER];
    chunk[BRIDGE_FORMAT_DEAL] = chunk2.chunk[BRIDGE_FORMAT_DEAL];
    return;
  }
  else
    THROW("Bad range");

  for (unsigned i = 0; i < end; i++)
    chunk[i] = chunk2.chunk[i];
}


bool Chunk::differsFrom(
  const Chunk& chunk2,
  const Label label)
{
  return (chunk[label] != chunk2.chunk[label]);
}


bool Chunk::differsFrom(
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

