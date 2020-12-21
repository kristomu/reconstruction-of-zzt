#ifndef __oop_h__
#define __oop_h__

#include "gamevars.h"

integer WorldGetFlagPosition(TString50 name);
void WorldSetFlag(TString50 name);
void WorldClearFlag(TString50 name);
boolean OopSend(integer statId, string sendLabel, boolean ignoreLock);
void OopExecute(integer statId, integer& position, TString50 name);

#endif
