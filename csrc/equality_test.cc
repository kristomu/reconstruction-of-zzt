#include "paszzt_test.h"

#include <iostream>
#include <iterator>
#include <fstream>
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

void disable_segv() {
	struct sigaction sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags   = SA_SIGINFO;

	sigaction(SIGSEGV, &sa, NULL);
}

// Returns true upon a recoverable crash (e.g. out of bounds caught by
// the compiler), false on no crash, and hard-crashes if there is one.
bool does_crash(std::string world_filename) {

	std::ifstream world_file(world_filename, std::ios::in | std::ios::binary);
	std::istreambuf_iterator<char> start(world_file), end;
	std::vector<char> world(start, end);
	std::ofstream foo("OUT.DAT");
	foo.write(world.data(), world.size());
	foo.close();

	EvolveZZT(world.data(), world.size());
	return FailFlag;
}

// The metric is:
//	If it crashes either native or FPC ZZT, then the program returns
//		without a crash (that's not the kind of discrepancy this
//		program is intended to find).
//	If it works successfully on both but the final board stat differs,
//		then that's a crash (not implemented yet).
//	Otherwise, the program returns successfully.

int main() {
	std::vector<std::string> worlds = {"TEST.ZZT", "TOWN.ZZT", "TEST.ZZT"};
	//disable_segv();

	for (std::string world: worlds) {
		std::cout << "Testing " << world << std::endl;
		if (does_crash(world)) {
			std::cout << world << " crashed" << std::endl;
		} else {
			std::cout << world << " finished successfully." << std::endl;
		}
	}
	return 0;
}
