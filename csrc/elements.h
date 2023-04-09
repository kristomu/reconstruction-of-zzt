#pragma once

#include "elements/indices.h"
#include "elements/info.h"

#include <cstddef>
#include <memory>
#include <string>
#include <array>

typedef unsigned char boolean;

typedef void(*TElementDrawProc)(short x, short y, unsigned char & ch);
typedef void(*TElementTickProc)(short statId);
typedef void(*TElementTouchProc)(short x, short y,
	short sourceStatId, short & deltaX, short & deltaY);

struct TElementProcDef {
	boolean HasDrawProc;
	TElementDrawProc DrawProc;
	TElementTickProc TickProc;
	TElementTouchProc TouchProc;
};

extern std::array<TElementProcDef, MAX_ELEMENT+1> ElementProcDefs;

// See elements.cxx TODO for the reason that this is not a const pointer
extern std::shared_ptr<ElementInfo> elem_info_ptr;

void ElementMove(short oldX, short oldY, short newX, short newY);
void ElementPushablePush(short x, short y, short deltaX,
	short deltaY);
void DrawPlayerSurroundings(short x, short y, short bombPhase);
void GamePromptEndPlay();
void ResetMessageNotShownFlags();
void InitElementsEditor();
void InitElementsGame();
void InitEditorStatSettings();