#pragma once

#include "special_keys.h"
#include "colors.h"
#include "io.h"

#include <vector>

class stub_io : public io {
	private:
		std::vector<key_response> key_responses;
		std::vector<key_response>::const_iterator response_pos;

	public:
		stub_io() {
			response_pos = key_responses.begin();
		}

		void move(int x, int y) {}

		bool supports_colors() {
			return true;
		}

		void set_black_and_white(bool BW) {}

		void show_cursor() const {}
		void hide_cursor() const {}

		void set_color(dos_color fg, dos_color bg) const {}
		void set_text_color(dos_color fg) const {}
		void set_background_color(dos_color bg) const {}

		bool print(const std::string str) const {
			return true;
		}
		bool print(int x, int y, const std::string str) const {
			return true;
		}

		bool print_ch(int x, int y, unsigned char to_print) const {
			return true;
		}
		bool print_ch(int x, int y, dos_color fg, dos_color bg,
			unsigned char to_print) const {
			return true;
		}
		bool print_ch(int x, int y, unsigned char packed_color,
			unsigned char to_print) const {
			return true;
		}

		void redraw() const {}
		void clrscr() const {}
		int window_max_x() const {
			return 80;
		}
		int window_max_y() const {
			return 25;
		}

		void set_window_boundaries(int left, int up, int right,
			int down) {};
		void clear_scr() {}
		void set_blocking() {}
		void set_nonblocking() {}

		bool key_pressed();
		key_response read_key();
		key_response read_key_blocking();
		void flush_keybuf();

		void set_key_responses(std::vector<key_response> responses_in);
};