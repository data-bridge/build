/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <cassert>

#include "SigmoidData.h"
#include "Sigmoid.h"


float Sigmoid::calcLinear(
  const SigmoidData& sigmoidData,
  const float input) const
{
  const float value = sigmoidData.crossover - 
    sigmoidData.slope * (input - sigmoidData.intercept);

  return (value >= 1.f ? 1.f : value);
}


float Sigmoid::calcSigmoid(
  const SigmoidData& sigmoidData,
  const float input) const
{
  return 1.f - 1.f / (1.f + 
    exp(-(input-sigmoidData.mean) / sigmoidData.divisor));
}


float Sigmoid::calc(
  const SigmoidData& sigmoidData,
  const float input) const
{
  if (input <= sigmoidData.intercept)
    return Sigmoid::calcLinear(sigmoidData, input);
  else
    return Sigmoid::calcSigmoid(sigmoidData, input);
}

