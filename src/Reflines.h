/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINES_H
#define BRIDGE_REFLINES_H

#include <string>

#include "RefLine.h"

using namespace std;


class RefLines
{
  private:

    enum RefControl
    {
      ERR_REF_STANDARD = 0,
      ERR_REF_SKIP = 1,
      ERR_REF_NOVAL = 2,
      ERR_REF_OUT_COCO = 3,
      ERR_REF_OUT_OOCC = 4
    };

    vector<RefLine> lines;
    RefControl control;


  public:

    RefLines();

    ~RefLines();

    vector<RefLine>::const_iterator begin() const { return lines.begin(); }
    vector<RefLine>::const_iterator end() const { return lines.end(); }

    void reset();

    void read(const string& fname);

    bool skip() const;
    bool validate() const;
    bool orderCOCO() const;
    bool orderOOCC() const;
};

#endif

