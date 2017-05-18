/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFEDIT_H
#define BRIDGE_REFEDIT_H

#include <string>
#include "refconst.h"

using namespace std;


class RefEdit
{
  private:

    unsigned tagno;
    bool reverseFlag; // tag is counted from the back, starting from -1
    string tagVal; // "rs", for example
    unsigned fieldno;
    unsigned tagcount;
    unsigned charno;
    string wasVal;
    string isVal;

    void setTables();

    void modifyFail(
      const string& line,
      const string& reason) const;

    void modifyReplaceGen(string& line) const;
    void modifyInsertGen(string& line) const;
    void modifyDeleteGen(string& line) const;

    void modifyLINCommon(
      const string& line,
      const bool insertFlag,
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

    RefEdit();

    ~RefEdit();

    void reset();

    void setTagNumber(const unsigned noIn);
    void setReverse();
    void setTag(const string& tagIn);
    void setFieldNumber(const unsigned fieldnoIn);
    void setTagCount(const unsigned countIn);
    void setCharNumber(const unsigned countIn);
    void setWas(const string& wasIn);
    void setIs(const string& isIn);

    string tag() const;
    string is() const;
    string was() const;

    void modify(
      string& line,
      const ActionType act) const;

    string str() const;
};

#endif
