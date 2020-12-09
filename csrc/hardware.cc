#include <string>
#include <iostream>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

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
      if (response < 0 || response > 127) {
            return 0;
      }

      return (char)response;
}

char HasColors() {
      return display->supports_colors();
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

void Delay(int msec) { usleep(msec*1000); }

integer InputDeltaX, InputDeltaY;      // translates arrow keys to movement
bool InputShiftPressed;                  // It does what it says
bool InputSpecialKeyPressed;
bool InputShiftAccepted; // ???
bool InputJoystickMoved; // not supported
integer InputKeyPressed;

// ZZT specs say that everything coming out of here should be CP437 points.
// Hence InputKeyPressed should be a char, and buffer also. Fix later?
void InputUpdateCore(bool blocking) {
      InputDeltaX = 0;
      InputDeltaY = 0;
      InputShiftPressed = false;
      InputJoystickMoved = false;

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
      // CP437 character. Also hard-code some control characters that
      // don't have a negative key value.
      // All of this must be fixed later, once I've got it running.

      InputSpecialKeyPressed = key_read < 0 ||
            key_read == E_KEY_ESCAPE ||
            key_read == E_KEY_ENTER ||
            key_read == E_KEY_TAB;

      if (InputSpecialKeyPressed) {
            InputKeyPressed = key_read;
      } else {
            InputKeyPressed = CodepointToCP437(key_read);
      }

      // To avoid input lag, flush the whole buffer. The first key,
      // registered above, is what counts.
      display->flush_keybuf();

      InputShiftPressed = InputKeyPressed == E_KEY_SHIFT_LEFT ||
            InputKeyPressed == E_KEY_SHIFT_RIGHT ||
            InputKeyPressed == E_KEY_SHIFT_UP ||
            InputKeyPressed == E_KEY_SHIFT_DOWN;

      // Set up the input deltas.

      switch(InputKeyPressed) {
            case E_KEY_SHIFT_UP: case E_KEY_UP: case '8':
                  InputDeltaX = 0;
                  InputDeltaY = -1;
                  break;
            case E_KEY_SHIFT_LEFT: case E_KEY_LEFT: case '4':
                  InputDeltaX = -1;
                  InputDeltaY = 0;
                  break;
            case E_KEY_SHIFT_RIGHT: case E_KEY_RIGHT: case '6':
                  InputDeltaX = 1;
                  InputDeltaY = 0;
                  break;
            case E_KEY_SHIFT_DOWN: case E_KEY_DOWN: case '2':
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