#pragma once
#include <string>
#include <iostream>

#include "curses.h"

// Everything works in terms of DOS colors. The ZZT conversion doesn't know
// about any others. Still, we need to port over some Pascal constants.

// All coordinates start at (1,1) (as is Pascal tradition).

enum Color { Black = 0,
             Blue = 1,
             Green = 2,
             Cyan = 3,
             Red = 4,
             Magenta = 5,
             Brown = 6,
             LightGray = 7,
             DarkGray = 8,
             LightBlue = 9,
             LightGreen = 10,
             LightCyan = 11,
             LightRed = 12,
             LightMagenta = 13,
             Yellow = 14,
             White = 15 };

// Sets attributes like blink, etc. (Move into function later.)
int TextAttr = 0x07;

// Corners of the currently defined window or screen.
// (Move into function later.)

int WindMaxX = 80;      // Lower right, X coordinate
int WindMaxY = 25;      // Lower right, Y coordinate
int WindMinX = 1;      // Upper left, X coordinate
int WindMinY = 1;      // Upper left, Y coordinate

curses * display;

// Set background color.
void TextBackground(Color bgColor) {
      display->set_dos_background_color(bgColor);
}

// Set foreground color.
void TextColor(Color fgColor) {
      display->set_dos_text_color(fgColor);
}

// Clear the screen
void ClrScr() {}

// Set the window size.
void Window(int left, int top, int right, int bottom) {}

// Go to (x, y).
void GotoXY(int x, int y) {
      display->move(x-1, y-1);
}

// Nop: https://www.freepascal.org/docs-html/current/rtl/dos/setcbreak.html
void SetCBreak(bool pollEveryTime) {}

void initCurses() {
      display = new curses();
}

void cursesWrite(std::string x) {
      display->print(x);
}

void cursesWriteLn(std::string x) {
      cursesWrite(x);
      display->print("\n");
}


void uninitCurses() {
      if(display != NULL) {
            delete display;
      }
}

/*bool Keypressed() {

}*/