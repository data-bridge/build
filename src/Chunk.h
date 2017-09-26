/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CHUNK_H
#define BRIDGE_CHUNK_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#pragma warning(pop)

#include "bconst.h"

using namespace std;

enum ChunkRange
{
  CHUNK_HEADER = 0,
  CHUNK_BOARD = 1,
  CHUNK_DEAL = 2,
  CHUNK_ALL = 3,
  CHUNK_DVD = 4,
  CHUNK_PBN = 5,
  CHUNK_PBN_SOFTLY = 6
};


class Chunk
{
  private:

    string chunk[BRIDGE_FORMAT_LABELS_SIZE];
    unsigned lineno[BRIDGE_FORMAT_LABELS_SIZE];


  public:

    Chunk();

    ~Chunk();

    void reset();
    void reset(const ChunkRange range);

    bool isSet(const Label label) const;
    bool isEmpty(const Label label) const;

    bool seemsEmpty() const;

    void set(
      const Label label,
      const string& value,
      const unsigned lno = BIGNUM);

    void append(
      const Label label,
      const string& value);

    string get(const Label label) const;
    string get(const unsigned label) const;

    void getCounts(
      const Format format,
      Counts& counts) const;

    void guessDealerAndVul(
      const unsigned bno,
      const Format format);

    void copyFrom(
      const Chunk& chunk2,
      const Label label);

    void copyFrom(
      const Chunk& chunk2,
      const ChunkRange range);

    bool differsFrom(
      const Chunk& chunk2,
      const Label label) const;

    bool differsFrom(
      const Chunk& chunk2,
      const ChunkRange range) const;

    string str() const;
    string str(const Label label) const;

    string strRange() const;
};

#endif

