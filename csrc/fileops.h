#include <string>
#include <fstream>

#ifndef __fileops_h__
#define __fileops_h__

std::ifstream OpenForRead(std::string name);
std::ofstream OpenForWrite(std::string name);

#endif
