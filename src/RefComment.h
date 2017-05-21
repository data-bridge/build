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
  CommentType category; // "ERR_LIN_VG_FIRST", for example
  unsigned count1, count2, count3;
  string quote;

  void setTables();
  void setCommentMap();
  void setRefTag();
  void setCommentTable();
  void setActionTable();
  void setTagTable();

  RefTag str2ref(const string& refstr) const;


  public:

    RefComment();

    ~RefComment();

    void reset();

    void parse(
      const string& refFile,
      const string& line,
      const unsigned start,
      unsigned& end);

    void checkAction(const ActionType action) const;

    string comment2str(const CommentType c) const;

    bool isTag(const string& tag) const;

    void checkTag(const string& tag) const;

    RefCountType countType() const;

    CommentType commentType() const;

    bool isCommented() const;

    bool isUncommented() const;

    void getEntry(
      CommentType& cat,
      RefEntry &re) const;

    string str() const;

    string strComment() const;
};

#endif
