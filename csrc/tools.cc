#include "tools.h"
#include <sstream>
#include <algorithm>

// Integer to string

std::string itos(int source) {
	std::ostringstream q;
	q << source;
	return (q.str());
}

// Uppercase a string. From
// https://en.cppreference.com/w/cpp/string/byte/toupper

std::string str_toupper(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(),
	[](unsigned char c) {
		return std::toupper(c);    // correct
	}
	);
	return s;
}

void update(std::string & to_update, std::string update_with) {
	if (to_update == "") {
		to_update = update_with;
	}
}