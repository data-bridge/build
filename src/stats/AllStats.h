/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This is a collective class for the main statistics that we
   collect over a run, a lot of it for debugging input files.
   Example outputs are shown in the .h files of the main classes.
   Output is controlled mainly by Options.

   Code that wants to use the AllStats members must include the
   relevant .h files.  The separation is an attempt to contain
   the necessary include files across the whole project.
*/

#ifndef BRIDGE_ALLSTATS_H
#define BRIDGE_ALLSTATS_H

#include <vector>
#include <string>

class ValStats;
class TextStats;
class CompStats;
class RefStats;
class DuplStats;
class ParamStats1D;
class ParamStats2D;
class RuleStats;
class Timers;

struct Options;

using namespace std;


struct AllStats
{
  ValStats * valStatsPtr;
  TextStats * textStatsPtr;
  CompStats * compStatsPtr;
  RefStats * refStatsPtr;
  DuplStats * duplStatsPtr;
  vector<ParamStats1D> * paramStats1DPtr;
  vector<ParamStats2D> * paramStats2DPtr;
  RuleStats * ruleStatsPtr;
  Timers * timersPtr;

  AllStats();

  ~AllStats();

  void operator += (const AllStats& as2);

  string str(const Options& options) const;
};


// This is not a struct method.

void mergeResults(
  vector<AllStats>& allStatsList,
  const Options& options);

#endif

