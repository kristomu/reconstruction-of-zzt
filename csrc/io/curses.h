#pragma once

#include "special_keys.h"
#include "colors.h"
#include "io.h"

#include <ncurses.h>

#include <vector>
#include <string>
#include <string.h>

// Everything works in terms of DOS colors and characters. The ZZT
// conversion doesn't know about any others.

typedef int64_t key_response;

// The curses IO class implicitly assumes that we're running on a Unicode
// terminal. Printing all the DOS code page 437 characters requires something
// more powerful than just the ACS_* macros, so the code assumes that
// wchar_int values do in fact represent UTF-8 codepoints.

class curses_io : public io {
	private:
		WINDOW * window;
		const int E_KEY_NONE = -33;

		key_response last_key_detected;

		mutable dos_color current_fg, current_bg;
		bool black_and_white;
		bool was_blocking;
		bool ignore_lack_of_keys;

		void use_color(dos_color fg, dos_color bg) const;
		short dos_color_to_curses(dos_color color) const;
		int linear(int x, int y, int xsize) const;
		bool prepare_colors();

		key_response parse(wchar_t unparsed, bool special_ncurses) const;

	public:
		curses_io();
		~curses_io();

		void move(int x, int y) {
			wmove(window, y, x);
		}

		bool supports_colors() {
			return has_colors();
		}

		void set_black_and_white(bool BW) {
			black_and_white = BW;
		}

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

		void redraw() const {
			wrefresh(window);
		}
		void clrscr() const {
			wclear(window);
		}

		int window_max_x() const {
			return getmaxx(window);
		}
		int window_max_y() const {
			return getmaxy(window);
		}

		void set_window_boundaries(int left, int up, int right,
			int down) {}; // NOP, currently
		void clear_scr() {
			wclear(window);
		}

		void set_blocking();
		void set_nonblocking();

		bool key_pressed();
		key_response read_key();
		key_response read_key_blocking();
		void flush_keybuf();
};