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

	video.TextBackground(Black);
	video.ClrScr();
	video.TextColor(White);
	video.TextColor(White);
	video.writeln("");
	video.writeln("                                 <=-  ZZT  -=>");
	video.TextColor(Yellow);
	if (ConfigRegistration.size() == 0) {
		video.writeln("                             Shareware version 3.2");
	} else {
		video.writeln("                                  Version  3.2");
	}
	video.writeln("                            Created by Tim Sweeney");
	video.TextColor(LightGray);
	if (bottomRow < 24) {
		video.writeln("                        Best played in 80x25 or larger.");
	}
	video.go_to_xy(0, 6);
	video.TextColor(Blue);
	video.writeln("================================================================================");
	video.go_to_xy(0, bottomRow-1);
	video.writeln("================================================================================");
	video.TextColor(White);
	video.go_to_xy(29, 6);
	video.write(" Game Configuration ");
	video.go_to_xy(0, bottomRow);
	video.writeln(" Copyright (c) 1991 Epic MegaGames                         Press ... to abort");
	video.TextColor(Black);
	video.TextBackground(LightGray);
	video.go_to_xy(65, bottomRow);
	video.write("ESC");
	video.TextColor(Yellow);
	video.TextBackground(Black);
	video.go_to_xy(0, 7);
	video.TextColor(LightGreen);
	video.TextBackground(Black);
	if (! video.Configure(keyboard)) {
		GameTitleExitRequested = true;
	}
}

void debug_video_input() {
	int col = 0;

	do {
		keyboard.wait_for_key();
		video.write(10, 10, col + 0x09,
			"Key read: " + itos(keyboard.InputKeyPressed) + " delta " + itos(
				keyboard.InputDeltaX) + "," + itos(keyboard.InputDeltaY));
		//video.write(10, 10, 0x0F, "Key read: " + itos(ReadKeyBlocking().key));
		++col;
		col = col % 7;
	} while (1 == 1);
}

void debug_display_output() {
	for (int bg = 0; bg < 16; ++bg) {
		for (int fg = 0; fg < 16; ++fg) {
			video.write(fg+10, bg+5, bg * 16 + fg, '!');
		}
	}
	keyboard.wait_for_key();
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

	init_IO(Blue);

	Randomize();

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
		order_print_id = GameVersion;
		IoTmpBuf = new byte[(MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1)+1];
		//new TIoTmpBuf;

		video.HideCursor();
		video.ClrScr();

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

	if (ConfigRegistration.size() == 0)  {
		GamePrintRegisterMessage();
	} else {
		video.writeln("");
		video.writeln("  Registered version -- Thank you for playing ZZT.");
		video.writeln("");
	}

	video.ShowCursor();
	return EXIT_SUCCESS;
}
