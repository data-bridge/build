/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINE_H
#define BRIDGE_REFLINE_H

#include <string>
#include <map>

#include "RefEdit.h"
#include "RefComment.h"
#include "RefAction.h"

using namespace std;


class RefLine
{
  private:

    struct Range
    {
      unsigned lno; // First line is 1
      unsigned lcount; // 1 unless multi-line delete
    };

    bool setFlag;
    string inputLine;
    Range range;
    RefAction action;
    RefEdit edit;
    RefComment comment;

    string fnameEmbed;
    Range rangeEmbed;


    void setTables();

    bool isSpecial(const string& word) const;

    unsigned parseUpos(
      const string& refFile,
      const string& line,
      const string& str) const;

    void parseRange(
      const string& refFile,
      const string& line,
      const string& range,
      Range& rangeIn);

    void parseFlexibleNumber(
      const string& refName,
      const string& field);

    void parseFrom(
      const string& refName,
      const string& field);

    string unquote(const string& entry) const;

    void parseReplaceGen(const string& refName, const string& line);
    void parseInsertGen(const string& refName, const string& line);
    void parseDeleteGen(const string& refName, const string& line);

    void parseReplaceLIN(const string& refName, const string& line);
    void parseInsertLIN(const string& refName, const string& line);
    void parseDeleteLIN(const string& refName, const string& line);

    void parseReplacePBN(const string& refName, const string& line);
    void parseInsertPBN(const string& refName, const string& line);
    void parseDeletePBN(const string& refName, const string& line);

    void parseReplaceRBN(const string& refName, const string& line);
    void parseInsertRBN(const string& refName, const string& line);
    void parseDeleteRBN(const string& refName, const string& line);

    void parseReplaceTXT(const string& refName, const string& line);
    void parseInsertTXT(const string& refName, const string& line);
    void parseDeleteTXT(const string& refName, const string& line);

    void parseReplaceWORD(const string& refName, const string& line);
    void parseInsertWORD(const string& refName, const string& line);
    void parseDeleteWORD(const string& refName, const string& line);

    void parseReplaceFrom(const string& refName, const string& line);
    void parseInsertFrom(const string& refName, const string& line);

    void parseDeleteEML(const string& refName, const string& line);

    void checkEntries(
      const RefEntry& re,
      const RefEntry& ractual) const;

    void checkCounts() const;

    void countHandsLIN(
      const string& line,
      vector<unsigned>& seen) const;

    void countHandsPBN(
      const string& line,
      unsigned &h,
      unsigned &b) const;

    void countHandsRBN(
      const string& line,
      unsigned &h,
      unsigned &b) const;

    void countHandsTXT(
      const string& line,
      unsigned &h) const;

    void countHandsEML(
      const string& line,
      unsigned &h) const;

    void countHandsREC(
      const string& line,
      unsigned &h) const;

    void countVector(
      const vector<unsigned>& seen,
      unsigned &h,
      unsigned &b) const;

    void checkMultiLineCounts(const vector<string>& lines) const;


  public:

    RefLine();

    ~RefLine();

    void reset();

    bool parse(
      const string& refFile,
      const string& line);

    unsigned lineno() const;
    unsigned linenoEmbed() const;

    bool isSet() const;
    string line() const;
    string tag() const;
    string is() const;
    string was() const;
    string embeddedRef() const;
    bool isCommented() const;
    bool isUncommented() const;
    ActionCategory type() const;
    unsigned rangeCount() const;
    unsigned rangeEmbedCount() const;

    void modify(string& line) const;
    void modify(vector<string>& lines) const;

    void checkMultiLine(const vector<string>& lines) const;

    void countHands(
      const vector<string>& lines,
      const Format format,
      unsigned &h,
      unsigned &b) const;

    void checkHeader(
      const RefEntry& re,
      map<string, vector<RefEntry>>& cumulCount) const;

    void getEntry(
      CommentType& cat,
      RefEntry& ref) const;

    string str() const;
};

#endif
