/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_TIMER_H
#define BRIDGE_TIMER_H

#include <chrono>

using namespace std;

class Timers;


class Timer
{
  private:

    unsigned no;
    double sum;
    std::chrono::time_point<std::chrono::high_resolution_clock> begin;


  public:

    friend Timers;

    Timer();

    ~Timer();

    void reset();

    void start();

    void stop();

    void operator += (const Timer& timer2);

    string str(const int prec = 1) const;
};

#endif

