/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

#include <sstream>
#include <iomanip>
#include <assert.h>

#include "TextStats.h"

#include "../Format.h"
#include "../Label.h"


TextStats::TextStats()
{
  TextStats::reset();
}


TextStats::~TextStats()
{
}


void TextStats::reset()
{
  stats.resize(BRIDGE_FORMAT_SIZE);
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    stats[f].resize(BRIDGE_FORMAT_LABELS_SIZE);
    for (auto& labelStat: stats[f])
      labelStat.reset();
  }
}


void TextStats::add(
  const string& example,
  const string& source,
  const Label label,
  const Format format)
{
  assert(format < BRIDGE_FORMAT_SIZE);
  assert(label < BRIDGE_FORMAT_LABELS_SIZE);

  stats[format][label].add(source, example);
}


  void TextStats::add(
  const unsigned len,
  const string& source,
  const Label label,
  const Format format)
{
  assert(format < BRIDGE_FORMAT_SIZE);
  assert(label < BRIDGE_FORMAT_LABELS_SIZE);

  stats[format][label].add(source, len);
}


void TextStats::operator += (const TextStats& statsIn)
{
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    for (unsigned l = 0; l < BRIDGE_FORMAT_LABELS_SIZE; l++)
      stats[f][l] += statsIn.stats[f][l];
}


void TextStats::strPrepare(
  vector<size_t>& activeFormats,
  vector<size_t>& labelMaxima) const
{
  // Find those formats have non-zero entries.
  // Find the highest count for each label across all formats.

  activeFormats.resize(BRIDGE_FORMAT_SIZE);
  labelMaxima.resize(BRIDGE_FORMAT_LABELS_SIZE);
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
  {
    for (unsigned label = 0; label < BRIDGE_FORMAT_LABELS_SIZE; label++)
    {
      const auto& labelStat = stats[f][label];
      if (! labelStat.empty())
      {
        activeFormats[f] = 1;
        labelMaxima[label] = max(labelMaxima[label], labelStat.last_used());
      }
    }
  }
}


void TextStats::strExamplesPrepare(
  TextStat& labelSum,
  const Label label) const
{
  labelSum.reset();
  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    labelSum += stats[f][label];
}


string TextStats::strExamples(const Label label) const
{
  TextStat labelSum;
  TextStats::strExamplesPrepare(labelSum, label);
  if (labelSum.empty())
    return "";

  stringstream ss;
  ss << labelSum.strHeader(LABEL_NAMES[label]);
  ss << labelSum.str();
  return ss.str();
}


string TextStats::strParamHeader(const vector<size_t>& activeFormats) const
{
  stringstream ss;
  ss << setw(8) << left << "Param.";

  for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
    if (activeFormats[f])
      ss << setw(8) << right << FORMAT_NAMES[f];

  ss << setw(8) << right << "max" << "\n";

  return ss.str();
}


string TextStats::strParams(
  const vector<size_t>& activeFormats,
  const vector<size_t>& labelMaxima) const
{
  stringstream ss;

  for (unsigned label = 0; label < BRIDGE_FORMAT_LABELS_SIZE; label++)
  {
    if (! labelMaxima[label])
      continue;

    ss << setw(8) << left << LABEL_NAMES[label];

    for (unsigned f = 0; f < BRIDGE_FORMAT_SIZE; f++)
      if (activeFormats[f])
        ss << setw(8) << right << stats[f][label].last_used();

    ss << setw(8) << right << labelMaxima[label] << "\n";
  }

  return ss.str();
}


string TextStats::str(const bool examplesFlag) const
{
  stringstream ss;

  if (examplesFlag)
  {
    // Show an example of each label.
    for (unsigned label = 0; label < BRIDGE_FORMAT_LABELS_SIZE; label++)
      ss << TextStats::strExamples(static_cast<Label>(label));

    ss << "\n";
  }

  // Add up across formats, preserve one example and source per size
  vector<size_t> activeFormats;
  vector<size_t> labelMaxima;
  TextStats::strPrepare(activeFormats, labelMaxima);

  ss << TextStats::strParamHeader(activeFormats);
  ss << TextStats::strParams(activeFormats, labelMaxima);

  return ss.str() + "\n";
}

