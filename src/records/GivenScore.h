/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_GIVENSCORE_H
#define BRIDGE_GIVENSCORE_H

enum Format: unsigned;

using namespace std;


class GivenScore
{
  private:

    float score;
    bool setFlag;


  public:

    GivenScore();

    void reset();

    void setIMP(
      const string& text,
      const Format format);

    void setMP(
      const string& text,
      const Format format);

    bool operator == (const GivenScore& gs2) const;

    bool operator != (const GivenScore& gs2) const;

    string str(const Format format) const;
};

#endif

