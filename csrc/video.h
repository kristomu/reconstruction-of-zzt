#pragma once

#include <vector>
#include <string>
#include <array>
#include <memory>

#include "input.h"
#include "io/io.h"
#include "ptoc.h"

typedef std::string video_line;

struct TTextChar {
	char Char;
	unsigned char Color;
};

// TODO: How does video know that we're interested in 80x25? Should be
// parametric somehow. As it is, write will reject writing to coordinates
// outside this buffer. Consider in particular the user changing the terminal
// dimensions - how should video_buffer change?
typedef std::array<std::array<TTextChar, 25>, 80> video_buffer;

// Every coordinate specification is zero-based (i.e. (0,0) is upper left).

class Video {
	private:
		std::shared_ptr<io> display_interface = NULL;
		video_buffer primary_buffer, secondary_buffer;
		TTextChar passthrough;

		bool chose_monochrome;

	public:
		~Video();

		void install(dos_color borderColor,
			std::shared_ptr<io> io_interface_in);

		// Set background and foreground colors.
		void TextBackground(dos_color bgColor);
		void TextColor(dos_color fgColor);

		void ClrScr();
		void redraw();
		void go_to_xy(int x, int y); // zero-based coordinates

		void write(int x, int y, const TTextChar & to_print);
		void write(int x, int y, char color, char to_print);
		void write(int x, int y, char color, const char * text);
		void write(int x, int y, char color, std::string text);

		void write(std::string text);
		void writeln(std::string text);

		bool Configure(Input & key_input);

		void ShowCursor();
		void HideCursor();
		void SetBorderColor(dos_color value);

		bool is_monochrome() const {
			return chose_monochrome;
		}

		// Copy between display (and primary) and secondary buffer.
		void Copy(int x_from, int y_from, int width, int height,
			bool to_display);

		void Refresh() {
			display_interface->redraw();
		}
};