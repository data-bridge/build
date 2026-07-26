/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_LEADTABLEAU_H
#define BRIDGE_LEADTABLEAU_H

#include <string>
#include <list>

#include "../analysis/LeadTriple.h"
#include "../include/bridge.h"

using namespace std;


class LeadTableau
{
  private:

    unsigned setNum;
    LeadTriple table[BRIDGE_DENOMS][BRIDGE_PLAYERS][BRIDGE_TRICKS];
    string tableStr[BRIDGE_DENOMS][BRIDGE_PLAYERS][BRIDGE_TRICKS];

    
    void constantTricks();

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

