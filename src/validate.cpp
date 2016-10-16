/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <regex>

#include "validate.h"
#include "valint.h"
#include "validateLIN.h"
#include "validatePBN.h"
#include "validateRBN.h"
#include "validateTXT.h"
#include "parse.h"
#include "Bexcept.h"


using namespace std;


const vector<formatType> formatActive =
{
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN_RP,
  BRIDGE_FORMAT_LIN_VG,
  BRIDGE_FORMAT_LIN_TRN,
  BRIDGE_FORMAT_PBN,
  BRIDGE_FORMAT_RBN,
  BRIDGE_FORMAT_RBX,
  BRIDGE_FORMAT_TXT,
  BRIDGE_FORMAT_EML,
  BRIDGE_FORMAT_REC
};


static void resetStats(ValFileStats& stats);

static bool isRECPlay(const string& line);

static bool isTXTAllPass(
  const string& lineOut,
  const string& lineRef,
  string& expectLine);

static bool isLINLongLine(
  const string& lineOut, 
  const string& lineRef, 
  string& expectLine);

static bool isRecordComment(
  const string& lineOut,
  const string& lineRef);

static void statsToVstat(
  const ValFileStats& stats,
  ValStatType& vstat);

static void printExample(const ValExample& ex);

static string posOrDash(const unsigned u);

static void printFileStats(
  ValFileStats& stats,
  const string& fname);



static void resetStats(ValFileStats& stats)
{
  for (unsigned v = 0; v < BRIDGE_VAL_SIZE; v++)
  {
    stats.counts[v] = 0;
    stats.examples[v].out.line = "";
    stats.examples[v].ref.line = "";
  }
}


static bool isRECPlay(const string& line)
{
  vector<string> words;
  splitIntoWords(line, words);

  if (words.size() < 3 || words.size() > 6)
    return false;

  unsigned u;
  if (! StringToNonzeroUnsigned(words[0], u))
    return false;
  if (u > 13)
    return false;

  for (unsigned i = 2; i < words.size(); i++)
  {
    if (words[i].length() > 3)
      return false;
  }

  // We could also check word[1] for South/East/North/West etc.

  return true;
}

static bool isRECJustMade(
  const string &lineOut, 
  const string &lineRef)
{
  unsigned lOut = lineOut.length();
  unsigned lRef = lineRef.length();

  if (lOut < 32 || lRef != lOut)
    return false;

  if (lineOut.substr(28) == "Made 0" && lineRef.substr(28) == "Won 32")
    return true;
  else
    return false;
}


static bool isRecordComment(
  const string& lineOut,
  const string& lineRef)
{
  // Detect Pavlicek bug with wrong record numbers.

  if (lineOut == "" || lineRef == "")
    return false;

  if (lineOut.at(0) != '%' || lineRef.at(0) != '%')
    return false;
  
  regex re("(\\d+) records (\\d+) deals");
  smatch match;
  unsigned r1, d1, r2, d2;

  if (! regex_search(lineOut, match, re))
    return false;

  if (! StringToUnsigned(match.str(1), r1)) return false;
  if (! StringToUnsigned(match.str(2), d1)) return false;

  if (! regex_search(lineRef, match, re))
    return false;

  if (! StringToUnsigned(match.str(1), r2)) return false;
  if (! StringToUnsigned(match.str(2), d2)) return false;

  if (r2 <= r1 || d2 <= d1)
    return false;

  return true;
}


enum refFixType
{
  BRIDGE_REF_INSERT = 0,
  BRIDGE_REF_REPLACE = 1,
  BRIDGE_REF_DELETE = 2
};

struct RefFix
{
  unsigned lno; // First line is 1
  refFixType type;
  string value;
  unsigned count;
};


static void readRefFix(
  const string& fname,
  vector<RefFix>& refFix)
{
  regex re("\\.\\w+$");
  string refName = regex_replace(fname, re, ".ref");

  // There might not be a .ref file (not an error).
  ifstream refstr(refName.c_str());
  if (! refstr.is_open())
    return;

  string line, s;
  RefFix rf;
  regex rer("^\\s*\"(.*)\"\\s*$");
  smatch match;
  while (getline(refstr, line))
  {
    if (line.empty() || line.at(0) == '%')
      continue;

    if (! GetNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (! StringToUnsigned(s, rf.lno))
      THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      
    if (! GetNextWord(line, s))
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    if (s == "insert")
    {
      rf.type = BRIDGE_REF_INSERT;
      if (! regex_search(line, match, rer) || match.size() < 1)
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

      rf.value = match.str(1);
    }
    else if (s == "replace")
    {
      rf.type = BRIDGE_REF_REPLACE;
      if (! regex_search(line, match, rer) || match.size() < 1)
        THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

      rf.value = match.str(1);
    }
    else if (s == "delete")
    {
      rf.type = BRIDGE_REF_DELETE;
      if (GetNextWord(line, s))
      {
        if (! StringToUnsigned(s, rf.count))
          THROW("Ref file " + refName + ": Bad number in '" + line + "'");
      }
      else
        rf.count = 1;
    }
    else
      THROW("Ref file " + refName + ": Syntax error in '" + line + "'");

    refFix.push_back(rf);
  }
  refstr.close();
}


bool valProgress(
  ifstream& fstr,
  valSide& side)
{
  if (! getline(fstr, side.line))
    return false;

  side.lno++;
  return true;
}


void valError(
  ValFileStats& stats,
  const ValExample& running,
  const ValDiffs label)
{
  // Only keep the first example of each kind.
  if (stats.examples[label].out.line == "")
    stats.examples[label] = running;

  stats.counts[label]++;
}


void validate(
  const string& fileOut,
  const string& fileRef,
  const formatType formatOrig,
  const formatType formatRef,
  const OptionsType& options,
  ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE])
{
  ifstream fostr(fileOut.c_str());
  if (! fostr.is_open())
    THROW("Cannot read from: " + fileOut);

  ifstream frstr(fileRef.c_str());
  if (! frstr.is_open())
    THROW("Cannot read from: " + fileRef);

  vector<RefFix> refFix;
  readRefFix(fileRef.c_str(), refFix);

  ValFileStats stats;
  resetStats(stats);

  ValExample running;
  running.out.line = "";
  running.ref.line = "";
  running.out.lno = 0;
  running.ref.lno = 0;
  bool keepLineOut = false;
  bool keepLineRef = false;
  unsigned emptyState = 0;

  while (keepLineRef || valProgress(frstr, running.ref))
  {
    if (! keepLineOut && ! valProgress(fostr, running.out))
    {
      stats.counts[BRIDGE_VAL_OUT_SHORT]++;
      break;
    }

    if (refFix.size() > 0 && refFix[0].lno == running.ref.lno)
    {
      if (refFix[0].type == BRIDGE_REF_INSERT)
      {
        if (refFix[0].value != running.out.line)
        {
          // This will mess up everything that follows...
          valError(stats, running, BRIDGE_VAL_ERROR);
          if (! valProgress(fostr, running.out))
            THROW("Next line is not there");
          keepLineRef = false;
          keepLineOut = false;
          continue;
        }

        // Get the next out line to compare with the ref line.
        if (! valProgress(fostr, running.out))
          THROW("Next line is not there");
      }
      else if (refFix[0].type == BRIDGE_REF_REPLACE)
      {
        running.ref.line = refFix[0].value;
      }
      else
      {
        for (unsigned i = 0; i < refFix[0].count; i++)
        {
          if (! valProgress(frstr, running.ref))
            THROW("Skip line is not there");
        }
      }

      refFix.erase(refFix.begin());
    }

    if (formatRef == BRIDGE_FORMAT_TXT &&
        running.out.line.substr(0, 5) == "-----")
      emptyState = 0;

    if (running.ref.line == running.out.line)
    {
      keepLineRef = false;
      keepLineOut = false;
      continue;
    }
// printExample(running);

    // General: % line numbers (Pavlicek error).
    if (isRecordComment(running.out.line, running.ref.line))
    {
      valError(stats, running, BRIDGE_VAL_RECORD_NUMBER);
      continue;
    }
    else if (formatRef == BRIDGE_FORMAT_LIN_RP)
    {
      if (validateLIN_RP(frstr, fostr, running, stats))
      {
        // Fix is already recorded in stats.
        continue;
      }
      else if (fostr.eof() || frstr.eof())
        break;

      // If not, fall through to general error.
    }
    else if (formatRef == BRIDGE_FORMAT_PBN)
    {
      if (validatePBN(frstr, running, stats))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_RBN)
    {
      if (validateRBN(frstr, running, stats))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_TXT)
    {
      if (validateTXT(frstr, running, emptyState, stats))
        continue;
      else if (fostr.eof() || frstr.eof())
        break;
    }
    else if (formatRef == BRIDGE_FORMAT_REC)
    {
      if (running.ref.line == "")
      {
        if (isRECPlay(running.out.line))
        {
          // Extra play line (Pavlicek error).
          valError(stats, running, BRIDGE_VAL_PLAY_SHORT);

          keepLineRef = true;
          continue;
        }
      }
      else if (isRECJustMade(running.out.line, running.ref.line))
      {
        // "Won 32" (Pavlicek error, should be "Made 0" or so.
        valError(stats, running, BRIDGE_VAL_REC_MADE_32);

        // The next line (Score, Points) is then also different.
        valProgress(fostr, running.out);
        valProgress(frstr, running.ref);
        continue;
      }
    }


// printExample(running);
    valError(stats, running, BRIDGE_VAL_ERROR);

    keepLineOut = false;
    keepLineRef = false;
  }

  if (valProgress(fostr, running.out))
  {
    stats.examples[BRIDGE_VAL_REF_SHORT] = running;
    stats.counts[BRIDGE_VAL_REF_SHORT]++;
  }

  if (options.verboseValDetails)
    printFileStats(stats, fileOut);

  statsToVstat(stats, vstats[formatOrig][formatRef]);

  fostr.close();
  frstr.close();
}


static void statsToVstat(
  const ValFileStats& stats,
  ValStatType& vstat)
{
  bool minorErrorFlag = false;
  for (unsigned v = 0; v < BRIDGE_VAL_ERROR; v++)
  {
    if (stats.counts[v])
    {
      minorErrorFlag = true;
      vstat.details[v]++;
    }
  }

  bool majorErrorFlag = false;
  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v])
    {
      majorErrorFlag = true;
      vstat.details[v]++;
    }
  }

  vstat.numFiles++;

  if (minorErrorFlag)
  {
    vstat.numExpectedDiffs++;
    if (majorErrorFlag)
      vstat.numErrors++;
  }
  else if (majorErrorFlag)
    vstat.numErrors++;
  else
    vstat.numIdentical++;

}


static void printExample(const ValExample& ex)
{
  cout << "Out (" << setw(4) << ex.out.lno << "): " << ex.out.line << "\n";
  cout << "Ref (" << setw(4) << ex.ref.lno << "): " << ex.ref.line << "\n";
}


static void printFileStats(
  ValFileStats& stats,
  const string& fname)
{
  // Only shows examples of bad errors.

  bool showFlag = false;
  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v])
    {
      showFlag = true;
      break;
    }
  }

  if (! showFlag)
  {
    cout.flush();
    return;
  }
  
  cout << "File stats: " << fname << "\n";
  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v] == 0)
      continue;
    
    cout << setw(12) << left << valNames[v] << 
        setw(4) << right << stats.counts[v] << "\n";
  }
  cout << "\n";

  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v] == 0)
      continue;
    
    cout << valNames[v] << ":\n";
    printExample(stats.examples[v]);
    cout << "\n";
  }
  cout << "\n";
}


static string posOrDash(const unsigned u)
{
  if (u == 0)
    return "-";
  else
    return STR(u);
}


static bool rowHasEntries(
  const ValStatType vstat[],
  const unsigned label)
{
  for (auto &g: formatActive)
    if (vstat[g].details[label] > 0)
      return true;
  return false;
}


static void printRow(
  const ValStatType vstat[],
  const unsigned label)
{
  cout << "  " << setw(5) << left << valNamesShort[label];
  for (auto &g: formatActive)
    cout << setw(7) << right << posOrDash(vstat[g].details[label]);
  cout << "\n";
}


void printOverallStats(
  const ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE],
  const bool detailsFlag)
{
  cout << setw(7) << "";
  for (auto &f: formatActive)
    cout << setw(7) << right << FORMAT_NAMES[f];
  cout << "\n\n";

  for (auto &f: formatActive)
  {
    cout << setw(7) << left << FORMAT_NAMES[f];
    for (auto &g: formatActive)
      cout << setw(7) << right << posOrDash(vstats[f][g].numFiles);
    cout << "\n";

    if (detailsFlag)
    {
      for (unsigned v = 0; v < BRIDGE_VAL_ERROR; v++)
      {
        if (rowHasEntries(vstats[f], v))
          printRow(vstats[f], v);
      }
    }

    cout << "  MINOR";
    for (auto &g: formatActive)
      cout << setw(7) << right << posOrDash(vstats[f][g].numExpectedDiffs);
    cout << "\n";

    if (detailsFlag)
    {
      for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
      {
        if (rowHasEntries(vstats[f], v))
          printRow(vstats[f], v);
      }
    }

    cout << "  MAJOR";
    for (auto &g: formatActive)
      cout << setw(7) << right << posOrDash(vstats[f][g].numErrors);
    cout << "\n\n";
  }
}

