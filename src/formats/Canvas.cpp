/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include "Canvas.h"


Canvas::Canvas()
{
  Canvas::reset();
}


void Canvas::reset()
{
  height = 0;
  width = 0;
}


void Canvas::resize(
  const unsigned heightIn,
  const unsigned widthIn)
{
  height = heightIn;
  width = widthIn;

  canvas.resize(height);
  for (unsigned i = 0; i < height; i++)
    canvas[i].assign(width, ' ');
}


void Canvas::setLine(
  const string& text,
  const unsigned lineNo,
  const unsigned colNo)
{
  if (lineNo >= height || colNo >= width)
    return;

  unsigned l = static_cast<unsigned>(text.length());
  if (colNo + l > width)
    l -= colNo - width;

  canvas[lineNo].replace(colNo, l, text);
}


void Canvas::setRectangle(
  const vector<string>& rectangle,
  const unsigned lineNo,
  const unsigned colNo)
{
  for (unsigned r = 0; r < rectangle.size(); r++)
  {
    if (lineNo+r >= height)
      return;

    Canvas::setLine(rectangle[r], lineNo+r, colNo);
  }
}


string Canvas::str(const bool RECflag) const
{
  // Drop trailing spaces.
  string st = "";
  for (unsigned i = 0; i < height; i++)
  {
    unsigned p = width;
    while (p >= 1 && canvas[i].at(p-1) == ' ')
      p--;
    if (p == 0)
    {
      st += "\n";
      continue;
    }

    // Needed for REC annoyance in the deal diagram (Voids).
    unsigned num = p;
    if (RECflag && (p == 13 || p == 25))
      num++;

    st += canvas[i].substr(0, num) + "\n";
  }

  return st;
}

