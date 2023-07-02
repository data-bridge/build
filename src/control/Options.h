/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_OPTIONS_H
#define BRIDGE_OPTIONS_H

#include "../analysis/DistMatcher.h"

enum Format: unsigned;

using namespace std;


// Input options.

struct FileOption
{
  bool setFlag;
  string name;
};

struct Options
{
  FileOption fileInput; // -i, --infile
  FileOption dirInput; // -I, --indir

  FileOption fileOutput; // -o, --outfile
  FileOption dirOutput; // -O, --outdir

  FileOption fileRef; // -r, --reffile
  FileOption dirRef; // -R, --refdir

  FileOption fileDigest; // -d, --digfile
  FileOption dirDigest; // -D, --digdir

  FileOption fileLog; // -l, --logfile

  bool tableIMPFlag; // -T, --tableIMP
  bool compareFlag; // -c, --compare
  bool playersFlag; // -p, --players
  bool passStatsFlag; // -Q, --pass
  bool equalFlag; // -e, --equal
  bool valuationFlag; // -V, --valuation
  bool solveFlag; // -S, --solve
  bool traceFlag; // -T, --trace

  bool formatSetFlag; // -f, --format
  Format format;

  DistMatcher distMatcher; // Comes from -Q

  bool statsFlag; // -s, --stats
  bool quoteFlag; // -q, --quote

  unsigned numThreads;

  bool verboseIO;
  bool verboseThrow;
  bool verboseBatch;

  bool verboseValStats;
  bool verboseValDetails;

  bool verboseTextStats;
};

#endif

