#ifndef __gamevars_h__
#define __gamevars_h__

#include "ptoc.h"
#include "board.h"
#include <array>
#include "random.h"

const integer HIGH_SCORE_COUNT = 30;
const integer TORCH_DURATION = 200;
const integer TORCH_DX = 8;
const integer TORCH_DY = 5;
const integer TORCH_DIST_SQR = 50;
const integer MAX_CYCLE = 420;

typedef varying_string<50> TString50;
struct TCoord {
	integer X;
	integer Y;
};

const std::array<std::string, 8> ColorNames =
{"Black", "Blue", "Green", "Cyan", "Red", "Purple", "Yellow", "White"};

struct TEditorStatSetting {
	byte P1, P2, P3;
	integer StepX, StepY;
};

struct THighScoreEntry {
	asciiz Name;
	integer Score;
};

typedef array<1, HIGH_SCORE_COUNT,THighScoreEntry> THighScoreList;
//typedef array<0 , (MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1),byte> TIoTmpBuf;
// ptoc arrays can't be directly addressed in C, so this hack is necessary.
typedef byte TIoTmpBuf;

#ifdef __GameVars_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN integer PlayerDirX;
EXTERN integer PlayerDirY;

EXTERN rng rnd;

EXTERN array<1, 80*25,TCoord> TransitionTable;
EXTERN TString50 LoadedGameFileName;
EXTERN TString50 SavedGameFileName;
EXTERN TString50 SavedBoardFileName;
EXTERN TString50 StartupWorldFileName;
EXTERN boolean MessageAmmoNotShown;
EXTERN boolean MessageOutOfAmmoNotShown;
EXTERN boolean MessageNoShootingNotShown;
EXTERN boolean MessageTorchNotShown;
EXTERN boolean MessageOutOfTorchesNotShown;
EXTERN boolean MessageRoomNotDarkNotShown;
EXTERN boolean MessageHintTorchNotShown;
EXTERN boolean MessageForestNotShown;
EXTERN boolean MessageFakeNotShown;
EXTERN boolean MessageGemNotShown;
EXTERN boolean MessageEnergizerNotShown;

EXTERN boolean GameTitleExitRequested;
EXTERN boolean GamePlayExitRequested;
EXTERN integer GameStateElement;
EXTERN integer ReturnBoardId;

EXTERN integer TransitionTableSize;
EXTERN byte TickSpeed;

EXTERN TIoTmpBuf* IoTmpBuf;

EXTERN std::array<TElementDef, MAX_ELEMENT+1> ElementDefs;
EXTERN TElementDef out_of_bounds_element;

EXTERN integer EditorPatternCount;
EXTERN array<1, 10,byte> EditorPatterns;

EXTERN integer TickTimeDuration;
EXTERN integer CurrentTick;
EXTERN integer CurrentStatTicked;
EXTERN boolean GamePaused;
EXTERN integer TickTimeCounter;

EXTERN boolean ForceDarknessOff;
EXTERN byte InitialTextAttr;

EXTERN char OopChar;
EXTERN varying_string<20> OopWord;
EXTERN integer OopValue;

EXTERN boolean DebugEnabled;

EXTERN THighScoreList HighScoreList;
EXTERN std::string ConfigRegistration;
EXTERN std::string ConfigWorldFile;
EXTERN boolean EditorEnabled;
EXTERN TString50 GameVersion;
EXTERN boolean ParsingConfigFile;
EXTERN boolean ResetConfig; /* This flag is a remnant from ZZT 3.0. */
EXTERN boolean JustStarted;

EXTERN integer WorldFileDescCount;
EXTERN array<1, 10,TString50> WorldFileDescKeys;
EXTERN array<1, 10,TString50> WorldFileDescValues;
#undef EXTERN
#define EXTERN extern

/**/
const integer CATEGORY_ITEM = 1;
const integer CATEGORY_CREATURE = 2;
const integer CATEGORY_TERRAIN = 3;
/**/
const integer COLOR_SPECIAL_MIN = 0xf0;
const integer COLOR_CHOICE_ON_BLACK = 0xff;
const integer COLOR_WHITE_ON_CHOICE = 0xfe;
const integer COLOR_CHOICE_ON_CHOICE = 0xfd;
/**/
const integer SHOT_SOURCE_PLAYER = 0;
const integer SHOT_SOURCE_ENEMY = 1;

#endif
