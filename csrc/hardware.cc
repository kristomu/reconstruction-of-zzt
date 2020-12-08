#include <string>
#include <iostream>
#include <unistd.h>
#include <ctype.h>

#include "hardware.h"

std::shared_ptr<curses_io> display;
Video video;

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
      display = std::make_shared<curses_io>();
}

void cursesWrite(std::string x) {
      display->print(x);
}

void cursesWriteLn(std::string x) {
      cursesWrite(x);
      display->print("\n");
}


void uninitCurses() {
      // No conditional deleting required...
}

bool Keypressed() {
      return display->key_pressed();
}

key_response ReadKey() {
      return display->read_key();
}

key_response ReadKeyBlocking() {
      return display->read_key_blocking();
}

// Returns the key value for an ASCII key with no modifiers, otherwise 0.
char LiteralKey(const key_response response) {
      if (response.alt || response.ctrl) { return 0; }
      if (response.key < 0 || response.key > 127) {
            return 0;
      }

      return (char)response.key;
}

char HasColors() {
      return display->supports_colors();
}

void SoundUninstall() {}
void SoundClearQueue() {}

int64_t keyUpCase(int64_t key) {
      if (key < 0) {
            return key;
      }
      return toupper((char(key)));
}

void Delay(int msec) { usleep(msec*1000); }

int InputDeltaX, InputDeltaY;      // translates arrow keys to movement
bool InputShiftPressed;                  // It does what it says
bool InputAltPressed, InputCtrlPressed; // NEW, to handle our kb interface
bool InputSpecialKeyPressed;
bool InputShiftAccepted; // ???
bool InputJoystickMoved; // not supported
int InputKeyPressed;

// ZZT specs say that everything coming out of here should be CP437 points.
// Hence InputKeyPressed should be a char, and buffer also. Fix later?
void InputUpdateCore(bool blocking) {
      InputDeltaX = 0;
      InputDeltaY = 0;
      InputShiftPressed = false;
      InputJoystickMoved = false;
      InputKeyPressed = 0;

      // If there are no keys to fetch and we're nonblocking, just bail.
      if (!blocking && !Keypressed()) { return; }

      key_response key_read;

      if (blocking) {
            key_read = ReadKeyBlocking();
      } else {
            key_read = ReadKey();
      }

      // How do we handle special keys? Probably this (ugly) way:
      // If it's negative, then it's a special key, otherwise it's a
      // CP437 character.
      // All of this must be fixed later, once I've got it running.

      InputSpecialKeyPressed = key_read.key < 0;

      if (InputSpecialKeyPressed) {
            InputKeyPressed = key_read.key;
      } else {
            InputKeyPressed = CodepointToCP437(key_read.key);
      }

      // To avoid input lag, flush the whole buffer. The first key,
      // registered above, is what counts.
      while (Keypressed()) { ReadKey(); }

      InputShiftPressed = key_read.shift;
      InputAltPressed = key_read.alt;
      InputCtrlPressed = key_read.ctrl;

      // Set up the input deltas, but not if Ctrl or Alt were pressed.
      if (InputAltPressed || InputCtrlPressed) { return; }

      switch(InputKeyPressed) {
            case E_KEY_UP: case '8':
                  InputDeltaX = 0;
                  InputDeltaY = -1;
                  break;
            case E_KEY_LEFT: case '4':
                  InputDeltaX = -1;
                  InputDeltaY = 0;
                  break;
            case E_KEY_RIGHT: case '6':
                  InputDeltaX = 1;
                  InputDeltaY = 0;
                  break;
            case E_KEY_DOWN: case '2':
                  InputDeltaX = 0;
                  InputDeltaY = 1;
                  break;
      }
}

void InputUpdate() {
      InputUpdateCore(false);
}

void InputReadWaitKey() {
      InputUpdateCore(true);
}