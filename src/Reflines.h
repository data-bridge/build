/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_REFLINES_H
#define BRIDGE_REFLINES_H

#include <string>

#include "Refline.h"

using namespace std;


class Reflines
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

    vector<Refline> lines;
    RefControl control;


  public:

    Reflines();

    ~Reflines();

    vector<Refline>::iterator begin() { return lines.begin(); }

    vector<Refline>::iterator end() { return lines.end(); }

    void reset();

    void read(const string& fname);

    bool skip() const;
    bool validate() const;
    bool orderCOCO() const;
    bool orderOOCC() const;
};

#endif

