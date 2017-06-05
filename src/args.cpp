/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// These functions parse the command line for options.


#include <iostream>
#include <iomanip>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "args.h"
#include "bconst.h"

using namespace std;


struct OptEntry
{
  string shortName;
  string longName;
  unsigned numArgs;
};

#define BRIDGE_NUM_OPTIONS 18

static const OptEntry OPT_LIST[BRIDGE_NUM_OPTIONS] =
{
  {"i", "infile", 1},
  {"I", "indir", 1},
  {"o", "outfile", 1},
  {"O", "outdir", 1},
  {"r", "reffile", 1},
  {"R", "refdir", 1},
  {"d", "digfile", 1},
  {"D", "digdir", 1},
  {"T", "tableIMP", 0},
  {"l", "logfile", 1},
  {"c", "compare", 0},
  {"p", "players", 0},
  {"f", "format", 1},
  {"w", "write", 1},
  {"s", "stats", 0},
  {"q", "quotes", 0},
  {"n", "threads", 1},
  {"v", "verbose", 1}
};

static string shortOptsAll, shortOptsWithArg;

static int getNextArgToken(
  int argc,
  char * argv[]);

static void setDefaults();

static void printFileOption(
  const FileOption& fopt,
  const string& text);

static void checkArgs(const Options& options);


void usage(
  const char base[])
{
  string basename(base);
  const size_t l = basename.find_last_of("\\/");
  if (l != string::npos)
    basename.erase(0, l+1);

  cout <<
    "Usage: " << basename << " [options]\n\n" <<
    "-i, --infile s     Input file.  Ending determines format.\n" <<
    "\n" <<
    "-I, --indir s      Input directory.  Read all eligible files.\n" <<
    "\n" <<
    "-o, --outfile s    Output file for -i.  Ending determines format.\n" <<
    "\n" <<
    "-O, --outdir s     Output base directory (may make subdirs).\n" <<
    "\n" <<
    "-r, --reffile s    Reference for -i.  Endings determine format.\n" <<
    "\n" <<
    "-R, --refdir s     Reference for -i and -I.\n" <<
    "\n" <<
    "-d, --digfile s    Produce a sheet of contracts and tricks.\n" <<
    "\n" <<
    "-D, --digdir s     Produce a sheet of contracts and tricks.\n" <<
    "\n" <<
    "-T, --tableIMP     Produce an IMP sheet.\n" <<
    "\n" <<
    "-l, --logfile s    Log file for outputs (default: stdout).\n" <<
    "\n" <<
    "-c, --compare      Re-read output and compare internally.\n" <<
    "                   (Default: not set)\n" <<
    "\n" <<
    "-p, --players      Flag incomplete tables of players.\n" <<
    "                   (Default: not set)\n" <<
    "\n" <<
    "-f, --format s     Output format for -O (default: ALL).\n" <<
    "                   Values LIN, PBN, RBN, TXT, EML, DOC, REC, ALL.\n" <<
    "                   Some dialects are set by the input filename.\n" <<
    "\n" <<
    "-w, --write s      Write .ref file for next run (default: 0).\n" <<
    "                   Values 0 (none), 1 (clear ones), 2 (all).\n" <<
    "\n" <<
    "-s, --stats        Output length stats of free-form fields.\n" <<
    "                   (Default: not set)\n" <<
    "\n" <<
    "-q, --quotes       Output stats of ref-file quotes.\n" <<
    "                   (Default: not set)\n" <<
    "\n" <<
    "-n, --threads n    Number of threads (default: 1).\n" <<
    "\n" <<
    "-v, -verbose n     Verbosity (default: 0x1a).  Bits:\n" <<
    "                   0x01: Show input/output file names.\n" <<
    "                   0x02: Show input error messages.\n" <<
    "                   0x04: Show input error details.\n" <<
    "                   0x08: Show validation issue type stats.\n" <<
    "                   0x10: Show an example of each issue.\n" <<
    endl;
}


static int nextToken = 1;
static char * optarg;

static int getNextArgToken(
  int argc,
  char * argv[])
{
  // 0 means done, -1 means error.

  if (nextToken >= argc)
    return 0;

  string str(argv[nextToken]);
  if (str[0] != '-' || str.size() == 1)
    return -1;

  if (str[1] == '-')
  {
    if (str.size() == 2)
      return -1;
    str.erase(0, 2);
  }
  else if (str.size() == 2)
    str.erase(0, 1);
  else
    return -1;

  for (unsigned i = 0; i < BRIDGE_NUM_OPTIONS; i++)
  {
    if (str == OPT_LIST[i].shortName || str == OPT_LIST[i].longName)
    {
      if (OPT_LIST[i].numArgs == 1)
      {
        if (nextToken+1 >= argc)
          return -1;

        optarg = argv[nextToken+1];
        nextToken += 2;
      }
      else
        nextToken++;

      return str[0];
    }
  }

  return -1;
}


static void setDefaults(Options& options)
{
  options.fileInput = {false, ""};
  options.dirInput = {false, ""};
  options.fileOutput = {false, ""};
  options.dirOutput = {false, ""};
  options.fileRef = {false, ""};
  options.dirRef = {false, ""};
  options.fileDigest = {false, ""};
  options.dirDigest = {false, ""};
  options.fileLog = {false, ""};

  options.tableIMPFlag = false;
  options.compareFlag = false;
  options.playersFlag = false;

  options.formatSetFlag = false;
  options.format = BRIDGE_FORMAT_SIZE;

  options.statsFlag = false;
  options.quoteFlag = false;

  options.refLevel = REF_LEVEL_NONE;

  options.numThreads = 1;

  options.verboseIO = false;
  options.verboseThrow = true;
  options.verboseBatch = false;
  options.verboseValStats = true;
  options.verboseValDetails = true;
}


static void printFileOption(
  const FileOption& fopt,
  const string& text)
{
  cout << setw(12) << text;
  if (fopt.setFlag)
    cout << setw(12) << fopt.name << "\n";
  else
    cout << setw(12) << "(not set)" << "\n";
}


void printOptions(const Options& options)
{
  cout << left;
  printFileOption(options.fileInput, "infile");
  printFileOption(options.dirInput, "indir");

  printFileOption(options.fileOutput, "outfile");
  printFileOption(options.dirOutput, "outdir");

  printFileOption(options.fileRef, "reffile");
  printFileOption(options.dirRef, "refdir");

  printFileOption(options.fileDigest, "digfile");
  printFileOption(options.dirDigest, "digdir");

  printFileOption(options.fileLog, "logfile");

  if (options.compareFlag)
    cout << setw(12) << "compare" << setw(12) << "set" << "\n";
  else
    cout << setw(12) << "compare" << setw(12) << "not set" << "\n";

  if (options.playersFlag)
    cout << setw(12) << "players" << setw(12) << "set" << "\n";
  else
    cout << setw(12) << "players" << setw(12) << "not set" << "\n";

  if (options.formatSetFlag)
  {
    cout << setw(12) << "format" << 
        setw(12) << FORMAT_NAMES[options.formatSetFlag] << "\n";
  }
  else
    cout << setw(12) << "(not set)\n";

  if (options.statsFlag)
    cout << setw(12) << "stats" << setw(12) << "set" << "\n";
  else
    cout << setw(12) << "stats" << setw(12) << "not set" << "\n";

  cout << setw(12) << "ref level" << setw(12) << options.refLevel << "\n\n";

  cout << setw(12) << "threads" << setw(12) << options.numThreads << "\n\n";
}


static void checkArgs(const Options& options)
{
  if (options.fileInput.setFlag && options.dirInput.setFlag)
  {
    cout << "Cannot use both -i and -I." << endl;
    exit(0);
  }

  if (! options.fileInput.setFlag && ! options.dirInput.setFlag)
  {
    cout << "Must use one of -i and -I." << endl;
    exit(0);
  }

  if (options.dirInput.setFlag && options.fileOutput.setFlag)
  {
    cout << "Cannot use -I with -o." << endl;
    exit(0);
  }

  if (options.fileOutput.setFlag && options.dirOutput.setFlag)
  {
    cout << "Cannot use both -o and -O." << endl;
    exit(0);
  }

  if (options.dirInput.setFlag && options.fileRef.setFlag)
  {
    cout << "Cannot use -I with -r." << endl;
    exit(0);
  }

  if (options.fileRef.setFlag && options.dirRef.setFlag)
  {
    cout << "Cannot use both -r and -R." << endl;
    exit(0);
  }

  if (options.fileDigest.setFlag && options.dirDigest.setFlag)
  {
    cout << "Cannot use both -d and -D." << endl;
    exit(0);
  }

  if (! options.fileOutput.setFlag && ! options.dirOutput.setFlag &&
      ! options.fileRef.setFlag && ! options.dirRef.setFlag &&
      ! options.fileDigest.setFlag && ! options.dirDigest.setFlag)
  {
    cout << "Need at least one of -o, -O, -r, -R, -d, -D." << endl;
    exit(0);
  }

  if (options.formatSetFlag &&
      (options.fileOutput.setFlag || options.fileRef.setFlag))
  {
    cout << "Cannot use -f with either -o or -r." << endl;
    exit(0);
  }
}


void readArgs(
  int argc,
  char * argv[],
  Options& options)
{
  for (unsigned i = 0; i < BRIDGE_NUM_OPTIONS; i++)
  {
    shortOptsAll += OPT_LIST[i].shortName;
    if (OPT_LIST[i].numArgs)
      shortOptsWithArg += OPT_LIST[i].shortName;
  }

  if (argc == 1)
  {
    usage(argv[0]);
    exit(0);
  }

  setDefaults(options);

  int c;
  long m;
  unsigned mu;
  bool errFlag = false, matchFlag;
  string stmp;
  char * temp;

  while ((c = getNextArgToken(argc, argv)) > 0)
  {
    switch(c)
    {
      case 'i':
        options.fileInput = {true, optarg};
        break;

      case 'I':
        options.dirInput = {true, optarg};
        break;

      case 'o':
        options.fileOutput = {true, optarg};
        break;

      case 'O':
        options.dirOutput = {true, optarg};
        break;

      case 'r':
        options.fileRef = {true, optarg};
        break;

      case 'R':
        options.dirRef = {true, optarg};
        break;

      case 'd':
        options.fileDigest = {true, optarg};
        break;

      case 'D':
        options.dirDigest = {true, optarg};
        break;

      case 'l':
        options.fileLog = {true, optarg};
        break;

      case 'T':
        options.tableIMPFlag = true;
        break;

      case 'c':
        options.compareFlag = true;
        break;

      case 'p':
        options.playersFlag = true;
        break;

      case 'f':
        matchFlag = false;
        for (unsigned i = 0; i < BRIDGE_FORMAT_SIZE; i++)
        {
          if (FORMAT_NAMES[i] != optarg)
            continue;

          matchFlag = true;
          options.formatSetFlag = true;
          options.format = static_cast<Format>(i);
          break;
        }

        if (! matchFlag)
        {
          cout << "Could not parse format\n";
          nextToken -= 2;
          errFlag = true;
        }
        break;

      case 's':
        options.statsFlag = true;
        break;

      case 'q':
        options.quoteFlag = true;
        break;

      case 'w':
        mu = static_cast<unsigned>(strtol(optarg, &temp, 0));
        if (temp == optarg || temp == '\0' || errno == ERANGE)
        {
          cout << "Could not parse write\n";
          nextToken -= 2;
          errFlag = true;
        }
        
        if (mu > REF_LEVEL_ALL)
        {
          cout << "numThreads out of range\n";
          nextToken -= 2;
          errFlag = true;
        }

        options.refLevel = static_cast<RefLevel>(mu);
        break;

      case 'n':
        mu = static_cast<unsigned>(strtol(optarg, &temp, 0));
        if (temp == optarg || temp == '\0' || errno == ERANGE)
        {
          cout << "Could not parse numThreads\n";
          nextToken -= 2;
          errFlag = true;
        }
        
        if (mu < 1 || mu > 16)
        {
          cout << "numThreads out of range\n";
          nextToken -= 2;
          errFlag = true;
        }

        options.numThreads = mu;
        break;

      case 'v':
        m = static_cast<int>(strtol(optarg, &temp, 0));
        if (temp == optarg || temp == '\0' ||
            ((m == LONG_MIN || m == LONG_MAX) && errno == ERANGE))
        {
          cout << "Could not parse verbose\n";
          nextToken -= 2;
          errFlag = true;
        }
        
        options.verboseIO = ((m & 0x01) != 0);
        options.verboseThrow = ((m & 0x02) != 0);
        options.verboseBatch = ((m & 0x04) != 0);
        options.verboseValStats = ((m & 0x08) != 0);
        options.verboseValDetails = ((m & 0x10) != 0);
        break;

      default:
        cout << "Unknown option\n";
        errFlag = true;
        break;
    }
    if (errFlag)
      break;
  }

  if (errFlag || c == -1)
  {
    cout << "Error while parsing option '" << argv[nextToken] << "'\n";
    cout << "Invoke the program without arguments for help" << endl;
    exit(0);
  }

  checkArgs(options);
}

