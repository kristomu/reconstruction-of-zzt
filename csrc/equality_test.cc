// I can't say I entirely like the code here, but I can't put my
// finger on just what I don't like, either. Check later?

#include "paszzt_test.h"
#include "hardware.h"
#include "testing.h"
#include "world.h"
#include "game.h"

#include "io/curses.h"
#include "io/stub.h"
#include "video.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <string>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// https://stackoverflow.com/a/2436368

void segfault_sigaction(int signal, siginfo_t *si, void *arg) {
	exit(0);
}

void disable_signals() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags   = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGBUS, &sa, NULL);
}

void enable_signals() {
	signal(SIGSEGV, SIG_DFL);
	signal(SIGSEGV, SIG_DFL);
	signal(SIGABRT, SIG_DFL);
	signal(SIGBUS, SIG_DFL);
}

// C++ ZZT

std::shared_ptr<stub_io> stub_ptr;
TWorld World;

void init_IO_fuzz(dos_color border_color) {
	std::shared_ptr<curses_io> curses_ptr = NULL;
	stub_ptr = std::make_shared<stub_io>();

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
}

std::vector<char> evolve_zzt_cpp(const std::vector<char> & world) {
	test_mode_disable_video = true;
	test_mode_disable_input = true;
	test_mode_disable_delay = true;
	test_mode_disable_dialog_boxes = true;
	test_mode_disable_text_input = true;

	init_IO_fuzz(Blue);
	StartupWorldFileName = "";
	ResourceDataFileName = "ZZT.DAT";
	ResetConfig = false;

	IoTmpBuf = new byte[(MAX_BOARD_LEN + MAX_RLE_OVERFLOW-1)+1];
	rnd.seed(1);
	GenerateTransitionTable();

	stub_ptr->set_key_responses({
		// run a few cycles doing nothing (20x)
		E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER,
		E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER,
		E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER,
		E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER, E_KEY_ENTER,
		// play and go right a bunch (20x)
		'P',
		E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT,
		E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT,
		E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT,
		E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT, E_KEY_RIGHT,
		// quit from play, and quit the game.
		E_KEY_ESCAPE, 'Y', E_KEY_ESCAPE, 'Y', E_KEY_ESCAPE, 'Y'});

	TextWindowInit(5, 3, 50, 18);

	TTextWindowState textWindow;

	video.HideCursor();
	video.ClrScr();

	TickSpeed = 4;
	DebugEnabled = false;
	SavedGameFileName = "SAVED";
	SavedBoardFileName = "TEMP";
	JustStarted = false;

	if (!WorldLoad(world, "NONE")) {
		WorldCreate();
	}

	ReturnBoardId = World.Info.CurrentBoard;
	BoardChange(0);
	CurrentStatTicked = 0;
	CurrentTick = 0;

	bool boardChanged = true;

	GameStateElement = E_MONITOR;
	GameTitleLoop();

	std::vector<char> after_play = WorldSaveVector();
	WorldUnload();

	delete[] IoTmpBuf;

	return after_play;
}

// -Comparison of the two implementations.-

// Returns true upon a recoverable crash (e.g. out of bounds caught by
// the compiler), false on no crash, and hard-crashes if there is one.
bool check_worlds(std::string world_filename) {

	std::ifstream world_file(world_filename, std::ios::in | std::ios::binary);
	std::istreambuf_iterator<char> start(world_file), end;
	std::vector<char> world(start, end);
	std::ofstream foo("OUT.DAT");
	foo.write(world.data(), world.size());
	foo.close();

	// Always provision enough space to write a standard yellow border world.
	size_t max_len = std::max(world.size()*2, (size_t)1000);

	std::vector<char> output(max_len);

	disable_signals();

	int bytes_written = EvolveZZT(world.data(), world.size(), output.data(),
			output.size());

	if (output.size() > bytes_written) {
		output.resize(bytes_written);
	}

	std::ofstream bar("OUT2.DAT");
	bar.write(output.data(), bytes_written);
	bar.close();

	std::vector<char> cpp_output = evolve_zzt_cpp(world);
	std::ofstream baz("OUT3.DAT");
	baz.write(cpp_output.data(), cpp_output.size());
	baz.close();

	enable_signals();

	if (cpp_output != output) {
		// The two worlds differ; crash.
		int * x = 0;
		x[3] = 0;
	}

	return FailFlag;
}

// The metric is:
//	If it crashes either native or FPC ZZT, then the program returns
//		without a crash (that's not the kind of discrepancy this
//		program is intended to find).
//	If it works successfully on both but the final board stat differs,
//		then that's a crash (not implemented yet).
//	Otherwise, the program returns successfully.

int main(int argc, char** argv) {
	if (argc < 2) {
		std::cout << "Usage: " << argv[0] << " world_to_test.zzt\r\n";
		std::cout << "Will terminate normally if either the Pascal or C++ ";
		std::cout << "implementation crashes,\r\n";
		std::cout << "or if they both succeed and produce the same output. ";
		std::cout << "Will crash if they both\r\n";
		std::cout << "succeed but produce differing output.\r" << std::endl;
		return 0;
	}

	std::string world = argv[1];

	if (check_worlds(world)) {
		std::cout << world << " crashed" << std::endl;
	} else {
		std::cout << world << " finished successfully." << std::endl;
	}
	return 0;
}