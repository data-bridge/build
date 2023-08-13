/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_DISTELEM_H
#define BRIDGE_DISTELEM_H

using namespace std;


class DistElem
{
  private:

    string name;
    bool setFlag;
    unsigned min;
    unsigned max;

  public:

    DistElem();

    void reset();

    void set(
      const string& nameIn,
      const string& spec);

    bool match(const int value) const;

    unsigned getMax() const;

    string str() const;
};

#endif
