/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SCORING_H
#define BRIDGE_SCORING_H

#include <string>

enum Format: unsigned;

using namespace std;


class Scoring
{
  private:

    // At present only IMPS and MATCHPOINTS are implemented.
    enum ScoringType
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

    ScoringType scoring;

    void setLIN(const string& text);
    void setPBN(const string& text);
    void setRBN(const string& text);

    string strLIN() const;
    string strPBN() const;
    string strRBN() const;
    string strRBX() const;
    string strEML() const;
    string strREC() const;


  public:

    Scoring();

    void reset();

    void set(
      const string& text,
      const Format format);

    bool isIMPs() const;

    bool operator == (const Scoring& scoring2) const;

    bool operator != (const Scoring& scoring2) const;

    string str(const Format format) const;
};

#endif

