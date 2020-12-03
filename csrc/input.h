#ifndef __input_h__
#define __input_h__

const char KEY_BACKSPACE = '\10';
const char KEY_TAB = '\11';
const char KEY_ENTER = '\15';
const char KEY_CTRL_Y = '\31';
const char KEY_ESCAPE = '\33';
const char KEY_ALT_P = '\231';
const char KEY_F1 = '\273';
const char KEY_F2 = '\274';
const char KEY_F3 = '\275';
const char KEY_F4 = '\276';
const char KEY_F5 = '\277';
const char KEY_F6 = '\300';
const char KEY_F7 = '\301';
const char KEY_F8 = '\302';
const char KEY_F9 = '\303';
const char KEY_F10 = '\304';
const char KEY_UP = '\310';
const char KEY_PAGE_UP = '\311';
const char KEY_LEFT = '\313';
const char KEY_RIGHT = '\315';
const char KEY_DOWN = '\320';
const char KEY_PAGE_DOWN = '\321';
const char KEY_INSERT = '\322';
const char KEY_DELETE = '\323';
const char KEY_HOME = '\307';
const char KEY_END = '\317';
const char KEY_SHIFT_UP = '\301';
const char KEY_SHIFT_DOWN = '\302';
const char KEY_SHIFT_LEFT = '\304';
const char KEY_SHIFT_RIGHT = '\303';


#ifdef __Input_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN integer InputDeltaX, InputDeltaY;
EXTERN boolean InputShiftPressed;
EXTERN boolean InputShiftAccepted;
EXTERN boolean InputJoystickEnabled;
EXTERN boolean InputMouseEnabled;
EXTERN boolean InputSpecialKeyPressed;
EXTERN char InputKeyPressed;
EXTERN integer InputMouseX, InputMouseY;
EXTERN integer InputMouseActivationX, InputMouseActivationY;
EXTERN integer InputMouseButtonX, InputMouseButtonY;
EXTERN boolean InputJoystickMoved;
#undef EXTERN
#define EXTERN extern

        void InputUpdate();
        void InputInitDevices();
        void InputReadWaitKey();
        boolean InputConfigure();

#endif
