/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_BUFFER_H
#define BRIDGE_BUFFER_H

#include <string>
#include <vector>

#include "valint.h"
#include "bconst.h"

using namespace std;


class Buffer
{
  private:

    enum FixType
    {
      BRIDGE_REF_INSERT = 0,
      BRIDGE_REF_REPLACE = 1,
      BRIDGE_REF_DELETE = 2,
    };

    struct RefFix
    {
      unsigned lno; // First line is 1
      FixType type;
      string value;
      unsigned count;
    };

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

    void readRefFix(
      const string& fname,
      vector<RefFix>& refFix);

    bool nextLIN(LineData& vside);
    void nextRBX(LineData& vside);


  public:

    Buffer();

    ~Buffer();

    void reset();

    bool read(
      const string& fname,
      const Format format);

    bool split(
      const string& st,
      const Format format);

    bool fix(const string& fname);

    bool advance();

    bool next(LineData& lineData);

    bool previous(LineData& lineData);

    unsigned previousHeaderStart() const;

    int peek() const;

    void print() const;
};

#endif
