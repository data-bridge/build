/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#include <sstream>
#include <iomanip>

#include "TextDatum.h"


void TextDatum::reset()
{
  source = "";
  example = "";
  count = 0;
}


void TextDatum::add(
  const string& sourceIn,
  const string& exampleIn,
  const size_t countIn)
{
  if (count == 0)
  {
    source = sourceIn;
    example = exampleIn;
  }
  count += countIn;
}


void TextDatum::operator += (const TextDatum& td2)
{
  TextDatum::add(td2.source, td2.example, td2.count);
}


bool TextDatum::empty() const
{
  return (count == 0);
}


string TextDatum::strHeader() const
{
  stringstream ss;
  ss <<
    setw(6) << right << "count" << "  " <<
    setw(24) << left << "source" <<
    left << "example";
  return ss.str();
}


string TextDatum::str() const
{
  stringstream ss;
  ss <<
    setw(6) << right << count << "  " <<
    setw(24) << left << source <<
    left << example;
  return ss.str();
}

