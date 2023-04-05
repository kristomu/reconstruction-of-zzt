#pragma once

#include <cstddef>
#include <string>
#include <array>

typedef unsigned char boolean;
typedef unsigned char element_t;

const size_t MAX_ELEMENT = 61;			/*E_TEXT_BLINK_WHITE;*/

const element_t E_EMPTY = 0;
const element_t E_BOARD_EDGE = 1;
const element_t E_MESSAGE_TIMER = 2;
const element_t E_MONITOR = 3; /* State - Title screen */
const element_t E_PLAYER = 4; /* State - Playing */
const element_t E_AMMO = 5;
const element_t E_TORCH = 6;
const element_t E_GEM = 7;
const element_t E_KEY = 8;
const element_t E_DOOR = 9;
const element_t E_SCROLL = 10;
const element_t E_PASSAGE = 11;
const element_t E_DUPLICATOR = 12;
const element_t E_BOMB = 13;
const element_t E_ENERGIZER = 14;
const element_t E_STAR = 15;
const element_t E_CONVEYOR_CW = 16;
const element_t E_CONVEYOR_CCW = 17;
const element_t E_BULLET = 18;
const element_t E_WATER = 19;
const element_t E_FOREST = 20;
const element_t E_SOLID = 21;
const element_t E_NORMAL = 22;
const element_t E_BREAKABLE = 23;
const element_t E_BOULDER = 24;
const element_t E_SLIDER_NS = 25;
const element_t E_SLIDER_EW = 26;
const element_t E_FAKE = 27;
const element_t E_INVISIBLE = 28;
const element_t E_BLINK_WALL = 29;
const element_t E_TRANSPORTER = 30;
const element_t E_LINE = 31;
const element_t E_RICOCHET = 32;
const element_t E_BLINK_RAY_EW = 33;
const element_t E_BEAR = 34;
const element_t E_RUFFIAN = 35;
const element_t E_OBJECT = 36;
const element_t E_SLIME = 37;
const element_t E_SHARK = 38;
const element_t E_SPINNING_GUN = 39;
const element_t E_PUSHER = 40;
const element_t E_LION = 41;
const element_t E_TIGER = 42;
const element_t E_BLINK_RAY_NS = 43;
const element_t E_CENTIPEDE_HEAD = 44;
const element_t E_CENTIPEDE_SEGMENT = 45;
const element_t E_TEXT_BLUE = 47;
const element_t E_TEXT_GREEN = 48;
const element_t E_TEXT_CYAN = 49;
const element_t E_TEXT_RED = 50;
const element_t E_TEXT_PURPLE = 51;
const element_t E_TEXT_YELLOW = 52;
const element_t E_TEXT_WHITE = 53;
const element_t E_TEXT_GREY = 54;
const element_t E_TEXT_BLINK_BLUE = 55;
const element_t E_TEXT_BLINK_GREEN = 56;
const element_t E_TEXT_BLINK_CYAN = 57;
const element_t E_TEXT_BLINK_RED = 58;
const element_t E_TEXT_BLINK_PURPLE = 59;
const element_t E_TEXT_BLINK_YELLOW = 60;
const element_t E_TEXT_BLINK_WHITE = 61;
/**/
const element_t E_TEXT_MIN = E_TEXT_BLUE;

typedef void(*TElementDrawProc)(short x, short y, unsigned char & ch);
typedef void(*TElementTickProc)(short statId);
typedef void(*TElementTouchProc)(short x, short y,
	short sourceStatId, short & deltaX, short & deltaY);

struct TElementDef {
	char Character;
	char Color;
	boolean Destructible;
	boolean Pushable;
	boolean VisibleInDark;
	boolean PlaceableOnTop;
	boolean Walkable;
	boolean HasDrawProc;
	TElementDrawProc DrawProc;
	short Cycle;
	TElementTickProc TickProc;
	TElementTouchProc TouchProc;
	short EditorCategory;
	char EditorShortcut;
	std::string Name;
	std::string CategoryName;
	std::string Param1Name;
	std::string Param2Name;
	std::string ParamBulletTypeName;
	std::string ParamBoardName;
	std::string ParamDirName;
	std::string ParamTextName;
	short ScoreValue;
};

extern std::array<TElementDef, MAX_ELEMENT+1> ElementDefs;
extern TElementDef out_of_bounds_element;

void ElementMove(short oldX, short oldY, short newX, short newY);
void ElementPushablePush(short x, short y, short deltaX,
	short deltaY);
void DrawPlayerSurroundings(short x, short y, short bombPhase);
void GamePromptEndPlay();
void ResetMessageNotShownFlags();
void InitElementsEditor();
void InitElementsGame();
void InitEditorStatSettings();