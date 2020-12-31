#pragma once

#include <string>
#include <fstream>

bool is_IO_error();
std::string get_error_string();

std::ifstream OpenForRead(std::string name);
std::ofstream OpenForWrite(std::string name);