/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

#include <string>
#include <iomanip>
#include <sstream>
#include <mutex>

#include "TextStats.h"
#include "parse.h"

#define BRIDGE_STATS_MAX_LENGTH 64
#define BRIDGE_STATS_NUM_FIELDS 7

static mutex mtx;
static bool setTextTables = false;
static unsigned BRIDGE_STATS_MAP[BRIDGE_FORMAT_SIZE];
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
    for (unsigned l = 0; l < BRIDGE_STATS_NUM_FIELDS; l++)
    {
      stats[f][l].datum.reserve(BRIDGE_STATS_MAX_LENGTH);
      stats[f][l].count = 0;
      stats[f][l].datum.resize(BRIDGE_STATS_MAX_LENGTH);
      for (unsigned i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
      {
        stats[f][l].datum[i].count = 0;
        stats[f][l].datum[i].source = "";
        stats[f][l].datum[i].example = "";
      }
    }
  }
}


void TextStats::setTables()
{
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    BRIDGE_STATS_MAP[f] = BRIDGE_FORMAT_SIZE;

  BRIDGE_STATS_MAP[BRIDGE_FORMAT_TITLE] = 0;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_LOCATION] = 1;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_EVENT] = 2;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_SESSION] = 3;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_TEAMS] = 4;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_AUCTION] = 5;
  BRIDGE_STATS_MAP[BRIDGE_FORMAT_PLAYERS_BOARD] = 6;

  BRIDGE_STATS_NAMES[0] = LABEL_NAMES[BRIDGE_FORMAT_TITLE];
  BRIDGE_STATS_NAMES[1] = LABEL_NAMES[BRIDGE_FORMAT_LOCATION];
  BRIDGE_STATS_NAMES[2] = LABEL_NAMES[BRIDGE_FORMAT_EVENT];
  BRIDGE_STATS_NAMES[3] = LABEL_NAMES[BRIDGE_FORMAT_SESSION];
  BRIDGE_STATS_NAMES[4] = LABEL_NAMES[BRIDGE_FORMAT_TEAMS];
  BRIDGE_STATS_NAMES[5] = LABEL_NAMES[BRIDGE_FORMAT_AUCTION];
  BRIDGE_STATS_NAMES[6] = LABEL_NAMES[BRIDGE_FORMAT_PLAYERS_BOARD];
}


void TextStats::add(
  const string& text,
  const string& source,
  const Label label,
  const Format format)
{
  unsigned lb = BRIDGE_STATS_MAP[label];
  unsigned len = text.length();
  if (stats[format][lb].datum[len].count == 0)
  {
    stats[format][lb].datum[len].source = basefile(source);
    stats[format][lb].datum[len].example = text;
  }
  stats[format][lb].datum[len].count++;
  stats[format][lb].count++;
}


void TextStats::add(
  const unsigned len,
  const string& source,
  const Label label,
  const Format format)
{
  unsigned lb = BRIDGE_STATS_MAP[label];
  if (stats[format][lb].datum[len].count == 0)
    stats[format][lb].datum[len].source = basefile(source);

  stats[format][lb].datum[len].count++;
  stats[format][lb].count++;
}


void TextStats::operator += (const TextStats& statsIn)
{
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    for (unsigned l = 0; l < BRIDGE_STATS_NUM_FIELDS; l++)
    {
      for (unsigned i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
      {
        TextDatum &td = stats[f][l].datum[i];
        const TextDatum &tdIn = statsIn.stats[f][l].datum[i];

        if (td.count == 0)
        {
          td.source = tdIn.source;
          td.example = tdIn.example;
        }
        td.count += tdIn.count;
      }
      stats[f][l].count += statsIn.stats[f][l].count;
    }
  }
}


string TextStats::posOrDash(const unsigned u) const
{
  if (u == 0)
    return "-";
  else
    return STR(u);
}


void TextStats::printDetails(
  const unsigned label,
  ostream& fstr) const
{
  fstr << setw(10) << left << BRIDGE_STATS_NAMES[label] <<
    setw(6) << right << "count" << "  " <<
    setw(24) << left << "source" <<
    left << "example" << endl;

  vector<TextDatum> labelSum;
  labelSum.reserve(BRIDGE_STATS_MAX_LENGTH);
  for (unsigned i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
    labelSum[i].count = 0;


  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    for (unsigned i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
    {
      const unsigned c = stats[f][label].datum[i].count;
      labelSum[i].count += c;
      if (c)
      {
        labelSum[i].source = stats[f][label].datum[i].source;
        labelSum[i].example = stats[f][label].datum[i].example;
      }
    }

  for (unsigned i = 0; i < BRIDGE_STATS_MAX_LENGTH; i++)
  {
    unsigned j = BRIDGE_STATS_MAX_LENGTH-1-i;
    if (labelSum[j].count == 0)
      continue;

    fstr << setw(10) << left << j <<
      setw(6) << right << labelSum[j].count << "  " <<
      setw(24) << left << labelSum[j].source <<
      left << labelSum[j].example << endl;
  }
  fstr << "\n";
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
      if (stats[f][l].count)
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
    unsigned m = 0;
    for (auto &f: activeFormats)
    {
      unsigned mf = 0;
      for (unsigned l = 0; l < BRIDGE_STATS_MAX_LENGTH; l++)
      {
        if (stats[f][lb].datum[l].count)
          mf = l;
      }

      fstr << setw(8) << right << mf;
      m = Max(m, mf);
    }
    fstr << setw(8) << right << m << "\n";
  }
  fstr << "\n";
}

