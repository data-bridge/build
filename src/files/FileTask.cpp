/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#include <iostream>
#include <iomanip>
#include <sstream>

#include "FileTask.h"


string FileTask::str() const
{
  stringstream ss;
  ss << setw(16) << left << "File input" << fileInput << "\n";
  ss << setw(16) << left << "Format input" << formatInput << "\n";
  ss << setw(16) << left << "Remove output" << removeOutputFlag << "\n";
  ss << "\n";

  for (size_t i = 0; i < taskList.size(); i++)
  {
    const auto& foref = taskList[i];
    ss << "  " << setw(14) << "Number" << i << "\n" <<
      "  " << setw(14) << "File output" << foref.fileOutput << "\n" <<
      "  " << setw(14) << "Format output" << foref.formatOutput << "\n" <<
      "  " << setw(14) << "Ref flag" << foref.refFlag << "\n" <<
      "  " << setw(14) << "File ref" << foref.fileRef << "\n\n";
  }
  return ss.str();
}

