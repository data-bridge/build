/* 
   Part of BridgeData.

   Copyright (C) 2016-17 by Soren Hein.

   See LICENSE and README.
*/

#ifndef BRIDGE_VALRBN_H
#define BRIDGE_VALRBN_H

class ValProfile;


bool validateRBN(
  ValState& valState,
  ValProfile& prof);

bool validateRBX(
  ValState& valState,
  ValProfile& prof);

#endif
