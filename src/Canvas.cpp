/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <sstream>
#include "Canvas.h"


Canvas::Canvas()
{
  Canvas::Reset();
}


Canvas::~Canvas()
{
}


void Canvas::Reset()
{
  height = 0;
  width = 0;
}


void Canvas::SetDimensions(
  const unsigned h,
  const unsigned w)
{
  height = h;
  width = w;

  canvas.resize(height);
  for (unsigned i = 0; i < height; i++)
    canvas[i].assign(w, ' ');
}


void Canvas::SetLine(
  const string& s,
  const unsigned lineNo,
  const unsigned colNo)
{
  if (lineNo >= height || colNo >= width)
    return;

  unsigned l = s.length();
  if (colNo + l > width)
    l -= colNo - width;

  canvas[lineNo].replace(colNo, l, s);
}


void Canvas::SetRectangle(
  const vector<string>& rect,
  const unsigned lineNo,
  const unsigned colNo)
{
  for (unsigned r = 0; r < rect.size(); r++)
  {
    if (lineNo +r >= height)
      return;

    Canvas::SetLine(rect[r], lineNo+r, colNo);
  }
}


string Canvas::AsString(const bool specialFlag) const
{
  // Drop trailing spaces.
  stringstream s;
  for (unsigned i = 0; i < height; i++)
  {
    int p = static_cast<int>(width) - 1;
    while (p >= 0 && canvas[i].at(static_cast<unsigned>(p)) == ' ')
      p--;
    if (p < 0)
      s << "\n";
    else if (specialFlag && (p == 12 || p == 24))
      // Needed for REC annoyance in the deal diagram
      s << canvas[i].substr(0, static_cast<unsigned>(p+2)) << "\n";
    else
      s << canvas[i].substr(0, static_cast<unsigned>(p+1)) << "\n";
  }

  return s.str();
}

