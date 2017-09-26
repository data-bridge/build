/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TIMERS_H
#define BRIDGE_TIMERS_H

#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iostream>
#include <chrono>
#pragma warning(pop)

#include "Timer.h"
#include "bconst.h"

using namespace std;


enum TimerFunction
{
  BRIDGE_TIMER_READ = 0,
  BRIDGE_TIMER_WRITE = 1,
  BRIDGE_TIMER_VALIDATE = 2,
  BRIDGE_TIMER_COMPARE = 3,
  BRIDGE_TIMER_STATS = 4,
  BRIDGE_TIMER_REF_STATS = 5,
  BRIDGE_TIMER_VALUE = 6,
  BRIDGE_TIMER_DIGEST = 7,
  BRIDGE_TIMER_SIZE = 8
};



class Timers
{
  private:

    Timer timer[BRIDGE_TIMER_SIZE][BRIDGE_FORMAT_SIZE];


    void findActive(vector<unsigned>& active) const;

    void printTable(
      const string& header,
      const double table[][BRIDGE_FORMAT_SIZE],
      const vector<unsigned> active,
      const int prec = 1) const;


  public:

    Timers();

    ~Timers();

    void reset();

    void start(
      const TimerFunction fnc,
      const Format format);

    void stop(
      const TimerFunction fnc,
      const Format format);

    void operator += (const Timers& timers2);

    void print(const unsigned numThreads = 1) const;
};

#endif

