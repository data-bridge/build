/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <sstream>
#pragma warning(pop)

#include "Scoring.h"
#include "Bexcept.h"
#include "Bdiff.h"


// This maps the ScoringStruct fields to scoring tags.
// No doubt LIN also has tags for some of these.
static const string SCORING_LIN[] =
{
  "I", "B", "X", "X", "P", "X", "X", "X", "X", ""
};

static const string SCORING_PBN[] =
{
  "IMP", "BAM", "X", "X", "Matchpoints", 
  "Instant", "Rubber", "Chicago", "Cavendish", ""
};

static const string SCORING_RBN[] =
{
  "I", "B", "T", "X", "M", "N", "R", "C", "A", ""
};

static const string SCORING_EML[] =
{
  "IMPs", 
  "Board-a-match", 
  "Total points", 
  "Cross IMPs", 
  "Matchpoints", 
  "Instant", 
  "Rubber", 
  "Chicago", 
  "Cavendish", 
  ""
};


Scoring::Scoring()
{
  Scoring::reset();
}


Scoring::~Scoring()
{
}


void Scoring::reset()
{
  scoring = BRIDGE_SCORING_UNDEFINED;
}


void Scoring::setLIN(const string& text)
{
  if (text == "I")
    scoring = BRIDGE_SCORING_IMPS;
  else if (text == "P")
    scoring = BRIDGE_SCORING_MATCHPOINTS;
  else if (text == "B")
    scoring = BRIDGE_SCORING_BAM;
  else
    THROW("Unknown LIN scoring");
}


void Scoring::setPBN(const string& text)
{
  if (text == "IMP" || text == "IMPs")
    scoring = BRIDGE_SCORING_IMPS;
  else if (text == "BAM")
    scoring = BRIDGE_SCORING_BAM;
  else if (text == "MP" || text == "Matchpoints" || text == "P")
    scoring = BRIDGE_SCORING_MATCHPOINTS;
  else if (text == "Instant")
    scoring = BRIDGE_SCORING_INSTANT;
  else if (text == "Rubber")
    scoring = BRIDGE_SCORING_RUBBER;
  else if (text == "Chicago")
    scoring = BRIDGE_SCORING_CHICAGO;
  else if (text == "Cavendish")
    scoring = BRIDGE_SCORING_CAVENDISH;
  else
    THROW("Unknown PBN scoring");
}


void Scoring::setRBN(const string& text)
{
  if (text == "I")
    scoring = BRIDGE_SCORING_IMPS;
  else if (text == "B")
    scoring = BRIDGE_SCORING_BAM;
  else if (text == "T")
    scoring = BRIDGE_SCORING_TOTAL;
  else if (text == "X")
    scoring = BRIDGE_SCORING_CROSS_IMPS;
  else if (text == "M")
    scoring = BRIDGE_SCORING_MATCHPOINTS;
  else if (text == "N")
    scoring = BRIDGE_SCORING_INSTANT;
  else if (text == "R")
    scoring = BRIDGE_SCORING_RUBBER;
  else if (text == "C")
    scoring = BRIDGE_SCORING_CHICAGO;
  else if (text == "A")
    scoring = BRIDGE_SCORING_CAVENDISH;
  else
    THROW("Unknown RBN scoring");
}


void Scoring::set(
  const string& text,
  const Format format)
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
      return Scoring::setLIN(text);
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      return Scoring::setPBN(text);
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Scoring::setRBN(text);
    
    default:
      THROW("Invalid format: " + STR(format));
  }
}


bool Scoring::isIMPs() const
{
  return (scoring == BRIDGE_SCORING_IMPS);
}


bool Scoring::operator == (const Scoring& scoring2) const
{
  if (scoring != scoring2.scoring)
    DIFF("Scoring differs");
  else
    return true;
}


bool Scoring::operator != (const Scoring& scoring2) const
{
  return ! (* this == scoring2);
}


string Scoring::strLIN() const
{
  return SCORING_LIN[scoring];
}


string Scoring::strPBN() const
{
  return "[Scoring \"" + SCORING_PBN[scoring] + "\"]\n";
}


string Scoring::strRBN() const
{
  return "F " + SCORING_RBN[scoring] + "\n";
}

string Scoring::strRBX() const
{
  return "F{" + SCORING_RBN[scoring] + "}";
}


string Scoring::strEML() const
{
  return SCORING_EML[scoring];
}


string Scoring::strREC() const
{
  return SCORING_PBN[scoring];
}


string Scoring::str(const Format format) const
{
  switch(format)
  {
    case BRIDGE_FORMAT_LIN:
    case BRIDGE_FORMAT_LIN_RP:
      return Scoring::strLIN();
    
    case BRIDGE_FORMAT_PBN:
      return Scoring::strPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Scoring::strRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Scoring::strRBX();
    
    case BRIDGE_FORMAT_EML:
      return Scoring::strEML();
    
    case BRIDGE_FORMAT_REC:
      return Scoring::strREC();

    default:
      THROW("Invalid format " + STR(format));
  }
}

