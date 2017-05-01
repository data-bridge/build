/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CHUNK_H
#define BRIDGE_CHUNK_H

#include <string>
#include "bconst.h"

using namespace std;

enum ChunkRange
{
  CHUNK_HEADER = 0,
  CHUNK_BOARD = 1,
  CHUNK_ALL = 2,
  CHUNK_DVD = 3
};


class Chunk
{
  private:

    string chunk[BRIDGE_FORMAT_LABELS_SIZE];


  public:

    Chunk();

    ~Chunk();

    void reset();
    void reset(const ChunkRange range);

    bool isSet(const Label label) const;
    bool isEmpty(const Label label) const;

    void set(
      const Label label,
      const string& value);

    string get(const Label label) const;

    void getRange(Counts& counts) const;

    void guessDealerAndVul(
      const unsigned bno,
      const Format format);

    void guessDealerAndVul(const Format format);

    void copyFrom(
      const Chunk& chunk2,
      const Label label);

    void copyFrom(
      const Chunk& chunk2,
      const ChunkRange range);

    bool differsFrom(
      const Chunk& chunk2,
      const Label label);

    bool differsFrom(
      const Chunk& chunk2,
      const ChunkRange range);

    string str() const;
    string str(const Label label) const;
};

#endif

