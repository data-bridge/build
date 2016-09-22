/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CANVAS_H
#define BRIDGE_CANVAS_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;


class Canvas
{
  private:
    
    vector<string> canvas;
    unsigned height;
    unsigned width;

  public:

    Canvas();

    ~Canvas();

    void Reset();

    void SetDimensions(
      const unsigned h,
      const unsigned w);

    void SetLine(
      const string& s,
      const unsigned lineNo,
      const unsigned colNo);

    void SetRectangle(
      const vector<string>& rect,
      const unsigned lineNo,
      const unsigned colNo);

    string AsString() const;
};

#endif

