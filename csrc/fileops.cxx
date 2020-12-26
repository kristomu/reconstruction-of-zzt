#include "ptoc.h"
#include "io.h"

/*
        Copyright (c) 2020 Kristofer Munsterhjelm

        Permission is hereby granted, free of charge, to any person obtaining a copy
        of this software and associated documentation files (the "Software"), to deal
        in the Software without restriction, including without limitation the rights
        to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
        copies of the Software, and to permit persons to whom the Software is
        furnished to do so, subject to the following conditions:

        The above copyright notice and this permission notice shall be included in all
        copies or substantial portions of the Software.

        THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
        IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
        FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
        LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
        OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
        SOFTWARE.
*/

/* A simple file-handling shim for Linux. All of these functions alter IOResult,
  as is the convention in ZZT. */

/*$I-*/

#define __Fileops_implementation__

#include "ptoc.h"
#include "fileops.h"
#include "dos.h"

#include <stdexcept>

std::ifstream OpenForRead(std::string name) {
	// TODO: Test if this overwrites files, etc...
	std::ifstream file(name);
	return file;
}

std::ofstream OpenForWrite(std::string name) {
	// TODO: Test if this overwrites files, etc...
	std::ofstream file(name);
	return file;
}