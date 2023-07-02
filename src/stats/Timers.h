/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TIMERS_H
#define BRIDGE_TIMERS_H

#include <string>
#include <chrono>

#include "Timer.h"

#include "../include/Format.h"

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
  BRIDGE_TIMER_PASS = 7,
  BRIDGE_TIMER_DIGEST = 8,
  BRIDGE_TIMER_SIZE = 9
};



class Timers
{
  private:

    Timer timer[BRIDGE_TIMER_SIZE][BRIDGE_FORMAT_SIZE];


    void findActive(vector<unsigned>& active) const;

    string strTable(
      const string& header,
      const double table[][BRIDGE_FORMAT_SIZE],
      const vector<unsigned>& active,
      const int prec = 1) const;


  public:

    Timers();

    void reset();

    void start(
      const TimerFunction fnc,
      const Format format);

    void stop(
      const TimerFunction fnc,
      const Format format);

    void operator += (const Timers& timers2);

    string str(const unsigned numThreads = 1) const;
};

#endif

