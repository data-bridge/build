/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SIGMOID_H
#define BRIDGE_SIGMOID_H

#include <string>

using namespace std;

struct SigmoidData;


class Sigmoid
{
  // We start out as a class, as we might do some table lookups later.

  public:
    
    float calcLinear(
      const SigmoidData& sigmoidData,
      const float input) const;
    
    float calcSigmoid(
      const SigmoidData& sigmoidData,
      const float input) const;
    
    float calc(
      const SigmoidData& sigmoidData,
      const float input) const;
};

#endif

