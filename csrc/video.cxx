#include "ptoc.h"
#include "hardware.h"
#include "video.h"

#include <unistd.h>
#include <memory>

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

#include "unicode.h"

boolean VideoMonochrome;

/* The input x,y values are offset by 0, i.e. 0,0 is upper left. */
void Video::VideoWriteText(int x, int y, const TTextChar & to_print) {
	io_interface->print_ch(x, y, to_print.Color, to_print.Char);
	primary_buffer[x][y] = to_print;
}

void Video::VideoWriteText(int x, int y, char color, char to_print) {
	// Call the prior function so we're sure to always be writing to the
	// primary buffer.

	passthrough.Color = color;
	passthrough.Char = to_print;
	VideoWriteText(x, y, passthrough);
}

void Video::VideoWriteText(int x, int y, char color, const char * text) {

	const char * cur_text_char = text;
	int cidx = 0;

	while (*cur_text_char != 0) {
		TTextChar cur_char;
		cur_char.Char = text[cidx];
		cur_char.Color = color;

		//if (x+offset >= terminalWidth)  return;

		VideoWriteText(x+cidx, y, cur_char);
		++cidx;
		++cur_text_char;
	}

}

void Video::VideoWriteText(int x, int y, char color, video_line text) {
	integer offset;
	char c;

	/*integer terminalWidth;
	integer terminalHeight;
	integer charPseudoEnd;*/

	/*Get the terminal height and width to avoid printing
	outside it. TODO: Determine the dimensions at the moment of the call.
		 https://stackoverflow.com/questions/26776980 */

	// Should no longer be necessary with curses.

	/*terminalWidth = WindMaxX - WindMinX + 1;
	terminalHeight = WindMaxY - WindMinY + 1;*/

	//if (y >= terminalHeight)  return;

	// With the curses class, blink works out of the box.
	// TODO: Deal with unicode when that time happens. The conversion
	// between DOS characters and Unicode should either happen here or
	// in curses - I'm inclined to think that it should happen in curses,
	// though putting it here would make "Unicode ZZT" much easier to
	// handle.

	for (int cidx = 0; cidx < text.size(); cidx ++) {
		TTextChar cur_char;
		cur_char.Char = text[cidx];
		cur_char.Color = color;

		//if (x+offset >= terminalWidth)  return;

		VideoWriteText(x+cidx, y, cur_char);
	}

	return;
}

/* Does nothing in Linux. The point in DOS is to change the charset from 9x16 to
8x14. It will have to be done in some other way in Linux. I'm keeping the empty
function as reminder to myself. TODO */
void VideoToggleEGAMode(boolean EGA) {
}

bool Video::VideoConfigure() {
	char charTyped;

	boolean VideoConfigure_result;
	charTyped = ' ';
	bool MonochromeOnly =
		!HasColors(); // ???? Do something here with has_colors().
	if (MonochromeOnly)  {
		VideoMonochrome = true;
		return true;
	}

	display_writeln("");
	display_write("  Video mode:  C)olor,  M)onochrome?  ");

	bool gotResponse = false;
	int64_t typed;

	while (!gotResponse) {
		typed = keyUpCase(ReadKeyBlocking());
		gotResponse = true;

		switch (typed) {
			case 'C': VideoMonochrome = false; break;
			case 'M': VideoMonochrome = true; break;
			case E_KEY_ESCAPE: VideoMonochrome = MonochromeOnly; break;
			default: gotResponse = false; break;
		}
	}
	return typed != E_KEY_ESCAPE;
}

void Video::VideoInstall(integer columns, dos_color borderColor,
	std::shared_ptr<io> io_in) {

	io_interface = io_in;
	VideoToggleEGAMode(true);

	if (! VideoMonochrome) {
		TextBackground(borderColor);
	}

	/*VideoColumns = columns;
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
	}*/
	/*    if (! VideoCursorVisible)
	        VideoHideCursor();*/
	VideoSetBorderColor(borderColor);
}

void Video::VideoUninstall() {
	VideoToggleEGAMode(false);
	TextBackground(Black);
	/*VideoColumns = 80;
	if (VideoMonochrome)
	    TextMode(BW80);
	else
	    TextMode(CO80);*/
	VideoSetBorderColor(Black);
	ClrScr();
}

/* These do nothing in Linux, but are meant to show or hide the terminal cursor.
It will have to be done in some other way. TODO */
void VideoShowCursor() {
	//VideoCursorVisible = true;
}

void VideoHideCursor() {
	//VideoCursorVisible = false;
}

/* This does nothing in Linux either. I'm keeping the empty function in case
someone who makes e.g. an SDL version would like to implement it. */
void Video::VideoSetBorderColor(dos_color value) {
}

/* X,Y are zero-indexed. If toVideo is true, it copies stored
  character/color data from the given array to screen by using WriteText. If
  toVideo is false, it copies the character/color data from the video memory
  mirror to the given array. */
void Video::VideoCopy(int x_from, int y_from, int width, int height,
	bool to_display) {

	int x, y;

	for (y = y_from; y < y_from + height; ++y) {
		for (x = x_from; x < x_from + width; x ++) {
			if (to_display) {
				VideoWriteText(x, y, secondary_buffer[x][y]);
			} else {
				secondary_buffer[x][y] = primary_buffer[x][y];
			}
		}
	}
	if (to_display) {
		redraw();
	}
}

// Border color should be 0, columns should be 80.