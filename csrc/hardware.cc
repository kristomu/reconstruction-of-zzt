#include <string>
#include <iostream>

#include "hardware.h"

curses_io * display;

// Set background color.
void TextBackground(dos_color bgColor) {
      display->set_background_color(bgColor);
}

// Set foreground color.
void TextColor(dos_color fgColor) {
      display->set_text_color(fgColor);
}

// Clear the screen
void ClrScr() { display->clrscr(); }

// Set the window size.
void Window(int left, int top, int right, int bottom) {}

// Go to (x, y).
void GotoXY(int x, int y) {
      display->move(x-1, y-1);
}

// Nop: https://www.freepascal.org/docs-html/current/rtl/dos/setcbreak.html
void SetCBreak(bool pollEveryTime) {}

void initCurses() {
      display = new curses_io();
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

bool Keypressed() {
      return display->key_pressed();
}

char ReadKey() {
      return display->read_key();
}

char ReadKeyBlocking() {
      return display->read_key_blocking();
}

char HasColors() {
      return display->supports_colors();
}

void SoundUninstall() {}
void SoundClearQueue() {}