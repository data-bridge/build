/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TIMER_H
#define BRIDGE_TIMER_H

#include <iostream>
#include <chrono>

#include "bconst.h"

using namespace std;


enum TimerFunction
{
  BRIDGE_TIMER_READ = 0,
  BRIDGE_TIMER_WRITE = 1,
  BRIDGE_TIMER_VALIDATE = 2,
  BRIDGE_TIMER_COMPARE = 3,
  BRIDGE_TIMER_SIZE = 4
};



class Timer
{
  private:

    struct TimerCell
    {
      unsigned no;
      double sum;
      std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };

    TimerCell timer[BRIDGE_TIMER_SIZE][BRIDGE_FORMAT_SIZE];


    void printTable(
      const string& header,
      const double table[][BRIDGE_FORMAT_SIZE],
      const unsigned prec = 1) const;


  public:

    Timer();

    ~Timer();

    void reset();

    void start(
      const TimerFunction fnc,
      const Format format);

    void stop(
      const TimerFunction fnc,
      const Format format);

    void operator += (const Timer& timer2);

    void print(const unsigned numThreads = 1) const;
};

#endif

