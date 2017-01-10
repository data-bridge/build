/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CANVAS_H
#define BRIDGE_CANVAS_H

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

    void reset();

    void resize(
      const unsigned height,
      const unsigned width);

    void setLine(
      const string& text,
      const unsigned lineNo,
      const unsigned colNo);

    void setRectangle(
      const vector<string>& rectangle,
      const unsigned lineNo,
      const unsigned colNo);

    string str(const bool RECflag = false) const;
};

#endif

