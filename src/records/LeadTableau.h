/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LEADTABLEAU_H
#define BRIDGE_LEADTABLEAU_H

#include <vector>
#include <array>
#include <list>
#include <string>

#include "../analysis/LeadTriple.h"
#include "../include/bridge.h"

using namespace std;


struct LeadGroup
{
  unsigned tricks;
  unsigned lastSeenRank;
  unsigned numRanks;
  unsigned numCards;
  string text;

  void clear()
  {
    tricks = 0;
    lastSeenRank = 99;
    numRanks = 0;
    numCards = 0;
    text = "";
  };
};

struct SuitProfile
{
  unsigned voidFlag;
  unsigned constantFlag;
  unsigned value;
};

typedef array<list<LeadGroup>, BRIDGE_SUITS> SuitGroupElem;
typedef array<array<list<unsigned>, BRIDGE_TRICKS+1>, BRIDGE_SUITS> 
  SuitIndicesElem;


class LeadTableau
{
  private:

    unsigned setNum;

    bool cardsKnownFlag;

    string cardStrings[BRIDGE_SUITS];

    LeadTriple table[BRIDGE_DENOMS][BRIDGE_PLAYERS][BRIDGE_TRICKS];

    void setCards(const string cards[BRIDGE_SUITS]);

    void makeSuitGroups(
      array<array<SuitGroupElem, BRIDGE_PLAYERS>, BRIDGE_DENOMS>&
          suitGroups,
      array<array<SuitIndicesElem, BRIDGE_PLAYERS>, BRIDGE_DENOMS>&
          suitIndices) const;

    int lookupSuit(const char suitChar) const;

    int lookupRank(const char rankChar) const;

    void setCard(
      LeadTriple tmp[BRIDGE_SUITS][BRIDGE_TRICKS],
      bool seen[BRIDGE_SUITS][BRIDGE_TRICKS],
      const int suit,
      const int rank,
      const int tricks);
  
    void flattenTable(
      const LeadTriple tmp[BRIDGE_SUITS][BRIDGE_TRICKS],
      const bool seen[BRIDGE_SUITS][BRIDGE_TRICKS],
      LeadTriple dst[BRIDGE_TRICKS]);

    void setRBNline(
      const size_t strain,
      const size_t leader,
      const string& text);

    void setRBN(const string& text);

    string strElementTXT(
      const SuitGroupElem& leadGroups,
      const SuitIndicesElem& trickIndices) const;

    string strRBN() const; // Not really

    string strTXT() const;

  public:

    LeadTableau();

    void reset();

    void set(
      const string& text,
      const Format format);

    bool isComplete() const;

    void setDDS(
      const array<array<array<LeadTriple, BRIDGE_TRICKS>, 
        BRIDGE_PLAYERS>, BRIDGE_DENOMS> res);

    bool operator == (const LeadTableau& tableau2) const;

    bool operator != (const LeadTableau& tableau2) const;

    string str(const Format format = BRIDGE_FORMAT_LIN) const;
};

#endif

