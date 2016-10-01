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

#include "debug.h"
#include "portab.h"

extern Debug debug;



using namespace std;


void setTables()
{
  setLINtables();
  setPBNtables();
  setRBNtables();
  setRBXtables();
  setEMLtables();
  setTXTtables();
  setRECtables();
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
  switch(task.formatInput)
  {
    case BRIDGE_FORMAT_LIN:
      if (! readLIN(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    case BRIDGE_FORMAT_PBN:
      if (! readPBN(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    case BRIDGE_FORMAT_RBN:
      if (! readRBN(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    case BRIDGE_FORMAT_RBX:
      if (! readRBX(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    case BRIDGE_FORMAT_EML:
      if (! readEML(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    case BRIDGE_FORMAT_TXT:
      if (! readTXT(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    case BRIDGE_FORMAT_REC:
      if (! readREC(group, task.fileInput))
      {
        debug.Print();
        assert(false);
      }
      break;

    default:
      cout << "Unknown input format" << endl;
      exit(0);
  }

  for (auto &t: task.taskList)
  {
    switch(t.formatOutput)
    {
      case BRIDGE_FORMAT_LIN:
        if (! writeLIN(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_LIN_VG:
        if (! writeLIN_VG(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_LIN_RP:
        if (! writeLIN_RP(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_LIN_TRN:
        if (! writeLIN_TRN(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_PBN:
        if (! writePBN(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_RBN:
        if (! writeRBN(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_RBX:
        if (! writeRBX(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_EML:
        if (! writeEML(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_TXT:
        if (! writeTXT(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      case BRIDGE_FORMAT_REC:
        if (! writeREC(group, t.fileOutput))
        {
          debug.Print();
          assert(false);
        }
        break;

      default:
        cout << "Unknown output format" << endl;
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
