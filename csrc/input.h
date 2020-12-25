#ifndef __input_h__
#define __input_h__

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
