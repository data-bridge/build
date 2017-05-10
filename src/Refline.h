/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINE_H
#define BRIDGE_REFLINE_H

#include <string>

#include "refcodes.h"

using namespace std;


enum FixType
{
  BRIDGE_REF_REPLACE_GEN = 0,
  BRIDGE_REF_INSERT_GEN = 1,
  BRIDGE_REF_DELETE_GEN = 2,

  BRIDGE_REF_REPLACE_LIN = 3,
  BRIDGE_REF_INSERT_LIN = 4,
  BRIDGE_REF_DELETE_LIN = 5,

  BRIDGE_REF_REPLACE_PBN = 6,
  BRIDGE_REF_INSERT_PBN = 7,
  BRIDGE_REF_DELETE_PBN = 8,

  BRIDGE_REF_REPLACE_RBN = 9,
  BRIDGE_REF_INSERT_RBN = 10,
  BRIDGE_REF_DELETE_RBN = 11,

  BRIDGE_REF_REPLACE_TXT = 12,
  BRIDGE_REF_INSERT_TXT = 13,
  BRIDGE_REF_DELETE_TXT = 14,

  BRIDGE_REF_FIX_SIZE = 15
};


class Refline
{
  private:

    enum EditType
    {
      EDIT_TAG_ONLY = 0,
      EDIT_TAG_FIELD = 1,
      EDIT_CHAR = 2,
      EDIT_MATCH = 3,
      EDIT_TYPE_SIZE = 4
    };

    struct Range
    {
      unsigned lno; // First line is 1
      unsigned lcount; // 1 unless multi-line delete
    };

    struct Edit
    {
      EditType type;
      unsigned tagno;
      bool reverseFlag; // tag is counted from the back, starting from -1
      string tag; // "rs", for example
      unsigned fieldno;
      unsigned tagcount;
      unsigned charno;
      string was;
      string is;
    };

    struct Comment
    {
      bool setFlag;
      RefErrorsType category; // "ERR_LIN_VG_FIRST", for example
      unsigned count1, count2, count3;
    };

    bool setFlag;
    Range range;
    FixType fix;
    Edit edit;
    Comment comment;


    void setTables();

    bool isSpecial(const string& word) const;

    void parseRange(
      const string& refFile,
      const string& line,
      const string& range,
      const unsigned start,
      const unsigned end);

    FixType parseAction(
      const string& refFile,
      const string& line,
      const string& action);

    void parseComment(
      const string& refFile,
      const string& line,
      const unsigned start,
      unsigned& end);

    void parseFlexibleNumber(
      const string& refName,
      const string& field);

    string unquote(const string& entry) const;

    void commonCheck(
      const string& refName,
      const string& quote,
      const string& tag) const;

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

    void modifyFail(
      const string& line,
      const string& reason) const;

    void modifyReplaceGen(string& line) const;
    void modifyInsertGen(string& line) const;
    void modifyDeleteGen(string& line) const;

    void modifyLINCommon(
      const string& line,
      unsigned& start,
      vector<string>& v,
      vector<string>& f,
      bool& endsOnPipe) const;

    void modifyReplaceLIN(string& line) const;
    void modifyInsertLIN(string& line) const;
    void modifyDeleteLIN(string& line) const;

    void modifyReplacePBN(string& line) const;
    void modifyInsertPBN(string& line) const;
    void modifyDeletePBN(string& line) const;

    void modifyReplaceRBN(string& line) const;
    void modifyInsertRBN(string& line) const;
    void modifyDeleteRBN(string& line) const;

    void modifyReplaceTXT(string& line) const;
    void modifyInsertTXT(string& line) const;
    void modifyDeleteTXT(string& line) const;


  public:

    Refline();

    ~Refline();

    void reset();

    bool parse(
      const string& refFile,
      const string& line);

    unsigned lineno() const;

    bool isSet() const;
    bool isCommented() const;
    bool isUncommented() const;

    FixType type() const;

    void modify(string& line) const;

    string str() const;
};

#endif
