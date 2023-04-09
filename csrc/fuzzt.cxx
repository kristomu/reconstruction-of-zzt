/*
	Copyright (c) 2020 Adrian Siekierka
	Copyright (c) 2020 Kristofer Munsterhjelm

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

#include <inttypes.h>

#include "ptoc.h"
#include "tools.h"

#include "io/stub.h"
#include "hardware.h"
#include "gamevars.h"
#include "game.h"
#include "sounds.h"
#include "fileops.h"
#include "video.h"
#include "world.h"

#include "testing.h"

#include <unistd.h>
#include <iostream>
#include <sstream>

std::array<TElementProcDef, MAX_ELEMENT+1> ElementProcDefs;
std::shared_ptr<ElementInfo> elem_info_ptr;
std::shared_ptr<TWorld> game_world;

void ParseArguments(int argc, const char ** argv) {
	integer i;
	std::string pArg;

	// Don't accept any switches, just consider the last
	// argument as a literal filename.

	for (i = 1; i < argc; i ++) {
		StartupWorldFileName = argv[i];
		if ((length(StartupWorldFileName) > 4)
			&& (StartupWorldFileName[length(StartupWorldFileName) - 3] == '.'))  {
			StartupWorldFileName = copy(StartupWorldFileName, 1,
					length(StartupWorldFileName) - 4);
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

std::shared_ptr<stub_io> stub_ptr;

void init_IO_fuzz(dos_color border_color) {
	stub_ptr = std::make_shared<stub_io>();
#ifdef FUZZ_DISABLE_CURSES
	video.install(border_color, stub_ptr);
	keyboard.set_interface(stub_ptr);
#else
	std::shared_ptr<curses_io> curses_ptr = NULL;

	if (!test_mode_disable_video || !test_mode_disable_input) {
		curses_ptr = std::make_shared<curses_io>();
	}

	if (test_mode_disable_video) {
		video.install(border_color, stub_ptr);
	} else {
		video.install(border_color, curses_ptr);
	}

	if (test_mode_disable_input) {
		keyboard.set_interface(stub_ptr);
	} else {
		keyboard.set_interface(curses_ptr);
	}
#endif
}

// Opens a world and goes through each of its boards so as to patch
// obvious errors with its format, then saves the world under a
// different name. This makes it easier to analyze complex bugs without
// e.g. KevEdit refusing to open the world due to simpler errors.
void refine(std::string input_filename,
	std::string refined_world_filename) {
	WorldLoad(input_filename, ".ZZT");
	for (int i = 0; i < game_world->BoardCount; ++i) {
		BoardChange(i);
	}
	BoardChange(0);
	WorldSave(refined_world_filename, ".ZZT");

}

// TODO: Prepend "LLVM" to the next two function names to enable persistent mode
// in honggfuzz. Only when I've got idempotency.

extern "C" int FuzzerInitialize(int argc, const char* * argv) {
	test_mode_disable_video = true;
	test_mode_disable_input = true;
	test_mode_disable_delay = true;
	test_mode_disable_dialog_boxes = true;
	test_mode_disable_text_input = true;

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

	init_IO_fuzz(Blue);
	stub_ptr->set_key_responses({'C'}); // skip init screen
	GameConfigure();
	StartupWorldFileName = "TOWN";
	ResourceDataFileName = "ZZT.DAT";
	ResetConfig = false;
	ParseArguments(argc, argv);

	return 0;
}

extern "C" int FuzzerTestOneInput(const uint8_t * data, size_t size) {

	rnd.seed(1);
	GenerateTransitionTable();
	TextWindowInit(5, 3, 50, 18);

	IoTmpBuf = new byte[(MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1)+1];
	preloaded_world_data.resize(size);
	std::copy(data, data+size, preloaded_world_data.begin());

	stub_ptr->set_key_responses({
		// run a few cycles doing nothing
		E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER,
		// play and go right a bunch
		'P', E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT,
		// more running cycles doing nothing
		E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER,
		// abort pause (??)
		E_KEY_RIGHT,
		// quit from play, and quit the game. There's some kind of
		// desynchronization bug that makes the player sometimes go
		// one step too far, so do enough ESC-Y combinations to be
		// sure. In addition, use a Q (for quit) which helps fix problems
		// where the player is continually being #ENDGAMEd. Q does nothing
		// if we're already in a quit prompt, but escapes to a quit prompt
		// if the ESCAPE got us out of a game over situation. I think that's
		// what's happening in ENDGAME.ZZT.
		E_KEY_ESCAPE, 'Q', 'Y', E_KEY_ESCAPE, 'Q', 'Y', E_KEY_ESCAPE, 'Q', 'Y',
	});

	GameTitleExitRequested = false;
	//TextWindowInit(5, 3, 50, 18);
	InitWorld();

	// When running manually, it may be useful to do a refinement, like this:
	// refine(StartupWorldFileName.str(), "REFINED");

	if (!GameTitleExitRequested)  {
		TTextWindowState textWindow;

		video.HideCursor();
		video.ClrScr();

		TickSpeed = 4;
		DebugEnabled = false;
		SavedGameFileName = "SAVED";
		SavedBoardFileName = "TEMP";

		WorldCreate();
		GameTitleLoop();

		WorldUnload();
	}

	SoundUninstall();
	SoundClearQueue();
	delete[] IoTmpBuf;
	return 0;
}

// Set up different fuzz approaches.

// I just choose one in main: currently forkserver works, but a single run through
// LLVMFuzzerTestOneInput disturbs too many global variables, which hurts the
// repeatability of persistent mode. Since honggfuzz doesn't have a fork server mode,
// I need to get persistent mode to some near-idempotent state before I can use that
// fuzzer...

int AFL_forkserver_fuzz(int argc, const char* argv[]) {

	FuzzerInitialize(argc, argv);

#ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_INIT();
#endif

	// fuzz shim hack, fix later
	std::ifstream infile(argv[1], std::ifstream::binary | std::ifstream::ate);
	size_t length = infile.tellg();
	infile.seekg(0, infile.beg);
	std::vector<char> file_dump(length);
	infile.read(file_dump.data(), length);
	infile.close();

	FuzzerTestOneInput((const uint8_t *)file_dump.data(), length);
	return 0;
}

// Mainly use this for testing idempotency, because afl will report if stability
// is too low.

int AFL_persistent_fuzz(int argc, const char* argv[]) {

	FuzzerInitialize(argc, argv);

#ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_FUZZ_INIT();
	__AFL_INIT();

	unsigned char * buf = __AFL_FUZZ_TESTCASE_BUF;

	while (__AFL_LOOP(10000)) {
		int length = __AFL_FUZZ_TESTCASE_LEN;

		FuzzerTestOneInput(buf, length);
	}
#endif

	return 0;
}

// Native file loading to a vector to bypass the file loading code in game.cxx.
// It's quite a hack, but presumably I can make it more elegant later :-)
std::vector<char> load_file(std::string filename) {
	std::ifstream infile(filename, std::ifstream::binary | std::ifstream::ate);
	size_t length = infile.tellg();
	infile.seekg(0, infile.beg);
	std::vector<char> file_dump(length);
	infile.read(file_dump.data(), length);
	infile.close();
	return file_dump;
}

// Test idempotency (in a rough way). Currently this returns quickly,
// because the slow world isn't really evaluated. It should be returning
// slow, as it would if the slow_world is directly invoked.
void perf_idempotence_test(std::string fast_world,
	std::string slow_world) {
	char fake_process_name[] = "";
	const char * fake_argv[1] = {fake_process_name};

	FuzzerInitialize(0, fake_argv);

	std::vector<char> file_dump = load_file(fast_world);
	FuzzerTestOneInput((const uint8_t *)file_dump.data(), file_dump.size());

	file_dump = load_file(slow_world);
	FuzzerTestOneInput((const uint8_t *)file_dump.data(), file_dump.size());
}

// Just do
int main(int argc, const char* argv[]) {
	return AFL_forkserver_fuzz(argc, argv);
}