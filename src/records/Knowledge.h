/* 
   Part of BridgeData.

   Copyright (C) 2016-26 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_KNOWLEDGE_H
#define BRIDGE_KNOWLEDGE_H

#include <array>
#include <string>

#include "../include/bridge.h"

static constexpr char HEX[] = "0123456789ABCDEF";

using namespace std;


struct SuitKnowledge
{
  unsigned min;
  unsigned max;
  unsigned known;

  string str() const
  {
    return {HEX[min], HEX[max], HEX[known] };
  };
};

class Knowledge
{
  // What we know about a single player in the context
  // of one of the other two players (dummy not counting).

  private:

    array<SuitKnowledge, BRIDGE_SUITS> suits;
    unsigned unknown;
    unsigned minSum;

  public:

    void init(const array<unsigned, BRIDGE_SUITS>& known);

    void increaseKnown(const unsigned suit);

    void increaseMinBy(
      Knowledge& other,
      const unsigned suit,
      const unsigned delta);

    void fixAtMin(const unsigned leadToTrick);

    void fixAtMax(const unsigned leadToTrick);

    void reduceMax(Knowledge& other);

    void adjustToCompleteness(Knowledge& other);

    bool distKnown() const;

    bool dealKnown() const;

    string strHeader() const;

    string str() const;
};

#endif
