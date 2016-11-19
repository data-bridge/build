/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TIMERS_H
#define BRIDGE_TIMERS_H

#include <iostream>
#include <chrono>

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
  BRIDGE_TIMER_SIZE = 5
};



class Timers
{
  private:

    Timer timer[BRIDGE_TIMER_SIZE][BRIDGE_FORMAT_SIZE];


    void printTable(
      const string& header,
      const double table[][BRIDGE_FORMAT_SIZE],
      const unsigned prec = 1) const;


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

