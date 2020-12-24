#pragma once

#include <vector>
#include <string>
#include <array>
#include <memory>

#include "curses_io.h"
#include "ptoc.h"

typedef std::string TVideoLine;

struct TTextChar {
	char Char;
	unsigned char Color;
};

typedef std::array<std::array<TTextChar, 25>, 80> TVideoBuffer;

extern boolean VideoMonochrome;

// ??? We're having a kind of delegation problem. WriteLn is in curses.
// But this handles VideoWriteText... so if we address io directly, we
// might go out of sync. That suggests that everything should be in
// curses - or in some ABC that curses inherits from. Hm...

class Video {
	private:
		std::shared_ptr<curses_io> io;
		TVideoBuffer primary_buffer, secondary_buffer;
		TTextChar passthrough;

	public:
		void VideoWriteText(int x, int y, const TTextChar & to_print);
		void VideoWriteText(int x, int y, char color, char to_print);
		void VideoWriteText(int x, int y, char color, const char * text);
		void VideoWriteText(int x, int y, char color, TVideoLine text);
		bool VideoConfigure();
		void VideoShowCursor() {
			io->show_cursor();
		}
		void VideoHideCursor() {
			io->hide_cursor();
		}
		void VideoSetBorderColor(dos_color value);

		void VideoInstall(integer columns, dos_color borderColor,
			std::shared_ptr<curses_io> io_in);
		void VideoUninstall();

		// Copy between display (and primary) and secondary buffer.
		void VideoCopy(int x_from, int y_from, int width, int height,
			bool to_display);

		void VideoRefresh() {
			io->redraw();
		}
};