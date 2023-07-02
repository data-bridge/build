/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "Timers.h"


const string TimerName[]
{
  "Read",
  "Write",
  "Valid",
  "Comp",
  "Stats",
  "Refs",
  "Value",
  "Pass",
  "Sheet"
};


Timers::Timers()
{
  Timers::reset();
}


void Timers::reset()
{
  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
      timer[fnc][format].reset();
}


void Timers::start(
  const TimerFunction fnc,
  const Format format)
{
  timer[fnc][format].start();
}


void Timers::stop(
  const TimerFunction fnc,
  const Format format)
{
  timer[fnc][format].stop();
}


void Timers::operator += (const Timers& timer2)
{
  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
      timer[fnc][format] += timer2.timer[fnc][format];
}


void Timers::findActive(vector<unsigned>& active) const
{
  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
  {
    bool flag = false;
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
    {
      if (timer[fnc][format].no != 0)
      {
        flag = true;
        break;
      }
    }
    if (flag)
      active.push_back(fnc);
  }
}


string Timers::strTable(
  const string& header,
  const double table[][BRIDGE_FORMAT_SIZE],
  const vector<unsigned>& active,
  const int prec) const
{
  stringstream ss;

  ss << setw(8) << left << header;
  for (auto &fnc: active)
    ss << setw(12) << right << TimerName[fnc];
  ss << "\n";

  for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
  {
    ss << setw(8) << left << FORMAT_NAMES[format];

    for (auto &fnc: active)
    {
      if (table[fnc][format] == 0.)
        ss << setw(12) << right << "-";
      else
        ss << setw(12) << right << fixed << setprecision(prec) <<
          table[fnc][format];
    }
    ss << "\n";
  }
  return ss.str() + "\n";
}


string Timers::str(const unsigned numThreads) const 
{
  double table[BRIDGE_TIMER_SIZE][BRIDGE_FORMAT_SIZE];
  double sum = 0.;
  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
      sum += timer[fnc][format].sum;

  if (sum == 0.)
    return "";

  vector<unsigned> active;
  Timers::findActive(active);

  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
      table[fnc][format] = 100. * timer[fnc][format].sum / sum;

  stringstream ss;

  ss << Timers::strTable("Sum %", table, active);

  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
  {
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
    {
      if (timer[fnc][format].no == 0)
        table[fnc][format] = 0.;
      else
      {
        double avg = timer[fnc][format].sum / timer[fnc][format].no;
        table[fnc][format] = avg / 1000.;
      }
    }
  }

  ss << Timers::strTable("Avg ms", table, active);

  ss << "Time spent in main functions: " <<
    fixed << setprecision(2) << sum / 1000000. << " seconds";
  if (numThreads > 1)
    ss << " (" << numThreads << " threads)";

  return ss.str() + "\n";
}

