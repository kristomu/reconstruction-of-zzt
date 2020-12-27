#pragma once

#include <string>

typedef int64_t key_response;

// Codes for special keys (non-literals).

const key_response	E_KEY_UP = -1,
					E_KEY_DOWN = -2,
					E_KEY_RIGHT = -3,
					E_KEY_LEFT = -4,
					E_KEY_NUMPAD_CLEAR = -5,	// the one in the center of the numpad
					E_KEY_INSERT = -6,
					E_KEY_HOME = -7,
					E_KEY_PAGE_UP = -8,
					E_KEY_PAGE_DOWN = -9,
					E_KEY_DELETE = -10,
					E_KEY_END = -11,
					E_KEY_F1 = -12,
					E_KEY_F2 = -13,
					E_KEY_F3 = -14,
					E_KEY_F4 = -15,
					E_KEY_F5 = -16,
					E_KEY_F6 = -17,
					E_KEY_F7 = -18,
					E_KEY_F8 = -19,
					E_KEY_F9 = -20,
					E_KEY_F10 = -21,
					E_KEY_F11 = -22,
					E_KEY_F12 = -23,
					E_KEY_PAUSE = -24,
					E_KEY_UNKNOWN = -25,
					E_KEY_BACKSPACE = -26,
					E_KEY_SHIFT_UP = -27,
					E_KEY_SHIFT_DOWN = -28,
					E_KEY_SHIFT_RIGHT = -29,
					E_KEY_SHIFT_LEFT = -30,
					E_KEY_CTRL_Y = -31,
					E_KEY_ALT_P = -32,
					//E_KEY_NONE = -33,			// private

					E_KEY_ESCAPE = '\33',
					E_KEY_ENTER = '\n',
					E_KEY_TAB = '\t';