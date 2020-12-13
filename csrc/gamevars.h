#ifndef __gamevars_h__
#define __gamevars_h__

#include "ptoc.h"
#include "board.h"
#include "world.h"
#include <array>

const integer MAX_ELEMENT = 61;               /*E_TEXT_BLINK_WHITE;*/
const integer MAX_BOARD = 100;
const integer HIGH_SCORE_COUNT = 30;
const integer TORCH_DURATION = 200;
const integer TORCH_DX = 8;
const integer TORCH_DY = 5;
const integer TORCH_DIST_SQR = 50;

typedef varying_string<50> TString50;
struct TCoord {
        integer X;
        integer Y;
};

const std::array<std::string, 8> ColorNames =
{"Black", "Blue", "Green", "Cyan", "Red", "Purple", "Yellow", "White"};

typedef void(*TElementDrawProc)(integer x, integer y, byte& ch);
typedef void(*TElementTickProc)(integer statId);
typedef void(*TElementTouchProc)(integer x, integer y, integer sourceStatId, integer& deltaX, integer& deltaY);
struct TElementDef {
        char Character;
        byte Color;
        boolean Destructible;
        boolean Pushable;
        boolean VisibleInDark;
        boolean PlaceableOnTop;
        boolean Walkable;
        boolean HasDrawProc;
        TElementDrawProc DrawProc;
        integer Cycle;
        TElementTickProc TickProc;
        TElementTouchProc TouchProc;
        integer EditorCategory;
        char EditorShortcut;
        asciiz Name;
        asciiz CategoryName;
        asciiz Param1Name;
        asciiz Param2Name;
        asciiz ParamBulletTypeName;
        asciiz ParamBoardName;
        asciiz ParamDirName;
        asciiz ParamTextName;
        integer ScoreValue;
};

struct TEditorStatSetting {
        byte P1, P2, P3;
        integer StepX, StepY;
};

struct TWorld {
        integer BoardCount;
        // dynamic board length.
        std::array<std::vector<unsigned char>, MAX_BOARD> BoardData;
        /* KevEdit treats board length as unsigned, so to handle >32k
			  boards without corrupting subsequent ones... */
        array<0 , MAX_BOARD,word> BoardLen;
        TWorldInfo Info;
        array<0 , MAX_ELEMENT,TEditorStatSetting> EditorStatSettings;
};
struct THighScoreEntry {
        asciiz Name;
        integer Score;
};

typedef array<1 , HIGH_SCORE_COUNT,THighScoreEntry> THighScoreList;
//typedef array<0 , (MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1),byte> TIoTmpBuf;
// ptoc arrays can't be directly addressed in C, so this hack is necessary.
typedef byte TIoTmpBuf;

#ifdef __GameVars_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN integer PlayerDirX;
EXTERN integer PlayerDirY;
EXTERN integer unkVar_0476;
EXTERN integer unkVar_0478;

EXTERN array<1 , 80*25,TCoord> TransitionTable;
EXTERN TString50 LoadedGameFileName;
EXTERN TString50 SavedGameFileName;
EXTERN TString50 SavedBoardFileName;
EXTERN TString50 StartupWorldFileName;
EXTERN TBoard Board;
EXTERN TWorld World;
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
EXTERN array<0 , 14,byte> unkVar_4ABA;

EXTERN boolean GameTitleExitRequested;
EXTERN boolean GamePlayExitRequested;
EXTERN integer GameStateElement;
EXTERN integer ReturnBoardId;

EXTERN integer TransitionTableSize;
EXTERN byte TickSpeed;

EXTERN TIoTmpBuf* IoTmpBuf;

EXTERN array<0 , MAX_ELEMENT,TElementDef> ElementDefs;
EXTERN integer EditorPatternCount;
EXTERN array<1 , 10,byte> EditorPatterns;

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
EXTERN array<1 , 10,TString50> WorldFileDescKeys;
EXTERN array<1 , 10,TString50> WorldFileDescValues;
#undef EXTERN
#define EXTERN extern

const integer E_EMPTY = 0;
const integer E_BOARD_EDGE = 1;
const integer E_MESSAGE_TIMER = 2;
const integer E_MONITOR = 3; /* State - Title screen */
const integer E_PLAYER = 4; /* State - Playing */
const integer E_AMMO = 5;
const integer E_TORCH = 6;
const integer E_GEM = 7;
const integer E_KEY = 8;
const integer E_DOOR = 9;
const integer E_SCROLL = 10;
const integer E_PASSAGE = 11;
const integer E_DUPLICATOR = 12;
const integer E_BOMB = 13;
const integer E_ENERGIZER = 14;
const integer E_STAR = 15;
const integer E_CONVEYOR_CW = 16;
const integer E_CONVEYOR_CCW = 17;
const integer E_BULLET = 18;
const integer E_WATER = 19;
const integer E_FOREST = 20;
const integer E_SOLID = 21;
const integer E_NORMAL = 22;
const integer E_BREAKABLE = 23;
const integer E_BOULDER = 24;
const integer E_SLIDER_NS = 25;
const integer E_SLIDER_EW = 26;
const integer E_FAKE = 27;
const integer E_INVISIBLE = 28;
const integer E_BLINK_WALL = 29;
const integer E_TRANSPORTER = 30;
const integer E_LINE = 31;
const integer E_RICOCHET = 32;
const integer E_BLINK_RAY_EW = 33;
const integer E_BEAR = 34;
const integer E_RUFFIAN = 35;
const integer E_OBJECT = 36;
const integer E_SLIME = 37;
const integer E_SHARK = 38;
const integer E_SPINNING_GUN = 39;
const integer E_PUSHER = 40;
const integer E_LION = 41;
const integer E_TIGER = 42;
const integer E_BLINK_RAY_NS = 43;
const integer E_CENTIPEDE_HEAD = 44;
const integer E_CENTIPEDE_SEGMENT = 45;
const integer E_TEXT_BLUE = 47;
const integer E_TEXT_GREEN = 48;
const integer E_TEXT_CYAN = 49;
const integer E_TEXT_RED = 50;
const integer E_TEXT_PURPLE = 51;
const integer E_TEXT_YELLOW = 52;
const integer E_TEXT_WHITE = 53;
const integer E_TEXT_GREY = 54;
const integer E_TEXT_BLINK_BLUE = 55;
const integer E_TEXT_BLINK_GREEN = 56;
const integer E_TEXT_BLINK_CYAN = 57;
const integer E_TEXT_BLINK_RED = 58;
const integer E_TEXT_BLINK_PURPLE = 59;
const integer E_TEXT_BLINK_YELLOW = 60;
const integer E_TEXT_BLINK_WHITE = 61;
/**/
const integer E_TEXT_MIN = E_TEXT_BLUE;
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
