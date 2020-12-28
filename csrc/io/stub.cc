#include "stub.h"
#include <stdexcept>

// If we're running "blind" (i.e. testing), we should be able to specify
// a sequence of keypresses that translate into e.g. player movement in
// the world in question. That's what this is for. Once every keypress is
// exhausted, the stub triggers an exception.

// Any refactoring to do demos, speedruns or the likes will have to wait
// for later.

bool stub_io::key_pressed() {
	return true;
}
key_response stub_io::read_key() {
	if (response_pos == key_responses.end()) {
		throw std::runtime_error("IO stub: no more keys to press!");
	}
	return *response_pos++;
}
key_response stub_io::read_key_blocking() {
	return read_key();
}

void stub_io::flush_keybuf() {
}

void stub_io::set_key_responses(std::vector<key_response> responses_in) {
	key_responses = responses_in;
	response_pos = key_responses.begin();
}