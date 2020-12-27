#include "ptoc.h"
#include "tools.h"

/*
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
*/

/*$I-*/

#include "hardware.h"
#include <unistd.h>
#include <iostream>
#include <sstream>

#include "gamevars.h"
#include "game.h"
#include "sounds.h"
#include "fileops.h"
#include "video.h"
#include "world.h"

TWorld World;

void ParseArguments() {
	integer i;
	string pArg;

	for (i = 1; i <= ParamCount; i ++) {
		pArg = ParamStr(i);
		if (pArg[1] == '/')  {
			switch (upcase(pArg[2])) {
				case 'T': {
					// TBD: sounds.pas
					/* SoundTimeCheckCounter = 0;
					UseSystemTimeForElapsed = false;*/
				}
				break;
				case 'R': ResetConfig = true; break;
			}
		} else {
			StartupWorldFileName = pArg;
			if ((length(StartupWorldFileName) > 4)
				&& (StartupWorldFileName[length(StartupWorldFileName) - 3] == '.'))  {
				StartupWorldFileName = copy(StartupWorldFileName, 1,
						length(StartupWorldFileName) - 4);
			}
		}
	}
}

void GameConfigure() {
	integer unk1;
	boolean joystickEnabled, mouseEnabled;
	integer bottomRow;

	ParsingConfigFile = true;
	EditorEnabled = true;
	ConfigRegistration = "";
	ConfigWorldFile = "";
	GameVersion = "3.2";

	std::ifstream cfgFile = OpenForRead("zzt.cfg");
	if (errno == 0)  {
		cfgFile >> ConfigWorldFile;
		cfgFile >> ConfigRegistration;
	}
	if (ConfigWorldFile[0] == '*')  {
		EditorEnabled = false;
		ConfigWorldFile = ConfigWorldFile.substr(1);
	}
	if (ConfigWorldFile.size() > 0)  {
		StartupWorldFileName = ConfigWorldFile.c_str();
	}
	cfgFile.close();

	// TBD: inputs.pas
	/*InputInitDevices();
	joystickEnabled = InputJoystickEnabled;
	mouseEnabled = InputMouseEnabled;*/

	/* Define the bottom row of the 80x25 terminal layout, or
	the bottom of the screen if it's smaller. */
	bottomRow = WindMaxY - WindMinY;
	if (bottomRow > 25) {
		bottomRow = 25;
	}

	ParsingConfigFile = false;

	Window(1, 1, 80, bottomRow+1);

	TextBackground(Black);
	ClrScr();
	TextColor(White);
	TextColor(White);
	display_writeln("");
	display_writeln("                                 <=-  ZZT  -=>");
	TextColor(Yellow);
	if (ConfigRegistration.size() == 0) {
		display_writeln("                             Shareware version 3.2");
	} else {
		display_writeln("                                  Version  3.2");
	}
	display_writeln("                            Created by Tim Sweeney");
	TextColor(LightGray);
	if (bottomRow < 24) {
		display_writeln("                        Best played in 80x25 or larger.");
	}
	GotoXY(1, 7);
	TextColor(Blue);
	display_writeln("================================================================================");
	GotoXY(1, bottomRow);
	display_writeln("================================================================================");
	TextColor(White);
	GotoXY(30, 7);
	display_write(" Game Configuration ");
	GotoXY(1, bottomRow+1);
	display_writeln(" Copyright (c) 1991 Epic MegaGames                         Press ... to abort");
	TextColor(Black);
	TextBackground(LightGray);
	GotoXY(66, bottomRow+1);
	display_write("ESC");
	/*Window(1, 8, 80, bottomRow-2);
	TextColor(Yellow);
	TextBackground(Black);
	ClrScr();
	TextColor(Yellow);*/
	// TBD: video.pas
	GotoXY(1, 8);
	TextColor(LightGreen);
	TextBackground(Black);
	if (! video.VideoConfigure()) {
		GameTitleExitRequested = true;
	}

	Window(1, 1, 80, bottomRow+1);
}

void debug_video_input() {
	int col = 0;

	do {
		InputReadWaitKey();
		video.VideoWriteText(10, 10, col + 0x09,
			"Key read: " + itos(InputKeyPressed) + " delta " + itos(
				InputDeltaX) + "," + itos(InputDeltaY));
		//video.VideoWriteText(10, 10, 0x0F, "Key read: " + itos(ReadKeyBlocking().key));
		++col;
		col = col % 7;
	} while (1 == 1);
}

void debug_display_output() {
	for (int bg = 0; bg < 16; ++bg) {
		for (int fg = 0; fg < 16; ++fg) {
			display->print_ch(fg+10, bg+5, bg * 16 + fg, '!');
		}
	}
	InputReadWaitKey();
}

int main(int argc, const char* argv[]) {
	pio_initialize(argc, argv);
	WorldFileDescCount = 7;
	WorldFileDescKeys[1] = "TOWN";
	WorldFileDescValues[1] = "TOWN       The Town of ZZT";
	WorldFileDescKeys[2] = "DEMO";
	WorldFileDescValues[2] = "DEMO       Demo of the ZZT World Editor";
	WorldFileDescKeys[3] = "CAVES";
	WorldFileDescValues[3] = "CAVES      The Caves of ZZT";
	WorldFileDescKeys[4] = "DUNGEONS";
	WorldFileDescValues[4] = "DUNGEONS   The Dungeons of ZZT";
	WorldFileDescKeys[5] = "CITY";
	WorldFileDescValues[5] = "CITY       Underground City of ZZT";
	WorldFileDescKeys[6] = "BEST";
	WorldFileDescValues[6] = "BEST       The Best of ZZT";
	WorldFileDescKeys[7] = "TOUR";
	WorldFileDescValues[7] = "TOUR       Guided Tour ZZT\47s Other Worlds";

	initCurses();
	video.VideoInstall(80, Blue, display);

	Randomize();
	SetCBreak(false);
	SetupCodepointToCP437();

	// Back up the text attributes. This shouldn't be necessary because
	// curses cleans up after itself.
	//InitialTextAttr = TextAttr;

	StartupWorldFileName = "TOWN";
	ResourceDataFileName = "ZZT.DAT";
	ResetConfig = false;
	GameTitleExitRequested = false;
	GameConfigure();
	ParseArguments();
	TextWindowInit(5, 3, 50, 18);

	/*debug_display_input();*/
	/*debug_display_output();*/

	if (!GameTitleExitRequested)  {
		TTextWindowState textWindow;

		// XXX: Fix
		//OrderPrintId = &GameVersion;
		IoTmpBuf = new byte[(MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1)+1];
		//new TIoTmpBuf;

		video.VideoHideCursor();
		ClrScr();

		TickSpeed = 4;
		DebugEnabled = false;
		SavedGameFileName = "SAVED";
		SavedBoardFileName = "TEMP";
		GenerateTransitionTable();
		WorldCreate();

		GameTitleLoop();

		//LEAKFIX: Remember to dispose of *everything* in use.
		WorldUnload();
		delete[] IoTmpBuf;
	}

	SoundUninstall();
	SoundClearQueue();

	/*VideoUninstall();
	TextAttr = InitialTextAttr;
	ClrScr();*/

	if (ConfigRegistration.size() == 0)  {
		GamePrintRegisterMessage();
	} else {
		display_writeln("");
		display_writeln("  Registered version -- Thank you for playing ZZT.");
		display_writeln("");
	}

	video.VideoShowCursor();
	return EXIT_SUCCESS;
}
