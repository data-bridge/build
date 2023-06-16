/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <fstream>

#include "../control/Options.h"

#include "funcDigest.h"

#include "../records/Sheet.h"

#include "../files/FileTask.h"

#include "../parse.h"

#include "../handling/Bexcept.h"


void dispatchDigest(
  const FileTask& task,
  const Options& options,
  ostream& flog)
{
  if (FORMAT_INPUT_MAP[task.formatInput] != BRIDGE_FORMAT_LIN)
    return;

  try
  {
    Sheet sheet;
    sheet.read(task.fileInput);
    const string st = sheet.str();
    if (st != "")
    {
      ofstream dlog;
      if (options.fileDigest.setFlag)
        dlog.open(options.fileDigest.name);
      else
      {
        const string base = basefile(task.fileInput);
	const string dout = options.dirDigest.name + "/" +
	  changeExt(base, ".sht");
        dlog.open(dout);
      }
      dlog << st;
      dlog.close();
    }
  }
  catch (Bexcept& bex)
  {
    bex.print(flog);
    cout << "Came from " << task.fileInput << "\n";
  }
}

