/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// #include <string>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <assert.h>

#include "TextStats.h"

// #include "../parse.h"

#define BRIDGE_STATS_NUM_FIELDS 7

static mutex mtx;
static bool setTextTables = false;
static unsigned BRIDGE_STATS_MAP[BRIDGE_FORMAT_LABELS_SIZE];
static string BRIDGE_STATS_NAMES[BRIDGE_STATS_NUM_FIELDS];


TextStats::TextStats()
{
  TextStats::reset();
  if (! setTextTables)
  {
    mtx.lock();
    if (! setTextTables)
      TextStats::setTables();
    setTextTables = true;
    mtx.unlock();
  }
}


TextStats::~TextStats()
{
}


void TextStats::reset()
{
  stats.resize(BRIDGE_FORMAT_SIZE);
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    stats[f].resize(BRIDGE_STATS_NUM_FIELDS);
    for (auto& labelStat: stats[f])
      labelStat.reset();
  }
}


void TextStats::setTables()
{
  for (unsigned lb = 0; lb < BRIDGE_FORMAT_LABELS_SIZE; lb++)
    BRIDGE_STATS_MAP[lb] = BRIDGE_FORMAT_LABELS_SIZE;

  BRIDGE_STATS_MAP[BRIDGE_FORMAT_TITLE] = 0;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_LOCATION] = 1;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_EVENT] = 2;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_SESSION] = 3;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_TEAMS] = 4;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_AUCTION] = 5;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_PLAYERS] = 6;

  BRIDGE_STATS_NAMES[0] = LABEL_NAMES[BRIDGE_FORMAT_TITLE];
  BRIDGE_STATS_NAMES[1] = LABEL_NAMES[BRIDGE_FORMAT_LOCATION];
  BRIDGE_STATS_NAMES[2] = LABEL_NAMES[BRIDGE_FORMAT_EVENT];
  BRIDGE_STATS_NAMES[3] = LABEL_NAMES[BRIDGE_FORMAT_SESSION];
  BRIDGE_STATS_NAMES[4] = LABEL_NAMES[BRIDGE_FORMAT_TEAMS];
  BRIDGE_STATS_NAMES[5] = LABEL_NAMES[BRIDGE_FORMAT_AUCTION];
  BRIDGE_STATS_NAMES[6] = LABEL_NAMES[BRIDGE_FORMAT_PLAYERS];
}


void TextStats::add(
  const string& text,
  const string& source,
  const Label label,
  const Format format)
{
  assert(format < BRIDGE_FORMAT_SIZE);
  unsigned lb = BRIDGE_STATS_MAP[label];
  assert(lb < BRIDGE_STATS_NUM_FIELDS);

  stats[format][lb].add(source, text);
}


void TextStats::add(
  const unsigned len,
  const string& source,
  const Label label,
  const Format format)
{
  assert(format < BRIDGE_FORMAT_SIZE);
  unsigned lb = BRIDGE_STATS_MAP[label];
  assert(lb < BRIDGE_STATS_NUM_FIELDS);

  stats[format][lb].add(source, len);
}


void TextStats::operator += (const TextStats& statsIn)
{
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    for (unsigned l = 0; l < BRIDGE_STATS_NUM_FIELDS; l++)
      stats[f][l] += statsIn.stats[f][l];
}


void TextStats::printDetails(
  const unsigned label,
  ostream& fstr) const
{
  assert(label < BRIDGE_STATS_NUM_FIELDS);

  TextStat labelSum;

  labelSum.reset();
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    labelSum += stats[f][label];

  fstr << labelSum.strHeader(BRIDGE_STATS_NAMES[label]);
  fstr << labelSum.str();
}


void TextStats::print(
  ostream& fstr,
  const bool detailsFlag) const
{
  if (detailsFlag)
    for (unsigned l = 0; l < BRIDGE_STATS_NUM_FIELDS; l++)
      TextStats::printDetails(l, fstr);
  fstr << "\n";

  // Add up across formats, preserve one example and source per size
  vector<unsigned> activeFormats;
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    for (unsigned l = 0; l < BRIDGE_STATS_NUM_FIELDS; l++)
    {
      if (! stats[f][l].empty())
      {
        activeFormats.push_back(f);
        break;
      }
    }
  }

  fstr << setw(8) << left << "Param.";
  for (auto &f: activeFormats)
    fstr << setw(8) << right << FORMAT_NAMES[f];
  fstr << setw(8) << right << "max" << "\n";

  for (unsigned lb = 0; lb < BRIDGE_STATS_NUM_FIELDS; lb++)
  {
    fstr << setw(8) << left << BRIDGE_STATS_NAMES[lb];
    size_t m = 0;
    for (auto &f: activeFormats)
    {
      size_t mf = stats[f][lb].last_used();

      fstr << setw(8) << right << mf;
      m = Max(m, mf);
    }
    fstr << setw(8) << right << m << "\n";
  }
  fstr << "\n";
}

