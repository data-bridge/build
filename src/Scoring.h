/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SCORING_H
#define BRIDGE_SCORING_H

#include "bconst.h"
#include <string>

using namespace std;


class Scoring
{
  private:

    // At present only IMPS and MATCHPOINTS are implemented.
    enum scoringType
    {
      BRIDGE_SCORING_IMPS = 0,
      BRIDGE_SCORING_BAM = 1,
      BRIDGE_SCORING_TOTAL = 2,
      BRIDGE_SCORING_CROSS_IMPS = 3,
      BRIDGE_SCORING_MATCHPOINTS = 4,
      BRIDGE_SCORING_INSTANT = 5,
      BRIDGE_SCORING_RUBBER = 6,
      BRIDGE_SCORING_CHICAGO = 7,
      BRIDGE_SCORING_CAVENDISH = 8,
      BRIDGE_SCORING_UNDEFINED = 9
    };

    scoringType scoring;

    bool SetLIN(const string& t);
    bool SetPBN(const string& t);
    bool SetRBN(const string& t);

    string AsLIN() const;
    string AsPBN() const;
    string AsRBN() const;
    string AsEML() const;


  public:

    Scoring();

    ~Scoring();

    void Reset();

    bool Set(
      const string& t,
      const formatType f);

    bool ScoringIsIMPs() const;

    bool operator == (const Scoring& s2) const;

    bool operator != (const Scoring& s2) const;

    string AsString(const formatType f) const;
};

#endif

