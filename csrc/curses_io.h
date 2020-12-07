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

const int64_t E_KEY_UP = -1,
	E_KEY_DOWN = -2,
	E_KEY_RIGHT = -3,
	E_KEY_LEFT = -4,
	E_KEY_NUMPAD_CLEAR = -5,	// the one in the center of the numpad
	E_KEY_INSERT = -6,
	E_KEY_HOME = -7,
	E_KEY_PAGE_UP = -8,
	E_KEY_PAGE_DOWN = -9,
	E_KEY_DELETE = -10,
	E_KEY_END = -11,
	E_KEY_F1 = -12,
	E_KEY_F2 = -13,
	E_KEY_F3 = -14,
	E_KEY_F4 = -15,
	E_KEY_F5 = -16,
	E_KEY_F6 = -17,
	E_KEY_F7 = -18,
	E_KEY_F8 = -19,
	E_KEY_F9 = -20,
	E_KEY_F10 = -21,
	E_KEY_F11 = -22,
	E_KEY_F12 = -23,
	E_KEY_PAUSE = -24,
	E_KEY_UNKNOWN = -25,
	E_KEY_ESCAPE = -26,
	E_KEY_ENTER = -27,
	E_KEY_TAB = -28,
	E_KEY_BACKSPACE = -29;

struct key_response {
	int64_t key = E_KEY_UNKNOWN; // if < 0, it's an extended key, otherwise a literal
	bool alt = false, ctrl = false, shift = false;
};

// The curses IO class implicitly assumes that we're running on an ANSI
// terminal with unicode support. It's very hard to do proper input and
// output otherwise; I, at least, have not found a way to do so. Using
// keypad(TRUE) causes collisions: some function key values now occupy
// the same space as non-function keys (ĉ and F1 keys), and there's no
// way (seemingly) to distinguish them using the external ncurses
// interface.

// Similarly, being able to print all the DOS code page 437 characters
// requires something more powerful than just the ACS_* macros, so the code
// assumes that wchar_int values do in fact represent UTF-8 codepoints.

class curses_io {

	WINDOW * window;

	private:
		dos_emulation interpreter;

		mutable dos_color current_fg, current_bg;
		bool black_and_white;
		bool was_blocking;

		void use_color(dos_color fg, dos_color bg) const;
		short dos_color_to_curses(dos_color color) const;
		int linear(int x, int y, int xsize) const;
		bool prepare_colors();

		key_response parse_extended(std::wstring unparsed) const;

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

		bool print_ch(int x, int y, unsigned char to_print) const;
		bool print_ch(int x, int y, dos_color fg, dos_color bg,
			unsigned char to_print) const;
		bool print_ch(int x, int y, char packed_color,
			unsigned char to_print) const;
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

		void set_blocking();
		void set_nonblocking();

		bool key_pressed() const;
		key_response read_key();
		key_response read_key_blocking();
};