/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_CANVAS_H
#define BRIDGE_CANVAS_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <string>
#include <vector>
#pragma warning(pop)

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

