/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>

#include "Term.h"

#include "Valuation.h"

#include "../handling/Bexcept.h"
#include "../handling/Bexcept.h"


Term::Term()
{
  Term::reset();
}


void Term::reset()
{
  setFlag = false;
  cat = TERM_SIZE;
  compVal = COMPARATOR_SIZE;
}


void Term::set([[maybe_unused]] const string& text)
{
}


TermCategory Term::category() const
{
  return cat;
}


unsigned Term::paramNo() const
{
  return pno;
}


unsigned Term::suit() const
{
  if (cat == TERM_SUIT)
    return suitVal;
  else
    THROW("Suit not relevant");
}


TermComparator Term::comparator() const
{
  if (setFlag)
    return compVal;
  else
    THROW("Term not set");
}


int Term::limit() const
{
  if (compVal == COMPARATOR_IN)
    THROW("Term has two limits");
  else
    return limit1Val;
}


int Term::limit1() const
{
  if (compVal != COMPARATOR_IN)
    THROW("Term has one limit");
  else
    return limit1Val;
}


int Term::limit2() const
{
  if (compVal != COMPARATOR_IN)
    THROW("Term has one limit");
  else
    return limit2Val;
}


string Term::str() const
{
  return "";
}

