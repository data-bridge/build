/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#include <iomanip>
#include <fstream>

#include "validate.h"
#include "funcValidate.h"
#include "Bexcept.h"


void dispatchValidate(
  const FileTask& task,
  const FileOutputTask& otask,
  const Options& options,
  const string& text,
  ValStats& vstats,
  ostream& flog)
{
  try
  {
    validate(text, otask.fileOutput, otask.fileRef,
      task.formatInput, otask.formatOutput, options, vstats);
  }
  catch (Bexcept& bex)
  {
    flog << "Files " << task.fileInput << " -> " <<
      otask.fileOutput << endl;
    bex.print(flog);
  }
}

