/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BUFFER_H
#define BRIDGE_BUFFER_H

#include <string>
#include <vector>

#include "valint.h"
#include "bconst.h"

using namespace std;


enum RefUse
{
  BRIDGE_REF_ALL,
  BRIDGE_REF_ONLY_PARTIAL,
  BRIDGE_REF_ONLY_NONPARTIAL
};


class Buffer
{
  private:

    string fileName;
    vector<LineData> lines;
    unsigned len;
    unsigned current;
    Format format;
    unsigned posLIN;
    unsigned posRBX;


    void readBinaryFile(const string& fname);

    bool isLIN(LineData& ld);
    bool isPBN(LineData& ld);
    bool isRBN(LineData& ld);
    bool isRBX(LineData& ld);

    void classify(LineData& ld);

    unsigned getInternalNumber(const unsigned no) const;

    bool nextLIN(
      LineData& vside,
      const bool skipChat = true);
    void nextRBX(LineData& vside);


  public:

    Buffer();

    ~Buffer();

    void reset();

    void rewind();

    bool read(
      const string& fname,
      const Format format);

    bool split(
      const string& st,
      const Format format);

    bool fix(
      const string& fname,
      const RefUse use = BRIDGE_REF_ALL);

    bool advance();

    bool next(
      LineData& lineData,
      const bool skipChat = true);

    bool previous(LineData& lineData);

    unsigned previousHeaderStart() const;

    unsigned firstRS() const;

    string getLine(const unsigned no) const;

    string name() const;

    int peek();

    void print() const;
};

#endif
