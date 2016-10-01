/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include "Scoring.h"
#include "portab.h"
#include "Debug.h"

extern Debug debug;


// No doubt LIN also has tags for some of these
const string SCORING_LIN[] =
{
  "I", "X", "X", "X", "P", "X", "X", "X", "X", "X"
};

const string SCORING_PBN[] =
{
  "IMP", "BAM", "X", "X", "Matchpoints", 
  "Instant", "Rubber", "Chicago", "Cavendish", "X"
};

const string SCORING_RBN[] =
{
  "I", "B", "T", "X", "M", "N", "R", "C", "A", "P"
};

const string SCORING_EML[] =
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
  "Undefined"
};


Scoring::Scoring()
{
  Scoring::Reset();
}


Scoring::~Scoring()
{
}


void Scoring::Reset()
{
  scoring = BRIDGE_SCORING_UNDEFINED;
}


bool Scoring::SetLIN(const string& t)
{
  if (t == "I")
    scoring = BRIDGE_SCORING_IMPS;
  else if (t == "P")
    scoring = BRIDGE_SCORING_MATCHPOINTS;
  else
  {
    LOG("Unknown LIN scoring");
    return false;
  }
  return true;
}


bool Scoring::SetPBN(const string& t)
{
  if (t == "IMP" || t == "IMPs")
    scoring = BRIDGE_SCORING_IMPS;
  else if (t == "BAM")
    scoring = BRIDGE_SCORING_BAM;
  else if (t == "MP" || t == "Matchpoints")
    scoring = BRIDGE_SCORING_MATCHPOINTS;
  else if (t == "Instant")
    scoring = BRIDGE_SCORING_INSTANT;
  else if (t == "Rubber")
    scoring = BRIDGE_SCORING_RUBBER;
  else if (t == "Chicago")
    scoring = BRIDGE_SCORING_CHICAGO;
  else if (t == "Cavendish")
    scoring = BRIDGE_SCORING_CAVENDISH;
  else
    return false;
  return true;
}


bool Scoring::SetRBN(const string& t)
{
  if (t == "I")
    scoring = BRIDGE_SCORING_IMPS;
  else if (t == "B")
    scoring = BRIDGE_SCORING_BAM;
  else if (t == "T")
    scoring = BRIDGE_SCORING_TOTAL;
  else if (t == "X")
    scoring = BRIDGE_SCORING_CROSS_IMPS;
  else if (t == "M")
    scoring = BRIDGE_SCORING_MATCHPOINTS;
  else if (t == "N")
    scoring = BRIDGE_SCORING_INSTANT;
  else if (t == "R")
    scoring = BRIDGE_SCORING_RUBBER;
  else if (t == "C")
    scoring = BRIDGE_SCORING_CHICAGO;
  else if (t == "A")
    scoring = BRIDGE_SCORING_CAVENDISH;
  else
    return false;
  return true;
}


bool Scoring::Set(
  const string& t,
  const formatType f)
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Scoring::SetLIN(t);
    
    case BRIDGE_FORMAT_PBN:
    case BRIDGE_FORMAT_EML:
    case BRIDGE_FORMAT_TXT:
    case BRIDGE_FORMAT_REC:
      return Scoring::SetPBN(t);
    
    case BRIDGE_FORMAT_RBN:
    case BRIDGE_FORMAT_RBX:
      return Scoring::SetRBN(t);
    
    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}


bool Scoring::ScoringIsIMPs() const
{
  return (scoring == BRIDGE_SCORING_IMPS);
}


bool Scoring::operator == (const Scoring& s2) const
{
  if (scoring != s2.scoring)
  {
    LOG("Scoring differs");
    return false;
  }
  else
    return true;
}


bool Scoring::operator != (const Scoring& s2) const
{
  return ! (* this == s2);
}


string Scoring::AsLIN() const
{
  if (scoring == BRIDGE_SCORING_UNDEFINED)
    return "";
  return SCORING_LIN[scoring];
}


string Scoring::AsPBN() const
{
  if (scoring == BRIDGE_SCORING_UNDEFINED)
    return "";
  return "[Scoring \"" + SCORING_PBN[scoring] + "\"]\n";
}


string Scoring::AsRBN() const
{
  if (scoring == BRIDGE_SCORING_UNDEFINED)
    return "";
  return "F " + SCORING_RBN[scoring] + "\n";
}

string Scoring::AsRBX() const
{
  if (scoring == BRIDGE_SCORING_UNDEFINED)
    return "";
  return "F{" + SCORING_RBN[scoring] + "}";
}


string Scoring::AsEML() const
{
  if (scoring == BRIDGE_SCORING_UNDEFINED)
    return "";
  return SCORING_EML[scoring];
}


string Scoring::AsREC() const
{
  if (scoring == BRIDGE_SCORING_UNDEFINED)
    return "";
  return SCORING_PBN[scoring];
}


string Scoring::AsString(const formatType f) const
{
  switch(f)
  {
    case BRIDGE_FORMAT_LIN:
      return Scoring::AsLIN();
    
    case BRIDGE_FORMAT_PBN:
      return Scoring::AsPBN();
    
    case BRIDGE_FORMAT_RBN:
      return Scoring::AsRBN();
    
    case BRIDGE_FORMAT_RBX:
      return Scoring::AsRBX();
    
    case BRIDGE_FORMAT_EML:
      return Scoring::AsEML();
    
    case BRIDGE_FORMAT_REC:
      return Scoring::AsREC();

    default:
      LOG("Invalid format " + STR(f));
      return "";
  }
}

