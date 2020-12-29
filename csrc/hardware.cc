#include <string>
#include <iostream>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include "hardware.h"
#include "io/stub.h"
#include "testing.h"
#include "input.h"

Input keyboard;
Video video;

void init_IO(dos_color border_color) {
	std::shared_ptr<stub_io> stub_ptr;
	std::shared_ptr<curses_io> curses_ptr;

	if (test_mode_disable_video || test_mode_disable_input) {
		stub_ptr = std::make_shared<stub_io>();
		//display =
	}

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

/*void SoundUninstall() {}
void SoundClearQueue() {}*/
void Sound(int hertz) {}
void NoSound() {}

int64_t keyUpCase(int64_t key) {
	if (key < 0) {
		return key;
	}
	return toupper((char)key);
}

void GetTime(short & hour, short & minute, short & second,
	short & hundredths) {

	// https://stackoverflow.com/questions/11242642

	time_t t = time(NULL);
	struct tm *tmp = gmtime(&t);

	hour = tmp->tm_hour;
	minute = tmp->tm_min;
	second = tmp->tm_sec;

	struct timeval tv;
	gettimeofday(&tv,NULL);
	hundredths = tv.tv_usec / 10000;
}

void Delay(int msec) {
	if (!test_mode_disable_delay) {
		usleep(msec*1000);
	}
}