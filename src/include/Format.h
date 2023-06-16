/* 
   Part of BridgeData.

   Copyright (C) 2016-23 by Soren Hein.

   See LICENSE and README.
*/


#ifndef BRIDGE_FORMATS_H
#define BRIDGE_FORMATS_H

#include <vector>
#include <string>

using namespace std;

// Bridge file formats.


enum Format: unsigned
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

const string FORMAT_NAMES[BRIDGE_FORMAT_SIZE] =
{
  "LIN",
  "LIN-RP",
  "LIN-VG",
  "LIN-TRN",
  "PBN",
  "RBN",
  "RBX",
  "TXT",
  "EML",
  "REC",
  "PAR"
};

const vector<Format> FORMAT_ACTIVE =
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

const string FORMAT_EXTENSIONS[BRIDGE_FORMAT_SIZE] =
{
  "LIN",
  "LIN",
  "LIN",
  "LIN",
  "PBN", 
  "RBN", 
  "RBX", 
  "TXT", 
  "EML",
  "REC",
  "PAR"
};


const Format FORMAT_INPUT_MAP[] =
{
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_LIN,
  BRIDGE_FORMAT_PBN,
  BRIDGE_FORMAT_RBN,
  BRIDGE_FORMAT_RBX,
  BRIDGE_FORMAT_TXT,
  BRIDGE_FORMAT_EML,
  BRIDGE_FORMAT_REC,
  BRIDGE_FORMAT_SIZE,
  BRIDGE_FORMAT_SIZE
};

#endif
