/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Canvas.h"


Canvas::Canvas()
{
  Canvas::reset();
}


Canvas::~Canvas()
{
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

  unsigned l = text.length();
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
  string str = "";
  for (unsigned i = 0; i < height; i++)
  {
    int p = static_cast<int>(width) - 1;
    while (p >= 0 && canvas[i].at(static_cast<unsigned>(p)) == ' ')
      p--;
    if (p < 0)
    {
      str += "\n";
      continue;
    }

    // Needed for REC annoyance in the deal diagram (Voids).
    unsigned num = static_cast<unsigned>(p+1);
    if (RECflag && (p == 12 || p == 24))
      num++;

    str += canvas[i].substr(0, num) + "\n";
  }

  return str;
}

