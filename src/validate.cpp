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
#include "parse.h"
#include "Bexcept.h"


using namespace std;


// Types of differences that can occur and be OK (except "ERROR").

enum ValDiffs
{
  BRIDGE_VAL_TITLE = 0,
  BRIDGE_VAL_DATE = 1,
  BRIDGE_VAL_LOCATION = 2,
  BRIDGE_VAL_EVENT = 3,
  BRIDGE_VAL_SESSION = 4,
  BRIDGE_VAL_SCORING = 5,
  BRIDGE_VAL_TEAMS = 6,

  BRIDGE_VAL_NAMES8 = 7,
  BRIDGE_VAL_TXT_ALL_PASS = 8,
  BRIDGE_VAL_LIN_EXCLAIM = 9,
  BRIDGE_VAL_LIN_PLAY_NL = 10,
  BRIDGE_VAL_REC_PLAY_SHORT = 11,
  BRIDGE_VAL_REC_MADE_32 = 12,
  BRIDGE_VAL_VG_CHAT = 13,
  BRIDGE_VAL_RECORD_NUMBER = 14,

  BRIDGE_VAL_ERROR = 15,
  BRIDGE_VAL_OUT_SHORT = 16,
  BRIDGE_VAL_REF_SHORT = 17,
  BRIDGE_VAL_SIZE = 18
};

const string valNames[] =
{
  "Title",
  "Date",
  "Location",
  "Event",
  "Session",
  "Scoring",
  "Teams",

  "Names8",
  "All-pass",
  "Lin-!",
  "Play-newline",
  "Play-short",
  "VG-chat",
  "Rec-comment",

  "Error",
  "Out-short",
  "Ref-short"
};

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


// A single example of a difference of any kind.

struct ValExample
{
  string lineOut;
  string lineRef;

  unsigned lnoOut;
  unsigned lnoRef;
};

// Counts of differences between two files, with the first examples.

struct ValFileStats
{
  unsigned counts[BRIDGE_VAL_SIZE];
  ValExample examples[BRIDGE_VAL_SIZE];
};


static void resetStats(ValFileStats& stats);

static void splitIntoWords(
  const string& line,
  vector<string>& words);

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
    stats.examples[v].lineOut = "";
    stats.examples[v].lineRef = "";
  }
}


static void splitIntoWords(
  const string& line,
  vector<string>& words)
{
  // Split into words (split on \s+, effectively).
  unsigned pos = 0;
  unsigned startPos = 0;
  bool isSpace = true;
  const unsigned l = line.length();

  while (pos < l)
  {
    if (line.at(pos) == ' ')
    {
      if (! isSpace)
      {
        words.push_back(line.substr(startPos, pos-startPos));
        isSpace = true;
      }
    }
    else if (isSpace)
    {
      isSpace = false;
      startPos = pos;
    }
    pos++;
  }

  if (! isSpace)
    words.push_back(line.substr(startPos, pos-startPos));
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


static bool isTXTAllPass(
  const string& lineOut,
  const string& lineRef,
  string& expectLine)
{
  vector<string> wordsRef;
  splitIntoWords(lineRef, wordsRef);
  unsigned lRef = wordsRef.size();
  if (lRef > 4)
    return false;

  vector<string> wordsOut;
  splitIntoWords(lineOut, wordsOut);
  unsigned lOut = wordsOut.size();
  if (lOut < 2 || lOut+2 < lRef || lOut > lRef+1)
    return false;

  if (wordsOut[lOut-2] != "All" || wordsOut[lOut-1] != "Pass")
    return false;

  for (unsigned i = lOut-2; i < lRef; i++)
    if (wordsRef[i] != "Pass")
      return false;

  for (unsigned i = 0; i < lOut-2; i++)
  {
    if (wordsRef[i] != wordsOut[i])
      return false;
  }

  if (lOut == 2 && lRef == 4)
    return true;

  unsigned pos = 0;
  while (pos < lineOut.length() && lineOut.at(pos) == ' ')
    pos++;
  if (pos == lineOut.length())
    return false;

  unsigned expectPasses = lOut + 1 - lRef;
  if (pos > 0 && lineOut.at(pos) == 'A')
    expectPasses++;

  expectLine = "";
  for (unsigned i = 0; i < expectPasses; i++)
  {
    expectLine += "Pass";
    if (i != expectPasses-1)
      expectLine += "        ";
  }
  return true;
}


static bool isLINLongRefLine(
  const string& lineOut, 
  const string& lineRef, 
  string& expectLine)
{
  const unsigned lOut = lineOut.length();
  const unsigned lRef = lineRef.length();

  if (lOut >= lRef)
    return false;

  const string sOut = lineOut.substr(0, 3);
  const string sRef = lineRef.substr(0, 3);

  if ((sOut != "mb|" && sOut != "pf|") || sOut != sRef)
    return false;

  if (lineRef.substr(0, lOut) == lineOut)
  {
    expectLine = lineRef.substr(lOut);
    return true;
  }

  // Can lop off "pg||" at end of lineOut and try again.
  if (lineOut.substr(lOut-4) != "pg||")
    return false;

  if (lineOut.substr(0, lOut-4) == lineRef.substr(0, lOut-4))
  {
    expectLine = lineRef.substr(lOut-4);
    return true;
  }
  else
    return false;
}


static bool isLINLongOutLine(
  const string& lineOut, 
  const string& lineRef, 
  string& expectLine)
{
  const unsigned lOut = lineOut.length();
  const unsigned lRef = lineRef.length();

  if (lRef >= lOut)
    return false;

  const string sOut = lineOut.substr(0, 3);
  const string sRef = lineRef.substr(0, 3);

  if ((sOut != "mb|" && sOut != "pf|") || sOut != sRef)
    return false;

  if (lineOut.substr(0, lRef) == lineRef)
  {
    expectLine = lineOut.substr(lRef);
    return true;
  }

  // Can lop off "pg||" at end of lineRef and try again.
  if (lineRef.substr(lRef-4) != "pg||")
    return false;

  if (lineRef.substr(0, lRef-4) == lineOut.substr(0, lRef-4))
  {
    expectLine = lineOut.substr(lRef-4);
    return true;
  }
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


void validate(
  const string& fileOut,
  const string& fileRef,
  const formatType formatOrig,
  const formatType formatRef,
  ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE])
{
  ifstream fostr(fileOut.c_str());
  if (! fostr.is_open())
    THROW("Cannot read from: " + fileOut);

  ifstream frstr(fileRef.c_str());
  if (! frstr.is_open())
    THROW("Cannot read from: " + fileRef);

  ValFileStats stats;
  resetStats(stats);

  ValExample running;
  running.lineOut = "";
  running.lineRef = "";
  running.lnoOut = 0;
  running.lnoRef = 0;
  bool keepLineOut = false;
  bool keepLineRef = false;

  while (keepLineRef || getline(frstr, running.lineRef))
  {
    if (! keepLineRef)
      running.lnoRef++;
    if (! keepLineOut && ! getline(fostr, running.lineOut))
    {
      stats.counts[BRIDGE_VAL_OUT_SHORT]++;
      break;
    }
    if (! keepLineOut)
      running.lnoOut++;

    if (running.lineRef == running.lineOut)
    {
      keepLineRef = false;
      keepLineOut = false;
      continue;
    }
// printExample(running);

    if (formatRef == BRIDGE_FORMAT_LIN_RP)
    {
      string expectLine;
      if (isLINLongRefLine(running.lineOut, running.lineRef, expectLine))
      {
        if (! getline(fostr, running.lineOut))
        {
          stats.counts[BRIDGE_VAL_OUT_SHORT]++;
          break;
        }

        running.lnoOut++;

        if (running.lineOut == expectLine)
        {
          // No newline when no play (Pavlicek error).
          if (stats.examples[BRIDGE_VAL_LIN_PLAY_NL].lineOut == "")
            stats.examples[BRIDGE_VAL_LIN_PLAY_NL] = running;
          stats.counts[BRIDGE_VAL_LIN_PLAY_NL]++;
          continue;
        }
      }
      else if (isLINLongOutLine(running.lineOut, running.lineRef, 
          expectLine))
      {
        if (! getline(frstr, running.lineRef))
        {
          stats.counts[BRIDGE_VAL_REF_SHORT]++;
          break;
        }

        running.lnoRef++;

        if (running.lineRef == expectLine)
        {
          // No newline when no play (Pavlicek error).
          if (stats.examples[BRIDGE_VAL_LIN_PLAY_NL].lineOut == "")
            stats.examples[BRIDGE_VAL_LIN_PLAY_NL] = running;
          stats.counts[BRIDGE_VAL_LIN_PLAY_NL]++;
          continue;
        }
      }
    }
    else if (formatRef == BRIDGE_FORMAT_TXT)
    {
      string expectLine;
      if (isTXTAllPass(running.lineOut, running.lineRef, expectLine))
      {
        // Reference does not have "All Pass" (Pavlicek error).
        if (expectLine != "" && ! getline(frstr, running.lineRef))
        {
          stats.counts[BRIDGE_VAL_OUT_SHORT]++;
          break;
        }

        if (expectLine == "" || running.lineRef == expectLine)
        {
          if (stats.examples[BRIDGE_VAL_TXT_ALL_PASS].lineOut == "")
            stats.examples[BRIDGE_VAL_TXT_ALL_PASS] = running;
          stats.counts[BRIDGE_VAL_TXT_ALL_PASS]++;
          continue;
        }
      }
    }
    else if (formatRef == BRIDGE_FORMAT_REC)
    {
      if (running.lineRef == "")
      {
        if (isRECPlay(running.lineOut))
        {
          // Extra play line (Pavlicek error).
          if (stats.examples[BRIDGE_VAL_REC_PLAY_SHORT].lineOut == "")
            stats.examples[BRIDGE_VAL_REC_PLAY_SHORT] = running;
          stats.counts[BRIDGE_VAL_REC_PLAY_SHORT]++;

          keepLineRef = true;
          continue;
        }
      }
      else if (isRECJustMade(running.lineOut, running.lineRef))
      {
        // "Won 32" (Pavlicek error, should be "Made 0" or so.
        if (stats.examples[BRIDGE_VAL_REC_MADE_32].lineOut == "")
          stats.examples[BRIDGE_VAL_REC_MADE_32] = running;
        stats.counts[BRIDGE_VAL_REC_MADE_32]++;

        // The next line (Score, Points) is then also different.
        getline(fostr, running.lineOut);
        getline(frstr, running.lineRef);
        running.lnoOut++;
        running.lnoRef++;
        continue;
      }
    }

    // General: % line numbers (Pavlicek error).
    if (isRecordComment(running.lineOut, running.lineRef))
    {
      if (stats.examples[BRIDGE_VAL_RECORD_NUMBER].lineOut == "")
        stats.examples[BRIDGE_VAL_RECORD_NUMBER] = running;
      stats.counts[BRIDGE_VAL_RECORD_NUMBER]++;
      continue;
    }


// printExample(running);
    stats.counts[BRIDGE_VAL_ERROR]++;
    if (stats.examples[BRIDGE_VAL_ERROR].lineOut == "")
      stats.examples[BRIDGE_VAL_ERROR] = running;

    keepLineOut = false;
    keepLineRef = false;
  }

  if (getline(fostr, running.lineOut))
  {
    stats.examples[BRIDGE_VAL_REF_SHORT] = running;
    stats.counts[BRIDGE_VAL_REF_SHORT]++;
  }

  printFileStats(stats, fileOut);

  statsToVstat(stats, vstats[formatOrig][formatRef]);

  fostr.close();
  frstr.close();
}


static void statsToVstat(
  const ValFileStats& stats,
  ValStatType& vstat)
{
  bool expectedFlag = false;
  for (unsigned v = 0; v < BRIDGE_VAL_ERROR; v++)
  {
    if (stats.counts[v])
    {
      expectedFlag = true;
      break;
    }
  }

  bool errorFlag = false;
  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v])
    {
      errorFlag = true;
      break;
    }
  }

  vstat.numFiles++;

  if (expectedFlag)
  {
    vstat.numExpectedDiffs++;
    if (errorFlag)
      vstat.numErrors++;
  }
  else if (errorFlag)
    vstat.numErrors++;
  else
    vstat.numIdentical++;
}


static void printExample(const ValExample& ex)
{
  cout << "Out (" << setw(4) << ex.lnoOut << "): " << ex.lineOut << "\n";
  cout << "Ref (" << setw(4) << ex.lnoRef << "): " << ex.lineRef << "\n";
}


static void printFileStats(
  ValFileStats& stats,
  const string& fname)
{
  bool showFlag = false;
  // for (unsigned v = 0; v < BRIDGE_VAL_SIZE; v++)
  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v])
    {
      showFlag = true;
      break;
    }
  }

  if (! showFlag)
    return;
  
  cout << "File stats: " << fname << "\n";
  // for (unsigned v = 0; v < BRIDGE_VAL_SIZE; v++)
  for (unsigned v = BRIDGE_VAL_ERROR; v < BRIDGE_VAL_SIZE; v++)
  {
    if (stats.counts[v] == 0)
      continue;
    
    cout << setw(12) << left << valNames[v] << 
        setw(4) << right << stats.counts[v] << "\n";
  }
  cout << "\n";

  // for (unsigned v = 0; v < BRIDGE_VAL_SIZE; v++)
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


void printOverallStats(
  ValStatType vstats[][BRIDGE_FORMAT_LABELS_SIZE])
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

    cout << setw(7) << left << "";
    for (auto &g: formatActive)
      cout << setw(7) << right << posOrDash(vstats[f][g].numExpectedDiffs);
    cout << "\n";

    cout << setw(7) << left << "";
    for (auto &g: formatActive)
      cout << setw(7) << right << posOrDash(vstats[f][g].numErrors);
    cout << "\n\n";
  }
}

