#pragma once
#include <string>

// Integer to string

std::string itos(int source);
std::string str_toupper(std::string s);
// like std::string::substr, but returns 0 if substring not found, like Pascal.
size_t string_pos(const std::string needle, const std::string haystack);
// update if the old string is empty.
void update(std::string & to_update, std::string update_with);