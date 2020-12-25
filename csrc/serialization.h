#pragma once

#include <vector>
#include <string>
#include <fstream>

// These functions help serializing and deserializing without having to deal
// with unpacked vs packed structures, endian-ness or alignment violations.
// There are also some functions to convert between Pascal string notation
// and std::string, since the former is dictated by the ZZT format.

// Max length is the maximum length of the actual string, initial length
// byte notwithstanding. The remaining span will be padded with zeroes.
void append_pascal_string(const std::string & in_str, size_t max_length,
	std::vector<unsigned char> & append_to);

template<typename T> void append_lsb_element(T value,
        std::vector<unsigned char> & append_to);

void append_zeroes(size_t how_many,
	std::vector<unsigned char> & append_to);

// Templated functions must be present directly in the header.

// Gets a padded pascal string. These strings consist of a length byte
// followed by the contents of that string. Some Pascal strings are padded
// (stored as fixed length however many bytes) and some are not. If the string
// is padded, it's followed by undefined data to fill up to max_length.
// max_length is initial length byte notwithstanding.
// TODO? "Load" name instead?
template<typename T> T get_pascal_string(const T & ptr_start,
	const T & end, size_t max_length, bool padded,
	std::string & out_str, bool & out_truncated) {

	T ptr = ptr_start;

	// We must always have at least one (length indicator) byte.
	// Otherwise things are definitely truncated.
	if (ptr == end) {
		out_truncated = true;
		return ptr;
	}

	// The first byte counts the length of the string. If that is longer
	// than the max allowed length, then we have a truncated string by
	// definition.
	size_t len = *ptr++;
	size_t i, read_length = std::min(len, max_length);

	out_truncated = (len > max_length);
	out_str = "";

	for (i = 0; i < std::min(len, max_length); ++i) {
		// If we reach the end before we're done, it's truncated.
		if (ptr == end) {
			out_truncated = true;
			return ptr;
		}
		out_str += (char)*ptr;
		ptr++;
	}

	if (!padded) { return ptr; }

	// Increment the pointer to skip the rest of the padding, if possible.
	if (end - ptr < max_length - i) {
		out_truncated = true;
		return end;
	}
	ptr += max_length - i;
	return ptr;
}

template<typename T> void append_lsb_element(T value,
	std::vector<unsigned char> & append_to) {

	for (size_t i = 0; i < sizeof(T); ++i) {
		append_to.push_back(value & 255);
		value >>= 8;
	}
}

template<typename T> void append_array(const T & arr, size_t elements,
	std::vector<unsigned char> & append_to) {

	// for each element, append these in little-endian order.
	for (size_t i = 0; i < elements; ++i) {
		append_lsb_element(arr[i], append_to);
	}
}

template<typename T> void append_array(const T & arr,
	std::vector<unsigned char> & append_to) {

	append_array(arr, arr.size(), append_to);
}

// TODO support for end iterator (abort early). Or throw exception if
// ptr *is* end.

// Read an LSB element from something of char or unsigned char (e.g. char*,
// vector<unsigned char>).
template<typename T, typename Q> T load_lsb_element(T iterator,
	Q & output) {

	output = 0;
	unsigned char current_byte;
	for (size_t i = 0; i < sizeof(Q); ++i) {
		current_byte = *iterator++;
		output += Q(current_byte) << (8 * i);
	}

	return iterator;
}

template<typename T, typename Q> T load_array(T iterator, size_t elements,
	Q & out_array) {

	// for each element, append these in little-endian order.
	for (size_t i = 0; i < elements; ++i) {
		iterator = load_lsb_element(iterator, out_array[i]);
	}

	return iterator;
}

template<typename T, typename Q> T load_array(T iterator, Q & out_array) {

	return load_array(iterator, out_array.size(), out_array);
}

template<typename Q> void load_lsb_from_file(std::istream & input,
	Q & output) {

	char temp_buffer[sizeof(output)];
	input.read(temp_buffer, sizeof(output));
	load_lsb_element(temp_buffer, output);
}