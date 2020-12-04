#pragma once
#include <string>
#include <iostream>

#include "curses.h"

// All coordinates start at (1,1) (as is Pascal tradition).

// Sets attributes like blink, etc. (Move into function later.)
const int TextAttr = 0x07;

// Corners of the currently defined window or screen.
// (Move into function later.)

const int WindMaxX = 80;      // Lower right, X coordinate
const int WindMaxY = 25;      // Lower right, Y coordinate
const int WindMinX = 1;       // Upper left, X coordinate
const int WindMinY = 1;       // Upper left, Y coordinate

extern curses * display;

// Set background color.
void TextBackground(dos_color bgColor);
// Set foreground color.
void TextColor(dos_color fgColor);

// Clear the screen (nop)
void ClrScr();

// Set the window size.
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
char ReadKey();