#include "ptoc.h"

/*
	Copyright (c) 2020 Adrian Siekierka

	Based on a reconstruction of code from ZZT,
	Copyright 1991 Epic MegaGames, used with permission.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

#define __Input_implementation__


#include "input.h"

#include "dos.h"
#include "sounds.h"

/*uses Dos, Sounds, Termio, Unicode;*/

const integer PORT_JOYSTICK = 0x201;
integer JoystickXInitial, JoystickYInitial;
integer InputLastDeltaX, InputLastDeltaY;
integer JoystickXMin, JoystickXCenter, JoystickXMax;
integer JoystickYMin, JoystickYCenter, JoystickYMax;
string InputKeyBuffer;

/* Everything joystick is right out in non-DOS. Ditto mouse. */

/* UTF-8 input part from https://bit.ly/39nWLu5 p. 22 */
void InputUpdate() {
    array<0,3,byte> utf;	        /* UTF8 storage array */
    byte i;					        /* Counter for UTF8 recording */

    InputDeltaX = 0;
    InputDeltaY = 0;
    InputShiftPressed = false;
    InputJoystickMoved = false;
    /* The raw text mode that Crt places a Linux terminal into seems
    to make it possible to fill the keypress queue faster than
    	  ReadKey can dispose of it. So we can only read once and then
    	  need to flush the whole queue afterwards. This will lead to
    	  nonstandard behavior if objects hog all the processing power.*/
    while (keypressed())  {
        InputKeyPressed = readkey();
        utf[0] = (byte)(InputKeyPressed);
        /* Special keys */
        if ((InputKeyPressed == '\0') || (InputKeyPressed == '\1')
                || (InputKeyPressed == '\2'))  {
            InputKeyBuffer = InputKeyBuffer + chr(ord(readkey()) | 0x80);
            InputSpecialKeyPressed = true;
            /* Key corresponding to multi-byte UTF8 */
        } else if (utf[0] > 0x7f)  {
            i = 1;
            while ((i <= 3) & ByteBool((utf[0] << i) & 128))  {
                utf[i] = (byte)(readkey());
                i += 1;
            }

            InputKeyBuffer = InputKeyBuffer + CodepointToCP437(UTF8ToCodepoint(utf));
            InputSpecialKeyPressed = false;
        } else {       /* Ordinary key, or... */
            InputSpecialKeyPressed = false;
            /* Ugly hack for Linux console and the end key. For
            some reason it outputs F[F[. */
            if ((InputKeyPressed == '\106') & keypressed())  {
                InputKeyPressed = readkey();
                if (InputKeyPressed == '[')  {
                    /* Get rid of the second F[. */
                    if (keypressed())  readkey();
                    if (keypressed())  readkey();

                    InputKeyPressed = KEY_END;
                    InputSpecialKeyPressed = true;
                } else InputKeyPressed = '\106';
            }
            InputKeyBuffer = InputKeyBuffer + InputKeyPressed;
        }
        TCFlush(0, TCIFLUSH); /* Flush the keybuffer. */
    }
    if (length(InputKeyBuffer) != 0)  {
        InputKeyPressed = InputKeyBuffer[1];
        if (length(InputKeyBuffer) == 1)
            InputKeyBuffer = "";
        else
            InputKeyBuffer = copy(InputKeyBuffer, length(InputKeyBuffer) - 1, 1);
        /* If the player pressed SHIFT+arrow, set
        InputShiftPressed.*/

        InputShiftPressed = false;

        switch (InputKeyPressed) {
        case KEY_SHIFT_UP: case KEY_SHIFT_DOWN:
        case KEY_SHIFT_LEFT: case KEY_SHIFT_RIGHT: {
            InputShiftPressed = true;
        }
        break;
        }

        switch (InputKeyPressed) {
        case KEY_UP: case KEY_SHIFT_UP: case '8': {
            InputDeltaX = 0;
            InputDeltaY = -1;
        }
        break;
        case KEY_LEFT: case KEY_SHIFT_LEFT: case '4': {
            InputDeltaX = -1;
            InputDeltaY = 0;
        }
        break;
        case KEY_RIGHT: case KEY_SHIFT_RIGHT: case '6': {
            InputDeltaX = 1;
            InputDeltaY = 0;
        }
        break;
        case KEY_DOWN: case KEY_SHIFT_DOWN: case '2': {
            InputDeltaX = 0;
            InputDeltaY = 1;
        }
        break;
        }
    } else {
        InputKeyPressed = '\0';
    }

    if ((InputDeltaX != 0) || (InputDeltaY != 0))  {
        InputLastDeltaX = InputDeltaX;
        InputLastDeltaY = InputDeltaY;
    }
}

void InputInitDevices() {
    InputJoystickEnabled = false;
    InputMouseEnabled = false;
}

boolean InputConfigure() {
    char charTyped;

    boolean InputConfigure_result;
    charTyped = ' ';
    if (InputJoystickEnabled || InputMouseEnabled)  {
        output << NL;
        output << "  Game controller:  K)eyboard";
        if (InputJoystickEnabled)
            output << ",  J)oystick";
        if (InputMouseEnabled)
            output << ",  M)ouse";
        output << "?  ";

        do {
            do {; } while (!keypressed());
            charTyped = upcase(readkey());
        } while (!((charTyped == 'K')
                   || (charTyped == '\33')));
        output << NL;

        InputJoystickEnabled = false;
        InputMouseEnabled = false;
        output << NL;
    }
    InputConfigure_result = charTyped != '\33';
    return InputConfigure_result;
}

void InputReadWaitKey() {
    do {
        InputUpdate();
        /* Don't busy-wait too much. */
        Delay(10);
    } while (!(InputKeyPressed != '\0'));
}

class unit_Input_initialize {
public: unit_Input_initialize();
};
static unit_Input_initialize Input_constructor;

unit_Input_initialize::unit_Input_initialize() {
    InputLastDeltaX = 0;
    InputLastDeltaY = 0;
    InputDeltaX = 0;
    InputDeltaY = 0;
    InputShiftPressed = false;
    InputShiftAccepted = false;
    InputSpecialKeyPressed = false;
    InputMouseX = 0;
    InputMouseY = 0;
    InputMouseActivationX = 60;
    InputMouseActivationY = 60;
    InputMouseButtonX = 0;
    InputMouseButtonY = 0;
    InputKeyBuffer = "";
    SetupCodepointToCP437;
}
