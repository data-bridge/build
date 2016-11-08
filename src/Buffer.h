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


    void readBinaryFile(const string& fname);

    bool isLIN(LineData& ld);
    bool isPBN(LineData& ld);
    bool isRBN(LineData& ld);
    bool isRBX(LineData& ld);

    void classify(
      LineData& ld,
      const Format format);

    void readRefFix(
      const string& fname,
      vector<RefFix>& refFix);


  public:

    Buffer();

    ~Buffer();

    void reset();

    bool read(
      const string& fname,
      const Format format);

    bool fix(
      const string& fname,
      const Format format);

    bool advance();

    bool next(LineData& lineData);

    string getLine() const;

    unsigned getNumber() const;

    unsigned getLength() const;

    LineType getType() const;

    string getLabel() const;

    string getValue() const;

    void print() const;
};

#endif
