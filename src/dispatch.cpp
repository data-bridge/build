/* 
   Part of BridgeData.

   Copyright (C) 2016 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <iterator>
#include <assert.h>

#include "Group.h"
#include "dispatch.h"

#include "fileLIN.h"
#include "filePBN.h"
#include "fileRBN.h"
#include "fileRBX.h"
#include "fileEML.h"
#include "fileTXT.h"
#include "fileREC.h"

#include "bconst.h"
#include "debug.h"
#include "portab.h"

extern Debug debug;



using namespace std;

struct FormatFunctionsType
{
  void (* set)();
  bool (* read)(Group&, const string&);
  bool (* write)(Group&, const string&);
};

FormatFunctionsType formatFncs[BRIDGE_FORMAT_SIZE];


static void dummySet()
{
}


static bool dummyRead(
  Group& group,
  const string& fname)
{
  UNUSED(group);
  UNUSED(fname);
  return true;
}


static bool dummyWrite(
  Group& group,
  const string& fname)
{
  UNUSED(group);
  UNUSED(fname);
  return true;
}


void setTables()
{
  formatFncs[BRIDGE_FORMAT_LIN].set = &setLINtables;
  formatFncs[BRIDGE_FORMAT_LIN].read = &readLIN;
  formatFncs[BRIDGE_FORMAT_LIN].write = &writeLIN;

  formatFncs[BRIDGE_FORMAT_LIN_RP].set = &dummySet;
  formatFncs[BRIDGE_FORMAT_LIN_RP].read = &readLIN;
  formatFncs[BRIDGE_FORMAT_LIN_RP].write = &writeLIN_RP;

  formatFncs[BRIDGE_FORMAT_LIN_VG].set = &dummySet;
  formatFncs[BRIDGE_FORMAT_LIN_VG].read = &readLIN;
  formatFncs[BRIDGE_FORMAT_LIN_VG].write = &writeLIN_VG;

  formatFncs[BRIDGE_FORMAT_LIN_TRN].set = &dummySet;
  formatFncs[BRIDGE_FORMAT_LIN_TRN].read = &readLIN;
  formatFncs[BRIDGE_FORMAT_LIN_TRN].write = &writeLIN_TRN;

  formatFncs[BRIDGE_FORMAT_LIN_EXT].set = &dummySet;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].read = &readLIN;
  formatFncs[BRIDGE_FORMAT_LIN_EXT].write = &dummyWrite;

  formatFncs[BRIDGE_FORMAT_PBN].set = &setPBNtables;
  formatFncs[BRIDGE_FORMAT_PBN].read = &readPBN;
  formatFncs[BRIDGE_FORMAT_PBN].write = &writePBN;

  formatFncs[BRIDGE_FORMAT_RBN].set = &setRBNtables;
  formatFncs[BRIDGE_FORMAT_RBN].read = &readRBN;
  formatFncs[BRIDGE_FORMAT_RBN].write = &writeRBN;

  formatFncs[BRIDGE_FORMAT_RBX].set = &setRBXtables;
  formatFncs[BRIDGE_FORMAT_RBX].read = &readRBX;
  formatFncs[BRIDGE_FORMAT_RBX].write = &writeRBX;

  formatFncs[BRIDGE_FORMAT_TXT].set = &setTXTtables;
  formatFncs[BRIDGE_FORMAT_TXT].read = &readTXT;
  formatFncs[BRIDGE_FORMAT_TXT].write = &writeTXT;

  formatFncs[BRIDGE_FORMAT_EML].set = &setEMLtables;
  formatFncs[BRIDGE_FORMAT_EML].read = &readEML;
  formatFncs[BRIDGE_FORMAT_EML].write = &writeEML;

  formatFncs[BRIDGE_FORMAT_REC].set = &setRECtables;
  formatFncs[BRIDGE_FORMAT_REC].read = &readREC;
  formatFncs[BRIDGE_FORMAT_REC].write = &writeREC;

  formatFncs[BRIDGE_FORMAT_PAR].set = &dummySet;
  formatFncs[BRIDGE_FORMAT_PAR].read = &dummyRead;
  formatFncs[BRIDGE_FORMAT_PAR].write = &dummyWrite;

  for (unsigned f = 0; f< BRIDGE_FORMAT_SIZE; f++)
    (* formatFncs[f].set)();
}


void dispatch(
  const int thrNo,
  Files& files)
{
  UNUSED(thrNo);

  FileTaskType task;
  if (! files.GetNextTask(task))
    return;

  Group group;

  if (! (* formatFncs[task.formatInput].read)(group, task.fileInput))
  {
    debug.Print();
    assert(false);
  }

  for (auto &t: task.taskList)
  {
    if (! (* formatFncs[t.formatOutput].write)(group, t.fileOutput))
    {
      debug.Print();
      assert(false);
    }

    if (t.refFlag)
    {
      // Compare magic on t.fileInput and t.fileOutput: TODO
    }

    if (task.removeOutputFlag)
    {
      // delete t.fileOutput: TODO
    }
  }

}
