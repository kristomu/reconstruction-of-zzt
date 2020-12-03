#pragma once

#include <ncurses.h>

#include <vector>
#include <string>
#include <string.h>

// This thing emulates DOS characters by using Unicode.
class dos_emulation {
	private:
		std::vector<std::string> uchars;

	public:
		dos_emulation();
		std::string unicode(unsigned char dos_char) const;
};


class curses {

	WINDOW * window;

	private:
		dos_emulation interpreter;

		mutable int current_fg = 7, current_bg = 0;

		void use_color(int fg, int bg) const;
		short dos_color_to_curses(int color) const;
		int linear(int x, int y, int xsize) const;
		bool prepare_colors();

	public:
		curses();
		~curses();

		void move(int x, int y) {
			wmove(window, y, x);
		}

		void set_dos_color(int fg, int bg) const;
		void set_dos_text_color(int fg) const;
		void set_dos_background_color(int bg) const;

		//bool print(const char * str) const;
		bool print(const std::string str) const;
		bool print(int x, int y, const char * str) const;
		bool print(int x, int y, const char * str, size_t maxlen) const;
		bool print(int x, int y, const std::string str) const;

		bool print_ch(int x, int y, char to_print) const;
		bool print_ch(int x, int y, int fg, int bg, char
				to_print) const;
		bool print_col(int x, int y, int fg, int bg, const std::string str) const;
		// TODO: Replace this with DOS char mapping.
		bool print_ext(int x, int y, int fg, int bg, const std::vector<short> & ext) const;

		void redraw() const { refresh(); }

		int window_max_x() const { return getmaxx(window); }
		int window_max_y() const { return getmaxy(window); }

		void set_window_boundaries(int left, int up, int right,
			int down) {}; // NOP, currently
		void clear_scr() { wclear(window); }
};