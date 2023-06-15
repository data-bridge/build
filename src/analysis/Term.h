/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TERM_H
#define BRIDGE_TERM_H

using namespace std;


enum TermCategory
{
  TERM_SUIT = 0,
  TERM_DIST = 1,
  TERM_COMP = 2,
  TERM_SIZE = 3
};

enum TermComparator
{
  COMPARATOR_LE = 0,
  COMPARATOR_LT = 1,
  COMPARATOR_EQ = 2,
  COMPARATOR_GT = 3,
  COMPARATOR_GE = 4,
  COMPARATOR_IN = 5,
  COMPARATOR_NE = 6,
  COMPARATOR_SIZE = 7
};


class Term
{
  private:

    bool setFlag;
    TermCategory cat;
    unsigned pno;
    unsigned suitVal;
    TermComparator compVal;
    int limit1Val, limit2Val;


  public:

    Term();

    ~Term();

    void reset();

    void set(const string& text);

    TermCategory category() const;

    unsigned paramNo() const;

    unsigned suit() const;

    TermComparator comparator() const;

    int limit() const;
    int limit1() const;
    int limit2() const;

    string str() const;

};

#endif
