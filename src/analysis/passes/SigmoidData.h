/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_SIGMOIDDATA_H
#define BRIDGE_SIGMOIDDATA_H

#include <string>

using namespace std;

struct SigmoidData
{
  float mean;
  float divisor;

  float intercept;
  float slope;

  float crossover; // Probability value corresponding to intercept

  void reset()
  {
    mean = 0.f;
    divisor = 0.f;
    intercept = 0.f;
    slope = 0.f;
    crossover = 0.f;
  };
};

#endif

