#include "ptoc.h"
#include "hardware.h"
#include "video.h"

#include <unistd.h>
#include <memory>

/*
	Copyright (c) 2020 Adrian Siekierka
	Copyright (c) 2020 Kristofer Munsterhjelm

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

// Set background color.
void Video::TextBackground(dos_color bgColor) {
	display_interface->set_background_color(bgColor);
}

// Set foreground color.
void Video::TextColor(dos_color fgColor) {
	display_interface->set_text_color(fgColor);
}

// Clear the screen
void Video::ClrScr() {
	display_interface->clrscr();
}

void Video::redraw() {
	display_interface->redraw();
}

// Go to (x, y).
void Video::go_to_xy(int x, int y) {
	display_interface->move(x, y);
}

/* The input x,y values are offset by 0, i.e. 0,0 is upper left. */
void Video::write(int x, int y, const TTextChar & to_print) {
	display_interface->print_ch(x, y, to_print.Color, to_print.Char);
	primary_buffer[x][y] = to_print;
}

void Video::write(int x, int y, char color, char to_print) {
	// Call the prior function so we're sure to always be writing to the
	// primary buffer.

	passthrough.Color = color;
	passthrough.Char = to_print;
	write(x, y, passthrough);
}

void Video::write(int x, int y, char color, const char * text) {

	const char * cur_text_char = text;
	int cidx = 0;

	while (*cur_text_char != 0) {
		TTextChar cur_char;
		cur_char.Char = text[cidx];
		cur_char.Color = color;

		//if (x+offset >= terminalWidth)  return;

		write(x+cidx, y, cur_char);
		++cidx;
		++cur_text_char;
	}

}

void Video::write(int x, int y, char color, video_line text) {
	for (int cidx = 0; cidx < text.size(); cidx ++) {
		TTextChar cur_char;
		cur_char.Char = text[cidx];
		cur_char.Color = color;

		write(x+cidx, y, cur_char);
	}
}

void Video::write(std::string text) {
	display_interface->print(text);
}

void Video::writeln(std::string text) {
	write(text);
	write("\n");
}

bool Video::Configure(Input & key_input) {

	bool MonochromeOnly = !has_colors();
	if (MonochromeOnly)  {
		chose_monochrome = true;
		return true;
	}

	writeln("");
	write("  Video mode:  C)olor,  M)onochrome?  ");

	bool gotResponse = false;
	int64_t typed = keyUpCase(key_input.read_key_blocking());

	switch (typed) {
		case 'C': chose_monochrome = false; break;
		case 'M': chose_monochrome = true; break;
		case E_KEY_ESCAPE: chose_monochrome = MonochromeOnly; break;
		default: gotResponse = false; break;
	}

	return typed != E_KEY_ESCAPE;
}

void Video::install(dos_color borderColor, std::shared_ptr<io> io_in) {

	display_interface = io_in;
	display_interface->set_black_and_white(chose_monochrome);

	if (!chose_monochrome) {
		TextBackground(borderColor);
	}

	SetBorderColor(borderColor);
}

Video::~Video() {
	if (display_interface) {
		TextBackground(Black);
		SetBorderColor(Black);
		ClrScr();
	}
}

/* These do nothing in Linux, but are meant to show or hide the terminal cursor.
It will have to be done in some other way. TODO */
void Video::ShowCursor() {
	display_interface->show_cursor();
}

void Video::HideCursor() {
	display_interface->hide_cursor();
}

/* This does nothing in Linux either. I'm keeping the empty function in case
someone who makes e.g. an SDL version would like to implement it. */
void Video::SetBorderColor(dos_color value) {
}

/* X,Y are zero-indexed. If toVideo is true, it copies stored
  character/color data from the given array to screen by using write(). If
  toVideo is false, it copies the character/color data from the video memory
  mirror to the given array. */
void Video::Copy(int x_from, int y_from, int width, int height,
	bool to_display) {

	int x, y;

	for (y = y_from; y < y_from + height; ++y) {
		for (x = x_from; x < x_from + width; x ++) {
			if (to_display) {
				write(x, y, secondary_buffer[x][y]);
			} else {
				secondary_buffer[x][y] = primary_buffer[x][y];
			}
		}
	}
	if (to_display) {
		redraw();
	}
}