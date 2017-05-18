/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINE_H
#define BRIDGE_REFLINE_H

#include <string>
#include "RefComment.h"

using namespace std;


enum RefCommentCategory
{
  REF_COMMENT_DELETE_LINE = 0,
  REF_COMMENT_INSERT_LINE = 1,
  REF_COMMENT_GENERAL = 2,
  REF_COMMENT_ERROR = 3
};


class RefLine
{
  private:

    enum EditType
    {
      EDIT_TAG_ONLY = 0,
      EDIT_TAG_FIELD = 1,
      EDIT_CHAR = 2,
      EDIT_MATCH = 3,
      EDIT_WORD = 4,
      EDIT_TYPE_SIZE = 5
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

    bool setFlag;
    string inputLine;
    Range range;
    FixType fix;
    Edit edit;
    RefComment comment;


    void setFixTables();
    void setRefTags();
    void setDispatch();
    void setCommentAction();
    void setCommentTag();
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

    void modifyRBNCommon(
      const string& line,
      string& s) const;

    bool modifyCommonRBX(
      const string& line,
      vector<string>& v,
      string& s,
      unsigned& pos) const;

    unsigned modifyCommonTXT(const string& line) const;
    unsigned modifyCommonWORD(const string& line) const;

    void modifyReplaceLIN(string& line) const;
    void modifyInsertLIN(string& line) const;
    void modifyDeleteLIN(string& line) const;

    void modifyReplacePBN(string& line) const;
    void modifyInsertPBN(string& line) const;
    void modifyDeletePBN(string& line) const;

    void modifyReplaceRBN(string& line) const;
    void modifyInsertRBN(string& line) const;
    void modifyDeleteRBN(string& line) const;

    void modifyReplaceRBX(string& line) const;
    void modifyInsertRBX(string& line) const;
    void modifyDeleteRBX(string& line) const;

    void modifyReplaceTXT(string& line) const;
    void modifyInsertTXT(string& line) const;
    void modifyDeleteTXT(string& line) const;

    void modifyReplaceWORD(string& line) const;
    void modifyInsertWORD(string& line) const;
    void modifyDeleteWORD(string& line) const;


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
    RefCommentCategory type() const;
    unsigned deletion() const;

    void modify(string& line) const;

    string str() const;
};

#endif
