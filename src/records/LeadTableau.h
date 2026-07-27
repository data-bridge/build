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



class LeadTableau
{
  private:

    unsigned setNum;

    LeadTriple table[BRIDGE_DENOMS][BRIDGE_PLAYERS][BRIDGE_TRICKS];

    // TODO Not yet used.
    string tableStr[BRIDGE_DENOMS][BRIDGE_PLAYERS][BRIDGE_TRICKS];

    array<array<array<list<LeadGroup>, BRIDGE_SUITS>, 
      BRIDGE_PLAYERS>, BRIDGE_DENOMS> suitGroups;

    
    void gradeTricks();

  public:

    LeadTableau();

    void reset();

    bool isComplete() const;

    bool set(
      const string& text,
      const Format format);
    
    void setDDS(
      const array<array<array<LeadTriple, BRIDGE_TRICKS>, 
        BRIDGE_PLAYERS>, BRIDGE_DENOMS> res);

    bool operator == (const LeadTableau& tableau2) const;

    bool operator != (const LeadTableau& tableau2) const;

    string str(const Format format = BRIDGE_FORMAT_LIN) const;
};

#endif

