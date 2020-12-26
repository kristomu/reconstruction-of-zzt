#include "curses_io.h"
#include "unicode.h"
#include <stdexcept>
#include <ncurses.h>
#include <iostream>

// Put the stuff below into a class? Singleton for window? Eh...

#define NCURSES_INIT_FAILURE 0

// Split off a go-between class later.

dos_emulation::dos_emulation() {

	// Insert the Unicode strings. (Do this better at some later time?)

	uchars.push_back(" "); // 00
	uchars.push_back("\u263A");
	uchars.push_back("\u263B");
	uchars.push_back("\u2665");
	uchars.push_back("\u2666");
	uchars.push_back("\u2663");
	uchars.push_back("\u2660");
	uchars.push_back("\u2022");
	uchars.push_back("\u25D8");
	uchars.push_back("\u25CB");
	uchars.push_back("\u25D9");
	uchars.push_back("\u2642");
	uchars.push_back("\u2640");
	uchars.push_back("\u266A");
	uchars.push_back("\u266B");
	uchars.push_back("\u263C");
	uchars.push_back("\u25BA");
	uchars.push_back("\u25C4");
	uchars.push_back("\u2195");
	uchars.push_back("\u203C");
	uchars.push_back("\u00B6");
	uchars.push_back("\u00A7");
	uchars.push_back("\u25AC");
	uchars.push_back("\u21A8");
	uchars.push_back("\u2191");
	uchars.push_back("\u2193");
	uchars.push_back("\u2192");
	uchars.push_back("\u2190");
	uchars.push_back("\u221F");
	uchars.push_back("\u2194");
	uchars.push_back("\u25B2");
	uchars.push_back("\u25BC");

	// then come the alphanumerics, which are plain.
	for (int counter = 0x20; counter <= 0x7E; ++counter) {
		uchars.push_back("");
	}

	uchars.push_back("\u2302"); // Energizer, 0x7F.

	uchars.push_back("\u00C7");
	uchars.push_back("\u00FC");
	uchars.push_back("\u00E9");
	uchars.push_back("\u00E2");
	uchars.push_back("\u00E4");
	uchars.push_back("\u00E0");
	uchars.push_back("\u00E5");
	uchars.push_back("\u00E7");
	uchars.push_back("\u00EA");
	uchars.push_back("\u00EB");
	uchars.push_back("\u00E8");
	uchars.push_back("\u00EF");
	uchars.push_back("\u00EE");
	uchars.push_back("\u00EC");
	uchars.push_back("\u00C4");
	uchars.push_back("\u00C5");

	uchars.push_back("\u00C9");
	uchars.push_back("\u00E6");
	uchars.push_back("\u00C6");
	uchars.push_back("\u00F4");
	uchars.push_back("\u00F6");
	uchars.push_back("\u00F2");
	uchars.push_back("\u00FB");
	uchars.push_back("\u00F9");
	uchars.push_back("\u00FF");
	uchars.push_back("\u00D6");
	uchars.push_back("\u00DC");
	uchars.push_back("\u00A2");
	uchars.push_back("\u00A3");
	uchars.push_back("\u00A5");
	uchars.push_back("\u20A7");
	uchars.push_back("\u0192");

	uchars.push_back("\u00E1");
	uchars.push_back("\u00ED");
	uchars.push_back("\u00F3");
	uchars.push_back("\u00FA");
	uchars.push_back("\u00F1");
	uchars.push_back("\u00D1");
	uchars.push_back("\u00AA");
	uchars.push_back("\u00BA");
	uchars.push_back("\u00BF");
	uchars.push_back("\u2310");
	uchars.push_back("\u00AC");
	uchars.push_back("\u00BD");
	uchars.push_back("\u00BC");
	uchars.push_back("\u00A1");
	uchars.push_back("\u00AB");
	uchars.push_back("\u00BB");

	uchars.push_back("\u2591"); // water
	uchars.push_back("\u2592"); // break
	uchars.push_back("\u2593"); // normal
	uchars.push_back("\u2502");
	uchars.push_back("\u2524");
	uchars.push_back("\u2561");
	uchars.push_back("\u2562");
	uchars.push_back("\u2556");
	uchars.push_back("\u2555");
	uchars.push_back("\u2563");
	uchars.push_back("\u2551");
	uchars.push_back("\u2557");
	uchars.push_back("\u255D");
	uchars.push_back("\u255C");
	uchars.push_back("\u255B");
	uchars.push_back("\u2510");

	uchars.push_back("\u2514");
	uchars.push_back("\u2534");
	uchars.push_back("\u252C");
	uchars.push_back("\u251C");
	uchars.push_back("\u2500");
	uchars.push_back("\u253C");
	uchars.push_back("\u255E");
	uchars.push_back("\u255F");
	uchars.push_back("\u255A");
	uchars.push_back("\u2554");
	uchars.push_back("\u2569");
	uchars.push_back("\u2566");
	uchars.push_back("\u2560");
	uchars.push_back("\u2550");
	uchars.push_back("\u256C");
	uchars.push_back("\u2567");

	uchars.push_back("\u2568");
	uchars.push_back("\u2564");
	uchars.push_back("\u2565");
	uchars.push_back("\u2559");
	uchars.push_back("\u2558");
	uchars.push_back("\u2552");
	uchars.push_back("\u2553");
	uchars.push_back("\u256B");
	uchars.push_back("\u256A");
	uchars.push_back("\u2518");
	uchars.push_back("\u250C");
	uchars.push_back("\u2588");
	uchars.push_back("\u2584");
	uchars.push_back("\u258C");
	uchars.push_back("\u2590");
	uchars.push_back("\u2580");

	uchars.push_back("\u03B1");
	uchars.push_back("\u00DF");
	uchars.push_back("\u0393");
	uchars.push_back("\u03C0");
	uchars.push_back("\u03A3");
	uchars.push_back("\u03C3");
	uchars.push_back("\u00B5");
	uchars.push_back("\u03C4");
	uchars.push_back("\u03A6");
	uchars.push_back("\u0398");
	uchars.push_back("\u03A9");
	uchars.push_back("\u03B4"); // delta
	uchars.push_back("\u221E");
	uchars.push_back("\u03D5"); // phi or empty-set
	uchars.push_back("\u03B5");
	uchars.push_back("\u2229");

	uchars.push_back("\u2261"); // passage
	uchars.push_back("\u00B1");
	uchars.push_back("\u2265");
	uchars.push_back("\u2264");
	uchars.push_back("\u2320");
	uchars.push_back("\u2321");
	uchars.push_back("\u00F7");
	uchars.push_back("\u2248");
	uchars.push_back("\u00B0");
	uchars.push_back("\u2219");
	uchars.push_back("\u00B7");
	uchars.push_back("\u221A");
	uchars.push_back("\u207F");
	uchars.push_back("\u00B2");
	uchars.push_back("\u25A0");
	uchars.push_back("\u00A0");
}

std::string dos_emulation::unicode(unsigned char dos_char) const {
	if (dos_char < uchars.size() && uchars[dos_char].size() != 0) {
		return (uchars[dos_char]);
	}

	std::string defstr = "Q";
	defstr[0] = dos_char;
	return (defstr);
}

short curses_io::dos_color_to_curses(dos_color color) const {
	switch (color) {
		case 0: return (COLOR_BLACK);
		case 1: return (COLOR_BLUE);
		case 2: return (COLOR_GREEN);
		case 3: return (COLOR_CYAN);
		case 4: return (COLOR_RED);
		case 5: return (COLOR_MAGENTA);
		case 6: return (COLOR_YELLOW);
		default:
		case 7: return (COLOR_WHITE);
	}
}

int curses_io::linear(int x, int y, int xsize) const {
	return (y * xsize + x);
}

bool curses_io::prepare_colors() {

	// There are 256 colors: 16 foreground colors and 16 background ones.
	// However, the lower 8 fg are turned into the higher 8 by setting
	// A_BOLD, and the lower 8 bg into the higher 8 bg by setting A_BLINK.
	// Thus we only need 8x8 and will handle the rest behind the scenes.

	// The +1 is needed because color pairs start indexing at 1.

	for (int fg = 0; fg < 8; ++fg)
		for (int bg = 0; bg < 8; ++bg)
			init_pair(linear(fg, bg, 8) + 1,
				dos_color_to_curses((dos_color)fg),
				dos_color_to_curses((dos_color)bg));

	return (true);
}

// Note that we lose the capacity to directly determine what modifier keys
// have been pressed. See
// https://invisible-island.net/ncurses/ncurses.faq.html#modified_keys
// or substitute your own cursing at curses at this point. (In any case,
// this actually makes the terminal behave more like ZZT's: for instance,
// CTRL+arrow keys no longer register as anything.)
key_response curses_io::parse(wchar_t unparsed,
	bool special_ncurses) const {

	key_response response;

	if (!special_ncurses) {
		switch (unparsed) {
			case 25: return E_KEY_CTRL_Y;	// ??? Seems to work...
			// I don't know why this doesn't register as a special key
			// but wth.
			case 8: return E_KEY_BACKSPACE;
			default: return unparsed;
		}
	}

	switch (unparsed) {
		case KEY_UP: return E_KEY_UP;
		case KEY_DOWN: return E_KEY_DOWN;
		case KEY_RIGHT: return E_KEY_RIGHT;
		case KEY_LEFT: return E_KEY_LEFT;
		case KEY_F(1): return E_KEY_F1;
		case KEY_F(2): return E_KEY_F2;
		case KEY_F(3): return E_KEY_F3;
		case KEY_F(4): return E_KEY_F4;
		case KEY_F(5): return E_KEY_F5;
		case KEY_F(6): return E_KEY_F6;
		case KEY_F(7): return E_KEY_F7;
		case KEY_F(8): return E_KEY_F8;
		case KEY_F(9): return E_KEY_F9;
		case KEY_F(10): return E_KEY_F10;
		case KEY_F(11): return E_KEY_F11;
		case KEY_F(12): return E_KEY_F12;
		case KEY_BREAK: return E_KEY_PAUSE;
		case KEY_B2: return E_KEY_NUMPAD_CLEAR; // center of keypad
		case KEY_NPAGE: return E_KEY_PAGE_DOWN;
		case KEY_PPAGE: return E_KEY_PAGE_UP;
		case KEY_IC: return E_KEY_INSERT;
		case KEY_HOME: return E_KEY_HOME;
		case KEY_DC: return E_KEY_DELETE;
		case KEY_END: return E_KEY_END;
		case KEY_BACKSPACE: return E_KEY_BACKSPACE;

		// The ones below were found empirically (on gnome-terminal)
		case KEY_SR: return E_KEY_SHIFT_UP;
		case KEY_SF: return E_KEY_SHIFT_DOWN;
		case KEY_SLEFT: return E_KEY_SHIFT_LEFT;
		case KEY_SRIGHT: return E_KEY_SHIFT_RIGHT;

		// Makes for funny characters like in the original ZZT.
		default: return unparsed;
	}
}

void curses_io::use_color(dos_color fg, dos_color bg) const {

	attroff(A_BOLD | A_BLINK);

	int fg_high = 0, bg_high = 0;

	if (fg > 7) {
		fg_high = A_BOLD;
	}
	if (bg > 7) {
		bg_high = A_BLINK;
	}

	if (black_and_white) {
		if (fg > 7) {
			fg = White;
			bg = bg == Black ? Black : LightGray;
		} else {
			if (fg == Black) {
				bg = LightGray;
			} else {
				fg = LightGray;
				bg = Black;
			}
		}
	}

	attron(COLOR_PAIR(linear(fg % 8, bg % 8, 8) + 1) | fg_high | bg_high);

	current_fg = fg;
	current_bg = bg;

	return;
}

// FML: https://stackoverflow.com/questions/54795303
// Do you wanna have a bad time? Then switch the two first lines
// inside here and enjoy.
curses_io::curses_io() {
	setlocale(LC_ALL, "");
	window = initscr();
	keypad(window, true);

	if (window == NULL) {
		throw (NCURSES_INIT_FAILURE);
	}

	nodelay(window, true);
	noecho();
	cbreak();
	set_black_and_white(false);
	start_color();
	prepare_colors();

	// Set a quicker time out on function keys so that the delay when
	// pressing escape isn't as noticeable.
	ESCDELAY = 100;

	// sentinel for key_pressed/read_key; see these functions for details.
	last_key_detected = E_KEY_NONE;

	set_color(LightGray, Black); // set a first color
}

curses_io::~curses_io() {
	// If we have an allocated screen, clean it up.

	if (window != NULL) {
		delwin(window);
		endwin();
		refresh();
	}
}

void curses_io::show_cursor() const {
	if (curs_set(1) == ERR) {
		throw std::runtime_error("Curses: Error making cursor visible.");
	}
}

void curses_io::hide_cursor() const {
	if (curs_set(0) == ERR) {
		throw std::runtime_error("Curses: Error making cursor hidden.");
	}
}

void curses_io::set_color(dos_color fg, dos_color bg) const {
	use_color(fg, bg);
}

void curses_io::set_text_color(dos_color fg) const {
	use_color(fg, current_bg);
}

void curses_io::set_background_color(dos_color bg) const {
	use_color(current_fg, bg);
}

// TODO: Use exceptions instead.

bool curses_io::print(const std::string to_print) const {
	// TODO: Unicode stuff.
	int errval = wprintw(window, to_print.c_str());

	return (errval != ERR);
}

bool curses_io::print(int x, int y, const char * str) const {
	return (print(x, y, str, strlen(str)));
}

bool curses_io::print(int x, int y, const char * str,
	size_t maxlen) const {
	bool done = true;
	for (int counter = 0; counter < std::min(strlen(str), maxlen); ++counter) {
		done &= print_ch(x + counter, y, str[counter]);
	}

	return (done);
}

bool curses_io::print(int x, int y, std::string str) const {
	return (print(x, y, str.c_str(), str.size()));
}

bool curses_io::print_ch(int x, int y, unsigned char to_print) const {
	// This is kind of a hack, but apparently that's the right way
	// to do it!
	wchar_t arr[2] = {CP437ToCodepoint(to_print), 0};
	int errval = mvwaddnwstr(window, y, x, arr, 1);

	return (errval != ERR);
}

bool curses_io::print_ch(int x, int y, dos_color fg, dos_color bg,
	unsigned char to_print) const {

	// Set our color pair to the desired color (might want to use preset
	// color pairs later).

	// also TODO: Permit DOS colors to be passed to fg and bg.

	use_color(fg, bg);
	bool worked = print_ch(x, y, to_print);
	return (worked);
}

bool curses_io::print_ch(int x, int y, char packed_color,
	unsigned char to_print) const {

	return print_ch(x, y, (dos_color)(packed_color & 0xF),
			(dos_color)(packed_color >> 4), to_print);
}

bool curses_io::print_col(int x, int y, dos_color fg, dos_color bg,
	std::string str) const {

	use_color(fg, bg);
	return (print(x, y, str));
}

bool curses_io::print_ext(int x, int y, dos_color fg, dos_color bg,
	const std::vector<short> & ext) const {

	use_color(fg, bg);

	bool retval = true;

	for (int counter = 0; counter < ext.size(); ++counter) {
		retval &= mvwaddch(window, y, x+counter, ext[counter]);
	}

	return (retval);
}

// Keyboard input.

void curses_io::set_blocking() {
	was_blocking = true;
	nodelay(window, false);
}

void curses_io::set_nonblocking() {
	was_blocking = false;
	nodelay(window, true);
}

bool curses_io::key_pressed() {
	// The traditional way to code this function under curses is to
	// try to get a key with wget_wch, then if it succeeds, push the key
	// back with unget_wch. However, doing so when keypad is true destroys
	// the KEY_CODE_YES return value that lets us distinguish curses meta-
	// codes from literals, so that approach can't work.. Instead, if we
	// read a value here , we must store it for later and then fake the
	// response from the read_key call if there's no newer key waiting.

	// If we already have something queued up, say it's there.
	if (last_key_detected != E_KEY_NONE) {
		return true;
	}

	ignore_lack_of_keys = true;
	last_key_detected = read_key();
	ignore_lack_of_keys = false;

	return last_key_detected != E_KEY_NONE;

}

key_response curses_io::read_key() {
	key_response out;
	std::vector<key_response> keys_read;
	int key_or_err;
	bool first = true;

	// If we got something from key_pressed, return it.
	if (last_key_detected != E_KEY_NONE) {
		out = last_key_detected;
		last_key_detected = E_KEY_NONE;
		// A real keypress will turn off blocking mode, so do that here too.
		set_nonblocking();
		return out;
	}

	// Get the raw input.
	do {
		wint_t next_key;
		key_or_err = wget_wch(window, &next_key);

		if (key_or_err == ERR) {
			if (first) {
				if (was_blocking) {
					throw std::logic_error("Curses has desynchronized. "
						"Something is wrong with the code, perhaps missing"
						" initialization of TxtWind?");
				}

				if (ignore_lack_of_keys) {
					return E_KEY_NONE;
				} else {
					throw std::runtime_error(
						"Tried to read key with no key available.");
				}
			}
		} else {
			keys_read.push_back(parse(next_key,
					key_or_err == KEY_CODE_YES));
		}
		// If the function is called from a blocking key read,
		// we must turn nonblocking on for subsequent keys.
		set_nonblocking();

		first = false;
	} while (key_or_err != ERR);

	// Hack for ALT+P on ANSI terminals. Not perfect, but it should work
	// as long as the user isn't spamming the key combination.
	if (keys_read.size() >= 2 && keys_read[0] == 27 && keys_read[1] == 112) {
		return E_KEY_ALT_P;
	}

	return keys_read[0];
}

key_response curses_io::read_key_blocking() {
	// Turn off nonblocking mode and get the key. Read_key() will
	// turn nonblocking back on.
	set_blocking();
	return read_key();
}

void curses_io::flush_keybuf() {
	flushinp();
	last_key_detected = E_KEY_NONE;
}