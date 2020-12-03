#include "ptoc.h"

/*
	Copyright (c) 2020 Adrian Siekierka

	Based on a reconstruction of code from ZZT,
	Copyright 1991 Epic MegaGames, used with permission.

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

/*$I-*/
#define __Video_implementation__


#include "videounix.h"

#include "dos.h"

integer VideoColumns;
integer VideoBorderColor;
word VideoTextSegment;
pointer VideoTextPointer;
boolean VideoCursorVisible;

void WriteUnicodeAsUTF8(smallint codepoint) {
    if (codepoint < 0x80)
        output << chr(codepoint);
    else if (codepoint < 0x800)  {
        output << chr((((cardinal)codepoint >>  6) & 0x1f) | 0xc0);
        output << chr((((cardinal)codepoint >>  0) & 0x3f) | 0x80);
    } else {       /* codepoint < $10000 */
        output << chr((((cardinal)codepoint >> 12) & 0xf) | 0xe0);
        output << chr((((cardinal)codepoint >>  6) & 0x3f) | 0x80);
        output << chr((((cardinal)codepoint >>  0) & 0x3f) | 0x80);
    }
}

/*$F+*/

/* The input x,y values are offset by 0, i.e. 0,0 is upper left. */
void VideoWriteTextAsUTF8(byte x, byte y, byte color, TVideoLine text) {
    integer offset;
    char c;

    integer terminalWidth;
    integer terminalHeight;
    integer charPseudoEnd;


    /*Get the terminal height and width to avoid printing
    outside it.
    	 https://stackoverflow.com/questions/26776980 */

    terminalWidth = WindMaxX - WindMinX + 1;
    terminalHeight = WindMaxY - WindMinY + 1;

    if (y >= terminalHeight)  return;

    if (color > 0x7f)
        TextColor((color & 0xf) + Blink);
    else
        TextColor(color & 0xf);
    TextBackground((cardinal)color >> 4);

    /* Hack from https://stackoverflow.com/a/35140822
    A better solution will have to move away from Crt altogether.*/

    /* Possible performance improvement: extract the contents of
    this loop to a separate procedure. */
    for( cidx = 1; cidx <= length(text); cidx ++) {
        if (x+offset >= terminalWidth)  return;

        /* The performance enhancement below doesn't work due to
        a strange bug I can't be arsed to track down. Somehow
        	  MainBuffer is going out of sync. TODO, fix */

        /*
        if (MainBuffer[x+offset+1][y+1].Color = color) and
        		   (MainBuffer[x+offset+1][y+1].Char = text[cidx]) then begin
        			offset := offset + 1;
        			Continue;
        		end;
        		*/

        /* Since Crt believes we're outputting ASCII, it'll
        "helpfully" scroll the terminal if we output multi-
        	  char unicode while at the very lower right. There's
        	  no way to avoid this misfeature, so just don't print
        	  it in that case. */
        charPseudoEnd = x+offset+UTF8Len(CP437ToCodepoint(ord(text[cidx])));
        if ((y == terminalHeight-1) && (charPseudoEnd >= terminalWidth))
            continue_;

        GotoXY(1, 1);
        GotoXY(x+offset+1, y+1);
        MainBuffer[x+offset+1][y+1].Color = color;
        MainBuffer[x+offset+1][y+1].Char = text[cidx];

        /* For the same reason, it'll corrupt every "too-wide" utf8
        point, so if we have any of those, just print a black-on-grey
        	  question mark. */
        if (charPseudoEnd > terminalWidth)  {
            TextColor(0);
            TextBackground(0x7);
            output << '?';
        } else
            WriteUnicodeAsUTF8(CP437ToCodepoint(ord(c)));

        offset = offset + 1;
    }
    /* Move the cursor out of the way of the playing field. */
    if ((! VideoCursorVisible) && (terminalWidth > 61))
        GotoXY(61, 1);
}

void VideoWriteTextColor(byte x, byte y, byte color, TVideoLine text) {
    VideoWriteTextAsUTF8(x, y, color, text);
}

void VideoWriteTextBW(byte x, byte y, byte color, TVideoLine text) {
    if ((color & 0x8) == 0x8)  {
        if ((color & 0xf0) == 0)
            color = 0xf;
        else
            color = 0x7f;
    } else {
        if ((color & 0x7) != 0)
            color = 0x7;
        else
            color = 0x70;
    }
    VideoWriteTextAsUTF8(x, y, color, text);
}

/*$F-*/

void VideoWriteText(byte x, byte y, byte color, TVideoLine text) {
    if (VideoMonochrome)
        VideoWriteTextBW(x, y, color, text);
    else
        VideoWriteTextColor(x, y, color, text);
}

/* Does nothing in Linux. The point in DOS is to change the charset from 9x16 to
8x14. It will have to be done in some other way in Linux. I'm keeping the empty
function as reminder to myself. TODO */
void VideoToggleEGAMode(boolean EGA) {
    ;
}

boolean VideoConfigure() {
    char charTyped;

    boolean VideoConfigure_result;
    charTyped = ' ';
    if (LastMode == 7)  {
        VideoMonochrome = true;
    } else {
        output << NL;
        output << "  Video mode:  C)olor,  M)onochrome?  ";
        do {
            do {
                /* Don't busy-wait too much. */
                Delay(100);
            } while (!keypressed());
            charTyped = upcase(readkey());
        } while (!(set::of('\33', 'C', 'M', eos).has(charTyped)));
        switch (charTyped) {
        case 'C': VideoMonochrome = false; break;
        case 'M': VideoMonochrome = true; break;
        case '\33': VideoMonochrome = (LastMode == 7); break;
        }
    }
    VideoConfigure_result = charTyped != '\33';
    return VideoConfigure_result;
}

void VideoInstall(integer columns, integer borderColor) {
    VideoToggleEGAMode(true);

    if (! VideoMonochrome)
        TextBackground(borderColor);

    VideoColumns = columns;
    if (VideoMonochrome)  {
        if (set::of(0, 1, 2, 3, eos).has(LastMode))  {
            if (columns == 80)  {
                TextMode(BW80);
            } else {
                TextMode(BW40);
            }
        } else {
            TextMode(7);
            VideoColumns = 80;
        }
    } else {
        if (VideoColumns == 80)  {
            TextMode(CO80);
        } else {
            TextMode(CO40);
        }
        if (! VideoMonochrome)
            TextBackground(borderColor);
        ClrScr;
    }
    if (! VideoCursorVisible)
        VideoHideCursor();
    VideoSetBorderColor(borderColor);
}

void VideoUninstall() {
    VideoToggleEGAMode(false);
    TextBackground(0);
    VideoColumns = 80;
    if (VideoMonochrome)
        TextMode(BW80);
    else
        TextMode(CO80);
    VideoSetBorderColor(0);
    ClrScr;
}

/* These do nothing in Linux, but are meant to show or hide the terminal cursor.
It will have to be done in some other way. TODO */
void VideoShowCursor() {
    VideoCursorVisible = true;
}

void VideoHideCursor() {
    VideoCursorVisible = false;
}

/* This does nothing in Linux either. I'm keeping the empty function in case
someone who makes e.g. an SDL version would like to implement it. */
void VideoSetBorderColor(integer value) {
    ;
}

/* X,Y are zero-indexed. If toVideo is true, it copies stored
  character/color data from the given array to screen by using WriteText. If
  toVideo is false, it copies the character/color data from the video memory
  mirror to the given array. */
void VideoCopy(integer xfrom, integer yfrom, integer width, integer height,
               TVideoBuffer& buf,
               boolean toVideo) {
    integer x, y;


    for( y = yfrom; y <= yfrom + height - 1; y ++)
        for( x = xfrom; x <= xfrom + width - 1; x ++) {
            if (toVideo)
                VideoWriteText(x, y,
                               buf[x+1][y+1].Color,
                               buf[x+1][y+1].Char);
            else
                buf[x+1][y+1] = MainBuffer[x+1][y+1];
        }
}

class unit_Video_initialize {
public: unit_Video_initialize();
};
static unit_Video_initialize Video_constructor;

unit_Video_initialize::unit_Video_initialize() {
    VideoBorderColor = 0;
    VideoColumns = 80;
    if (LastMode == 7)  {
        VideoTextSegment = 0xb000;
        VideoMonochrome = true;
    } else {
        VideoTextSegment = 0xb800;
        VideoMonochrome = false;
    }
    VideoTextPointer = Ptr(VideoTextSegment, 0);
    VideoCursorVisible = true;
}
