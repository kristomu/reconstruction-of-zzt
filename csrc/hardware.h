#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>

#include "unicode.h"
#include "io/curses.h"
#include "video.h"

// Corners of the currently defined window or screen.
// (Move into function later.)

// These coordinates start at (1,1) (as is Pascal tradition).

const int WindMaxX = 80;      // Lower right, X coordinate
const int WindMaxY = 25;      // Lower right, Y coordinate
const int WindMinX = 1;       // Upper left, X coordinate
const int WindMinY = 1;       // Upper left, Y coordinate

extern Video video;

extern std::shared_ptr<io> display;
void initCurses();
bool Keypressed();
key_response ReadKey();
key_response ReadKeyBlocking();

// Currently unimplemented as we have no sound.
/*void SoundUninstall();
void SoundClearQueue();*/
void Sound(int hertz);
void NoSound();

// This is getting pretty ugly.
int64_t keyUpCase(int64_t key);

// Get the current time.
void GetTime(short & hour, short & minute, short & second,
	short & hundredths);

void Delay(int msec);

extern integer InputDeltaX,
	   InputDeltaY; // translates arrow keys to movement
extern bool InputShiftPressed;			// It does what it says
extern bool InputSpecialKeyPressed;
extern bool InputShiftAccepted; // ???
extern integer InputKeyPressed;

const bool InputJoystickMoved = false;	 // not supported
const bool InputJoystickEnabled = false; // ditto

void InputUpdate();				// Polls and updates.
void InputReadWaitKey();