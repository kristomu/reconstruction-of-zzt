#pragma once
#include <string>
#include <iostream>
#include <memory>
#include <vector>

#include "unicode.h"
#include "curses_io.h"
#include "video.h"

// All coordinates start at (1,1) (as is Pascal tradition).

// Sets attributes like blink, etc. (Move into function later.)
const int TextAttr = 0x07;

// Corners of the currently defined window or screen.
// (Move into function later.)

const int WindMaxX = 80;      // Lower right, X coordinate
const int WindMaxY = 25;      // Lower right, Y coordinate
const int WindMinX = 1;       // Upper left, X coordinate
const int WindMinY = 1;       // Upper left, Y coordinate

extern std::shared_ptr<curses_io> display;
extern Video video;

// Set background color.
void TextBackground(dos_color bgColor);
// Set foreground color.
void TextColor(dos_color fgColor);

// Clear the screen
void ClrScr();
// Redraw
void redraw();

// Set the window size. Nop for now.
void Window(int left, int top, int right, int bottom);

// Go to (x, y).
void GotoXY(int x, int y);

// Nop: https://www.freepascal.org/docs-html/current/rtl/dos/setcbreak.html
void SetCBreak(bool pollEveryTime);

void initCurses();
void cursesWrite(std::string x);
void cursesWriteLn(std::string x);
void uninitCurses();
bool Keypressed();
key_response ReadKey();
key_response ReadKeyBlocking();
char LiteralKey(const key_response response);
char HasColors();

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
	InputDeltaY;	// translates arrow keys to movement
extern bool InputShiftPressed;			// It does what it says
extern bool InputSpecialKeyPressed;
extern bool InputShiftAccepted; // ???
extern bool InputJoystickMoved; // not supported
extern integer InputKeyPressed;

void InputUpdate();				// Polls and updates.
void InputReadWaitKey();