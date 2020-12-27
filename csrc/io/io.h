#pragma once

#include "special_keys.h"
#include "colors.h"
#include <vector>
#include <string>

class io {

	public:
		virtual void move(int x, int y) = 0;

		virtual bool supports_colors() = 0;

		virtual void set_black_and_white(bool BW) = 0;

		virtual void show_cursor() const = 0;
		virtual void hide_cursor() const = 0;

		virtual void set_color(dos_color fg, dos_color bg) const = 0;
		virtual void set_text_color(dos_color fg) const = 0;
		virtual void set_background_color(dos_color bg) const = 0;

		virtual bool print(const std::string str) const  = 0;
		virtual bool print(int x, int y, const std::string str) const  = 0;

		virtual bool print_ch(int x, int y, unsigned char to_print) const  = 0;
		virtual bool print_ch(int x, int y, dos_color fg, dos_color bg,
			unsigned char to_print) const  = 0;
		virtual bool print_ch(int x, int y, unsigned char packed_color,
			unsigned char to_print) const  = 0;

		virtual void redraw() const  = 0;
		virtual void clrscr() const  = 0;
		virtual int window_max_x() const  = 0;
		virtual int window_max_y() const  = 0;

		virtual void set_window_boundaries(int left, int up, int right,
			int down) = 0;
		virtual void clear_scr() = 0;
		virtual void set_blocking() = 0;
		virtual void set_nonblocking() = 0;

		virtual bool key_pressed() = 0;
		virtual key_response read_key() = 0;
		virtual key_response read_key_blocking() = 0;
		virtual void flush_keybuf() = 0;
};