#include "ptoc.h"

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

/* So as not to have to pull in a whole mathematics library just for min and
  max. */

#define __Unicode_implementation__


#include "unicode.h"


integer UTF8Len(smallint codepoint) {
    integer UTF8Len_result;
    if (codepoint < 0x80)  UTF8Len_result = 1;
    else if (codepoint < 0x800)  UTF8Len_result = 2;
    else UTF8Len_result = 3;
    return UTF8Len_result;
}

void SetupCodepointToCP437() {
    integer i;

    /* Everything that's not known maps to the character '?' */
    for (i = 0; i < 65536; ++i) {
        UnicodeToCP437[i] = '?';
    }
    /* First reverse the cp437 to unicode codepoint table. */
    for( i = 0; i <= 255; i ++)
        UnicodeToCP437[cp437ToUnicode[i]] = chr(i);

    /* Then add some "near-equivalents" */
    /*lowercase ø: make it look like phi*/
    UnicodeToCP437[248] = chr(237);

    /*uppercase Ø: make it look like Theta*/
    UnicodeToCP437[216] = chr(233);
}

char CodepointToCP437(longint cp) {
    char CodepointToCP437_result;
    if (cp > 65535)  CodepointToCP437_result = '?';
    else CodepointToCP437_result = UnicodeToCP437[cp];
    return CodepointToCP437_result;
}

smallint CP437ToCodepoint(byte c) {
    smallint CP437ToCodepoint_result;
    CP437ToCodepoint_result = cp437ToUnicode[ord(c)];
    return CP437ToCodepoint_result;
}


