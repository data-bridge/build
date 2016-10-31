/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/


#include <iomanip>

#include "Timers.h"


const string TimerName[]
{
  "Read",
  "Write",
  "Valid",
  "Comp"
};


Timers::Timers()
{
  Timers::reset();
}


Timers::~Timers()
{
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


void Timers::printTable(
  const string& header,
  const double table[][BRIDGE_FORMAT_SIZE],
  const unsigned prec) const
{
  cout << setw(8) << left << header;
  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    cout << setw(12) << right << TimerName[fnc];
  cout << "\n";

  for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
  {
    cout << setw(8) << left << FORMAT_NAMES[format];

    for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    {
      if (table[fnc][format] == 0.)
        cout << setw(12) << right << "-";
      else
        cout << setw(12) << right << fixed << setprecision(prec) <<
          table[fnc][format];
    }
    cout << "\n";
  }
  cout << "\n";
}


void Timers::print(const unsigned numThreads) const 
{
  double table[BRIDGE_TIMER_SIZE][BRIDGE_FORMAT_SIZE];
  double sum = 0.;
  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
      sum += timer[fnc][format].sum;

  if (sum == 0.)
    return;

  cout << "Time spent in main functions: " <<
    fixed << setprecision(2) << sum / 1000000. << " seconds";
  if (numThreads > 1)
    cout << " (" << numThreads << " threads)";
  cout << "\n\n";

  for (unsigned fnc = 0; fnc < BRIDGE_TIMER_SIZE; fnc++)
    for (unsigned format = 0; format < BRIDGE_FORMAT_SIZE; format++)
      table[fnc][format] = 100. * timer[fnc][format].sum / sum;

  Timers::printTable("Sum %", table);

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

  Timers::printTable("Avg ms", table);
}

