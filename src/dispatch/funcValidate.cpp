/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

// The functions in this file help to parse files.


#pragma warning(push)
#pragma warning(disable: 4365 4571 4625 4626 4774 5026 5027)
#include <iomanip>
#include <fstream>
#pragma warning(pop)

#include "funcValidate.h"

#include "../stats/ValStats.h"

#include "../Bexcept.h"

#include "../validate/validate.h"


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

