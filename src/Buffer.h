/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BUFFER_H
#define BRIDGE_BUFFER_H

#include <string>
#include <vector>
#include <list>

#include "bconst.h"

class RefLines;
class RefLine;

using namespace std;


class Buffer
{
  private:

    string fileName;
    vector<LineData> lines;
    unsigned len;
    unsigned lenOrig;
    unsigned current;
    Format format;
    unsigned posLIN;
    unsigned posRBX;

    Buffer * embeddedBuf;
    string embeddedName;


    void readBinaryFile(const string& fname);

    bool isLIN(LineData& ld);
    bool isPBN(LineData& ld);
    bool isRBN(LineData& ld);
    bool isRBX(LineData& ld);

    void cacheEmbedded(const string& embeddedNameIn);

    void getEmbeddedData(
      const RefLine& rl,
      list<LineData>& lnew);

    bool fix(const RefLines& refLines);

    void classify(LineData& ld);

    unsigned getInternalNumber(const unsigned no) const;

    bool extendLINValue(LineData& vside);
    void advanceLINPast(size_t pos);
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
      const Format format,
      RefLines& refLines);

    void readForce(
      const string& fname,
      const Format format);

    unsigned lengthOrig() const;

    bool split(
      const string& st,
      const Format format);

    bool fix(
      const string& fname,
      RefLines& refLines);

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
