/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <iomanip>
#include <sstream>
#pragma warning(pop)

#include "bconst.h"
#include "GivenScore.h"
#include "parse.h"
#include "Bexcept.h"
#include "Bdiff.h"


GivenScore::GivenScore()
{
  GivenScore::reset();
}


GivenScore::~GivenScore()
{
}


void GivenScore::reset()
{
  score = 0.0f;
  setFlag = false;
}


void GivenScore::setIMP(
  const string& text,
  const Format format)
{
  // We regenerate this ourselves, so mostly ignore for now.
  if (format == BRIDGE_FORMAT_LIN)
  {
    if (text == "--")
      score = 0.0f;
    else if (! str2float(text, score))
      THROW("Bad IMP score: " + text);

    setFlag = true;
  }
  else if (format == BRIDGE_FORMAT_PBN)
  {
    if (text == "0")
    {
      score = 0.f;
      setFlag = true;
      return;
    }

    if (text.length() < 4)
      THROW("Short RBN IMP score: " + text);
    int side = 0;
    if (text.substr(0, 3) == "NS ")
      side = 1;
    else if (text.substr(0, 3) == "EW ")
      side = -1;
    else
      THROW("RBN IMP score not NS/EW: " + text);

    // Double Dummy Captain has also used commas.
    string fixed = text.substr(3);
    size_t p = fixed.find(",");
    if (p < string::npos)
      fixed.at(p) = '.';

    if (! str2float(fixed, score))
      THROW("Bad RBN IMP score: " + fixed);

    score *= static_cast<float>(side);
    setFlag = true;
  }
}


void GivenScore::setMP(
  const string& text,
  const Format format)
{
  if (format == BRIDGE_FORMAT_PBN)
  {
    // We mostly ignore this for now.
  }
  else
  {
    if (! str2float(text, score))
      THROW("Bad matchpoint score");

    setFlag = true;
  }
}


bool GivenScore::operator == (const GivenScore& gs2) const
{
  if (setFlag != gs2.setFlag)
    THROW("setFlag differ");

  if (! setFlag)
    return true;

  const float d = score - gs2.score;
  if (d > 0.001 || d < -0.001)
    return false;
  else
    return true;
}


bool GivenScore::operator != (const GivenScore& gs2) const
{
  return ! (* this == gs2);
}


string GivenScore::str(const Format format) const
{
  stringstream ss;
  
  if (FORMAT_INPUT_MAP[format] == BRIDGE_FORMAT_LIN)
  {
    if (score == 0.0f)
    {
      if ((format == BRIDGE_FORMAT_LIN ||
          format == BRIDGE_FORMAT_LIN_TRN) && 
          ! setFlag)
        ss << ",,";
      else
        ss << "--,--,";
    }
    else if (score > 0.0f)
      ss << setprecision(1) << fixed << score << ",,";
    else
      ss << "," << setprecision(1) << fixed << -score << ",";
    return ss.str();
  }
  else if (format == BRIDGE_FORMAT_PBN && setFlag)
  {
    ss << "[ScoreIMP \"NS ";
    ss << setprecision(2) << fixed << score << "\"]\n";

    return ss.str();
  }
  else
    return "";
}

