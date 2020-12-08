#ifndef __elements_h__
#define __elements_h__

#include "gamevars.h"

void ElementMove(integer oldX, integer oldY, integer newX, integer newY);
void ElementPushablePush(integer x, integer y, integer deltaX,
                         integer deltaY);
void DrawPlayerSurroundings(integer x, integer y, integer bombPhase);
void GamePromptEndPlay();
void ResetMessageNotShownFlags();
void InitElementsEditor();
void InitElementsGame();
void InitEditorStatSettings();

#endif
