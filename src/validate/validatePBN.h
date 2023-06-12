/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALPBN_H
#define BRIDGE_VALPBN_H

class Chunk;
class ValProfile;


bool validatePBN(
  ValState& valState,
  ValProfile& prof);

bool validatePBNChunk(
  const Chunk& chunkRef,
  const Chunk& chunkOut,
  ValState& valState,
  ValProfile& prof);

#endif
