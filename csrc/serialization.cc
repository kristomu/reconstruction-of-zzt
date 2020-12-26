#include "serialization.h"
#include <stdexcept>

// Max length is the maximum length of the actual string, initial length
// byte notwithstanding. The remaining span will be padded with zeroes.
void append_pascal_string(const std::string & in_str, size_t max_length,
	std::vector<unsigned char> & append_to) {

	size_t actual_length = std::min(in_str.size(), max_length);
	if (actual_length >= 256) {
		throw std::logic_error("Can't output a Pascal string "
			"longer than 255 bytes!");
	}

	size_t i;
	// First dump the length byte.
	append_to.push_back((unsigned char)actual_length);
	// Then the data...
	for (i = 0; i < actual_length; ++i) {
		append_to.push_back(in_str[i]);
	}

	// ... then any required padding.
	for (; i < max_length; ++i) {
		append_to.push_back(0);
	}
}

void append_zeroes(size_t how_many,
	std::vector<unsigned char> & append_to) {
	for (size_t i = 0; i < how_many; ++i) {
		append_to.push_back(0);
	}
}