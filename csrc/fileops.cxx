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

/*https://forum.lazarus.freepascal.org/index.php/topic,46911.0.html*/
string ErrorString(integer e) {
    string numericError;

    string ErrorString_result;
    switch (e) {
    case 1: ErrorString_result = "Invalid function number"; break;
    case 2: ErrorString_result = "File not found."; break;
    case 3: ErrorString_result = "Path not found."; break;
    case 4: ErrorString_result = "Too many open files."; break;
    case 5: ErrorString_result = "Access denied."; break;
    case 6: ErrorString_result = "Invalid file handle."; break;
    case 12: ErrorString_result = "Invalid file-access mode."; break;
    case 15: ErrorString_result = "Invalid disk number."; break;
    case 16: ErrorString_result = "Cannot remove current directory.";
        break;
    case 17: ErrorString_result = "Cannot rename across volumes.";
        break;
    case 100: ErrorString_result = "Error when reading from disk.";
        break;
    case 101: ErrorString_result = "Error when writing to disk.";
        break;
    case 102: ErrorString_result = "File not assigned.";
        break;
    case 103: ErrorString_result = "File not open.";
        break;
    case 104: ErrorString_result = "File not opened for input.";
        break;
    case 105: ErrorString_result = "File not opened for output.";
        break;
    case 106: ErrorString_result = "Invalid number.";
        break;
    case 150: ErrorString_result = "Disk is write protected.";
        break;
    case 151: ErrorString_result = "Unknown device.";
        break;
    case 152: ErrorString_result = "Drive not ready.";
        break;
    case 153: ErrorString_result = "Unknown command.";
        break;
    case 154: ErrorString_result = "CRC check failed.";
        break;
    case 155: ErrorString_result = "Invalid drive specified..";
        break;
    case 156: ErrorString_result = "Seek error on disk.";
        break;
    case 157: ErrorString_result = "Invalid media type.";
        break;
    case 158: ErrorString_result = "Sector not found.";
        break;
    case 159: ErrorString_result = "Printer out of paper.";
        break;
    case 160: ErrorString_result = "Error when writing to device.";
        break;
    case 161: ErrorString_result = "Error when reading from device.";
        break;
    case 162: ErrorString_result = "Hardware failure.";
        break;
    case 200: ErrorString_result = "Division by zero";
        break;
    case 201: ErrorString_result = "Range check error";
        break;
    case 202: ErrorString_result = "Stack overflow error";
        break;
    case 203: ErrorString_result = "Heap overflow error";
        break;
    case 204: ErrorString_result = "Invalid pointer operation";
        break;
    case 205: ErrorString_result = "Floating point overflow";
        break;
    case 206: ErrorString_result = "Floating point underflow";
        break;
    case 207: ErrorString_result = "Invalid floating point operation";
        break;
    case 210: ErrorString_result = "Object not initialized";
        break;
    case 211: ErrorString_result = "Call to abstract method";
        break;
    case 212: ErrorString_result = "Stream registration error";
        break;
    case 213: ErrorString_result = "Collection index out of range";
        break;
    case 214: ErrorString_result = "Collection overflow error";
        break;
    case 215: ErrorString_result = "Arithmetic overflow error";
        break;
    case 216: ErrorString_result = "General protection fault";
        break;
    default: {
        str(e, numericError);
        ErrorString_result = string('#') + numericError;
    }
    }
    return ErrorString_result;
}

void OpenForRead(untyped_file& f, longint l) {
    throw std::logic_error("Open a file for writing, needs file size");
    /*FileMode = FILE_READ_ONLY;
    reset(f, l);*/
}

void OpenForRead(text& f) {
    /*FileMode = FILE_READ_ONLY;*/
    reset(f, NULL, NULL, ioResult);
    //reset(f);
}

void OpenForWrite(untyped_file& f, longint l) {
    throw std::logic_error("Open a file for writing, needs file size");
    /* Somewhat of a kludge to make it work both with existing
    write-only files and files that don't exist yet. It might
    	  have trouble with write-only directories. I don't know why
    	  Rewrite doesn't respect FileMode yet Reset does... */

    /*FileMode = FILE_WRITE_ONLY;
    reset(f, l);
    if (ioResult == 0)  return;*/

    /* Couldn't open existing file. Try opening a new one. */
    /*rewrite(f, l);*/
}

void OpenForWrite(text& f) {
    /*FileMode = FILE_WRITE_ONLY;*/
    reset(f, NULL, NULL, ioResult);
    if (ioResult == 0)  return;
    rewrite(f, NULL, NULL, ioResult);
}