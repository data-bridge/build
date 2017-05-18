/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINE_H
#define BRIDGE_REFLINE_H

#include <string>
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
      const unsigned start,
      const unsigned end);

    void parseFlexibleNumber(
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


  public:

    RefLine();

    ~RefLine();

    void reset();

    bool parse(
      const string& refFile,
      const string& line);

    unsigned lineno() const;

    bool isSet() const;
    string line() const;
    string tag() const;
    string is() const;
    string was() const;
    bool isCommented() const;
    bool isUncommented() const;
    ActionCategory type() const;
    unsigned deletion() const;

    void modify(string& line) const;

    string str() const;
};

#endif
