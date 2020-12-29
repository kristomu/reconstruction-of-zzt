#include "input.h"

// ZZT specs say that everything coming out of here should be CP437 points.
// Hence InputKeyPressed should be a char, and buffer also. Fix later?
void Input::update_key_variables(bool blocking) {
	InputDeltaX = 0;
	InputDeltaY = 0;
	InputShiftPressed = false;
	InputKeyPressed = 0;

	// If there are no keys to fetch and we're nonblocking, just bail.
	if (!blocking && !io_interface->key_pressed()) {
		return;
	}

	key_response key_read;

	if (blocking) {
		key_read = io_interface->read_key_blocking();
	} else {
		key_read = io_interface->read_key();
	}

	// How do we handle special keys? Probably this (ugly) way:
	// If it's negative, then it's a special key, otherwise it's a
	// CP437 character. Also hard-code some control characters that
	// don't have a negative key value.

	InputSpecialKeyPressed = key_read < 0 ||
		key_read == E_KEY_ESCAPE ||
		key_read == E_KEY_ENTER ||
		key_read == E_KEY_TAB;

	if (InputSpecialKeyPressed) {
		InputKeyPressed = key_read;
	} else {
		InputKeyPressed = converter.codepoint_to_CP437(key_read);
	}

	InputShiftPressed = InputKeyPressed == E_KEY_SHIFT_LEFT ||
		InputKeyPressed == E_KEY_SHIFT_RIGHT ||
		InputKeyPressed == E_KEY_SHIFT_UP ||
		InputKeyPressed == E_KEY_SHIFT_DOWN;

	// Set up the input deltas.

	switch (InputKeyPressed) {
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

void Input::update() {
	update_key_variables(false);
}

void Input::wait_for_key() {
	update_key_variables(true);
}