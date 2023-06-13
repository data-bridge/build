/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FORMATS_H
#define BRIDGE_FORMATS_H

using namespace std;

// Bridge file formats.

enum Format
{
  BRIDGE_FORMAT_LIN = 0,
  BRIDGE_FORMAT_LIN_RP = 1, // A la Pavlicek
  BRIDGE_FORMAT_LIN_VG = 2, // A la BBO Vugraph
  BRIDGE_FORMAT_LIN_TRN = 3, // A la BBO tournament play
  BRIDGE_FORMAT_PBN = 4,
  BRIDGE_FORMAT_RBN = 5,
  BRIDGE_FORMAT_RBX = 6,
  BRIDGE_FORMAT_TXT = 7,
  BRIDGE_FORMAT_EML = 8,
  BRIDGE_FORMAT_REC = 9,
  BRIDGE_FORMAT_PAR = 10, // Not a real file format
  BRIDGE_FORMAT_SIZE = 11
};

#endif
