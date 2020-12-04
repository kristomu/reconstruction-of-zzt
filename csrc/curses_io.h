#pragma once

#include <ncurses.h>

#include <vector>
#include <string>
#include <string.h>

// Everything works in terms of DOS colors and characters. The ZZT
// conversion doesn't know about any others.

// This thing emulates DOS characters by using Unicode.
class dos_emulation {
	private:
		std::vector<std::string> uchars;

	public:
		dos_emulation();
		std::string unicode(unsigned char dos_char) const;
};

enum dos_color{ Black = 0,
				Blue = 1,
				Green = 2,
             	Cyan = 3,
             	Red = 4,
             	Magenta = 5,
             	Brown = 6,
             	LightGray = 7,
             	DarkGray = 8,
             	LightBlue = 9,
             	LightGreen = 10,
             	LightCyan = 11,
             	LightRed = 12,
             	LightMagenta = 13,
             	Yellow = 14,
             	White = 15
};

class curses_io {

	WINDOW * window;

	private:
		dos_emulation interpreter;

		mutable dos_color current_fg, current_bg;
		bool black_and_white;

		void use_color(dos_color fg, dos_color bg) const;
		short dos_color_to_curses(dos_color color) const;
		int linear(int x, int y, int xsize) const;
		bool prepare_colors();

	public:
		curses_io();
		~curses_io();

		void move(int x, int y) {
			wmove(window, y, x);
		}

		bool supports_colors() { return has_colors(); }

		void set_black_and_white(bool BW) { black_and_white = BW; }

		void show_cursor() const;
		void hide_cursor() const;

		void set_color(dos_color fg, dos_color bg) const;
		void set_text_color(dos_color fg) const;
		void set_background_color(dos_color bg) const;

		//bool print(const char * str) const;
		bool print(const std::string str) const;
		bool print(int x, int y, const char * str) const;
		bool print(int x, int y, const char * str, size_t maxlen) const;
		bool print(int x, int y, const std::string str) const;

		bool print_ch(int x, int y, char to_print) const;
		bool print_ch(int x, int y, dos_color fg, dos_color bg,
			char to_print) const;
		bool print_col(int x, int y, dos_color fg, dos_color bg,
			const std::string str) const;
		// TODO: Replace this with DOS char mapping.
		bool print_ext(int x, int y, dos_color fg, dos_color bg,
			const std::vector<short> & ext) const;

		void redraw() const { wrefresh(window); }
		void clrscr() const { wclear(window); }

		int window_max_x() const { return getmaxx(window); }
		int window_max_y() const { return getmaxy(window); }

		void set_window_boundaries(int left, int up, int right,
			int down) {}; // NOP, currently
		void clear_scr() { wclear(window); }

		bool key_pressed() const;
		char read_key();
		char read_key_blocking();
};