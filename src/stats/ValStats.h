/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/

/*
   This class uses classes ValRow and ValStat.
   It generates a summary of errors by input and output format
   as follows:

           LIN LIN-RP LIN-VGLIN-TRN    PBN    RBN    RBX    ...
LIN        745      -      -      -      -      -      -    ...
  an        88      -      -      -      -      -      -    ...
  ...
  
   where the columns are reference formats, and the rows are
   original formats.  The rows are further divided into a couple
   of groups.
*/

#ifndef BRIDGE_VALSTATS_H
#define BRIDGE_VALSTATS_H

#include <string>

#include "ValRow.h"

class ValProfile;

enum Format: unsigned;

using namespace std;


class ValStats
{
  private:

    ValRow stats[BRIDGE_FORMAT_LABELS_SIZE];

    string strDetails(
      const unsigned lower,
      const unsigned upper,
      const Format fOrig) const;

  public:

    ValStats();

    void reset();

    void add(
      const Format formatOrig,
      const Format formatRef,
      const ValProfile& prof);

    void operator += (const ValStats& vs2);
      
    string str(const bool detailFlag = true) const;
};

#endif

