/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFCOMMENT_H
#define BRIDGE_REFCOMMENT_H

#include <string>
#include "refconst.h"


using namespace std;


class RefComment
{
  private:

  bool setFlag;
  string fileName;
  RefErrorsType category; // "ERR_LIN_VG_FIRST", for example
  unsigned count1, count2, count3;
  string quote;

  void setTables();
  void setCommentMap();
  void setRefTag();
  void setActionTable();
  void setTagTable();


  public:

    RefComment();

    ~RefComment();

    void reset();

    void parse(
      const string& refFile,
      const string& line,
      const unsigned start,
      unsigned& end);

    void checkAction(const FixType action) const;

    void checkTag(const string& tag) const;

    bool isCommented() const;

    bool isUncommented() const;

    string str() const;
};

#endif
