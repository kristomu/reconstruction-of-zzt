{
	Copyright (c) 2020 Adrian Siekierka

	Based on a reconstruction of code from ZZT,
	Copyright 1991 Epic MegaGames, used with permission.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
}

unit GameVars;

{Every record that is to be saved must be packed; otherwise, Pascal may waste
extra space and make incompatible records, which then makes board and game
files incompatible as well.}

interface
	const
		MAX_STAT = 150;
		MAX_CYCLE = 420;		{ maximum cycle number before reset }
		MAX_ELEMENT = 61; 		{E_TEXT_BLINK_WHITE;}
		MAX_BOARD = 100;
		MAX_FLAG = 10;
		BOARD_WIDTH = 60;
		BOARD_HEIGHT = 25;
		HIGH_SCORE_COUNT = 30;
		TORCH_DURATION = 200;
		TORCH_DX = 8;
		TORCH_DY = 5;
		TORCH_DIST_SQR = 50;
		MAX_BOARD_LEN = 20000;

		ERR_STATID_TOO_HIGH = 400;
		ERR_STATID_DOESNT_EXIST = 401;

		ERR_NO_PLAYER = 500;	{ Invariant violation. }

		{ Fired if ZZT uses more memory than was available on DOS.
		  For debugging purposes. }
		ERR_MEMORY_EXCEEDED = 600;
	type
		TString50 = string[50];
		TFileName = string[255];
		TCoord = packed record
			X: integer;
			Y: integer;
		end;
		TTile = packed record
			Element: byte;
			Color: byte;
		end;
		TElementDrawProc = procedure(x, y: integer; var ch: byte);
		TElementTickProc = procedure(statId: integer);
		TElementTouchProc = procedure(x, y: integer; sourceStatId: integer; var deltaX, deltaY: integer);
		TElementDef = record
			Character: char;
			Color: byte;
			Destructible: boolean;
			Pushable: boolean;
			VisibleInDark: boolean;
			PlaceableOnTop: boolean;
			Walkable: boolean;
			HasDrawProc: boolean;
			DrawProc: TElementDrawProc;
			Cycle: integer;
			TickProc: TElementTickProc;
			TouchProc: TElementTouchProc;
			EditorCategory: integer;
			EditorShortcut: char;
			Name: string[20];
			CategoryName: string[20];
			Param1Name: string[20];
			Param2Name: string[20];
			ParamBulletTypeName: string[20];
			ParamBoardName: string[20];
			ParamDirName: string[20];
			ParamTextName: string[20];
			ScoreValue: integer;
		end;
		{ Since pointers are 8 bytes on Linux, we have to rearrange
		  things in the stat structure to make room. Fortunately, we
		  have eight bytes of padding, and the Data structure is never
		  used in saved games or worlds, so we can simply move the
		  pointer all the way to the end. It won't be that simple if we
		  ever try to support Super ZZT, though... }
		TStat = packed record
			X, Y: byte;
			StepX, StepY: integer;
			Cycle: integer;
			P1, P2, P3: byte;
			Follower: integer;
			Leader: integer;
			Under: TTile;
			unk1: array[0 .. 3] of byte;
			DataPos: integer;
			DataLen: integer;
			Data: ^byte;
		end;
		TRleTile = packed record
			Count: byte;
			Tile: TTile;
		end;
		TBoardInfo = packed record
			MaxShots: byte;
			IsDark: boolean;
			NeighborBoards: array[0 .. 3] of byte;
			ReenterWhenZapped: boolean;
			Message: string[58];
			StartPlayerX: byte;
			StartPlayerY: byte;
			TimeLimitSec: integer;
			unk1: array[70 .. 85] of byte;
		end;
		TWorldInfo = packed record
			Ammo: integer;
			Gems: integer;
			Keys: array [1..7] of boolean;
			Health: integer;
			CurrentBoard: integer;
			Torches: integer;
			TorchTicks: integer;
			EnergizerTicks: integer;
			unk1: integer;
			Score: integer;
			Name: string[20];
			Flags: array[1 .. MAX_FLAG] of string[20];
			BoardTimeSec: integer;
			BoardTimeHsec: integer;
			IsSave: boolean;
			unkPad: array[0 .. 13] of byte;
		end;
		TEditorStatSetting = packed record
			P1, P2, P3: byte;
			StepX, StepY: integer;
		end;
		TBoard = packed record
			Name: TString50;
			Tiles: array[0 .. BOARD_WIDTH + 1] of array[0 .. BOARD_HEIGHT + 1] of TTile;
			StatCount: integer;
			Stats: array[0 .. MAX_STAT + 1] of TStat;
			Info: TBoardInfo;
		end;
		TWorld = packed record
			BoardCount: integer;
			BoardData: array[0 .. MAX_BOARD] of pointer;
			{ KevEdit treats board length as unsigned, so to handle >32k
			  boards without corrupting subsequent ones... }
			BoardLen: array[0 .. MAX_BOARD] of word;
			Info: TWorldInfo;
			EditorStatSettings: array[0 .. MAX_ELEMENT] of TEditorStatSetting;
		end;
		THighScoreEntry = packed record
			Name: string[50];
			Score: integer;
		end;
		THighScoreList = array[1 .. HIGH_SCORE_COUNT] of THighScoreEntry;
	const
		{ This is used to make sure IoTmpBuf is always large enough to hold
	      what changes may happen to the board. In the worst case, the board
	      is completely empty when loaded but takes max space due to stats,
	      then during play, the board gets filled with random tiles. That
	      will require MAX_RLE_OVERFLOW additional bytes to hold. So by
	      dimensioning IoTmpBuf with an excess of MAX_RLE_OVERFLOW, we ensure
	  	  that it can never run out of space from RLE shenanigans. }
		MAX_RLE_OVERFLOW = BOARD_WIDTH * BOARD_HEIGHT * SizeOf(TRleTile);
	type
		TIoTmpBuf = array[0 .. (MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1)] of byte;
	var
		PlayerDirX: integer;
		PlayerDirY: integer;
		unkVar_0476: integer;
		unkVar_0478: integer;

		TransitionTable: array[1 .. 80*25] of TCoord;
		LoadedGameFileName: TFileName;
		SavedGameFileName: TFileName;
		SavedBoardFileName: TFileName;
		StartupWorldFileName: TFileName;
		Board: TBoard;
		World: TWorld;
		MessageAmmoNotShown: boolean;
		MessageOutOfAmmoNotShown: boolean;
		MessageNoShootingNotShown: boolean;
		MessageTorchNotShown: boolean;
		MessageOutOfTorchesNotShown: boolean;
		MessageRoomNotDarkNotShown: boolean;
		MessageHintTorchNotShown: boolean;
		MessageForestNotShown: boolean;
		MessageFakeNotShown: boolean;
		MessageGemNotShown: boolean;
		MessageEnergizerNotShown: boolean;
		unkVar_4ABA: array[0 .. 14] of byte;

		GameTitleExitRequested: boolean;
		GamePlayExitRequested: boolean;
		GameStateElement: integer;
		ReturnBoardId: integer;

		TransitionTableSize: integer;
		TickSpeed: byte;

		IoTmpBuf: ^TIoTmpBuf;

		ElementDefs: array[0 .. MAX_ELEMENT] of TElementDef;
		EditorPatternCount: integer;
		EditorPatterns: array[1 .. 10] of byte;

		TickTimeDuration: integer;
		CurrentTick: integer;
		CurrentStatTicked: integer;
		GamePaused: boolean;
		TickTimeCounter: integer;

		ForceDarknessOff: boolean;
		InitialTextAttr: byte;

		OopChar: char;
		OopWord: string[20];
		OopValue: integer;

		DebugEnabled: boolean;

		HighScoreList: THighScoreList;
		ConfigRegistration: string;
		ConfigWorldFile: TString50;
		EditorEnabled: boolean;
		GameVersion: TString50;
		ParsingConfigFile: boolean;
		ResetConfig: boolean; { This flag is a remnant from ZZT 3.0. }
		JustStarted: boolean;

		WorldFileDescCount: integer;
		WorldFileDescKeys: array[1 .. 10] of TString50;
		WorldFileDescValues: array[1 .. 10] of TString50;
	const
		E_EMPTY = 0;
		E_BOARD_EDGE = 1;
		E_MESSAGE_TIMER = 2;
		E_MONITOR = 3; { State - Title screen }
		E_PLAYER = 4; { State - Playing }
		E_AMMO = 5;
		E_TORCH = 6;
		E_GEM = 7;
		E_KEY = 8;
		E_DOOR = 9;
		E_SCROLL = 10;
		E_PASSAGE = 11;
		E_DUPLICATOR = 12;
		E_BOMB = 13;
		E_ENERGIZER = 14;
		E_STAR = 15;
		E_CONVEYOR_CW = 16;
		E_CONVEYOR_CCW = 17;
		E_BULLET = 18;
		E_WATER = 19;
		E_FOREST = 20;
		E_SOLID = 21;
		E_NORMAL = 22;
		E_BREAKABLE = 23;
		E_BOULDER = 24;
		E_SLIDER_NS = 25;
		E_SLIDER_EW = 26;
		E_FAKE = 27;
		E_INVISIBLE = 28;
		E_BLINK_WALL = 29;
		E_TRANSPORTER = 30;
		E_LINE = 31;
		E_RICOCHET = 32;
		E_BLINK_RAY_EW = 33;
		E_BEAR = 34;
		E_RUFFIAN = 35;
		E_OBJECT = 36;
		E_SLIME = 37;
		E_SHARK = 38;
		E_SPINNING_GUN = 39;
		E_PUSHER = 40;
		E_LION = 41;
		E_TIGER = 42;
		E_BLINK_RAY_NS = 43;
		E_CENTIPEDE_HEAD = 44;
		E_CENTIPEDE_SEGMENT = 45;
		E_TEXT_BLUE = 47;
		E_TEXT_GREEN = 48;
		E_TEXT_CYAN = 49;
		E_TEXT_RED = 50;
		E_TEXT_PURPLE = 51;
		E_TEXT_YELLOW = 52;
		E_TEXT_WHITE = 53;
		E_TEXT_GREY = 54;
		E_TEXT_BLINK_BLUE = 55;
		E_TEXT_BLINK_GREEN = 56;
		E_TEXT_BLINK_CYAN = 57;
		E_TEXT_BLINK_RED = 58;
		E_TEXT_BLINK_PURPLE = 59;
		E_TEXT_BLINK_YELLOW = 60;
		E_TEXT_BLINK_WHITE = 61;
		{}
		E_TEXT_MIN = E_TEXT_BLUE;
		{}
		CATEGORY_ITEM = 1;
		CATEGORY_CREATURE = 2;
		CATEGORY_TERRAIN = 3;
		{}
		COLOR_SPECIAL_MIN = $F0;
		COLOR_CHOICE_ON_BLACK = $FF;
		COLOR_WHITE_ON_CHOICE = $FE;
		COLOR_CHOICE_ON_CHOICE = $FD;
		{}
		SHOT_SOURCE_PLAYER = 0;
		SHOT_SOURCE_ENEMY = 1;

implementation

begin
end.
