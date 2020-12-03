#include "curses.h"

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
	for (int counter = 0x20; counter <= 0x7E; ++counter)
		uchars.push_back("");

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
	if (dos_char < uchars.size() && uchars[dos_char].size() != 0)
		return(uchars[dos_char]);

	std::string defstr = "Q";
	defstr[0] = dos_char;
	return(defstr);
}

short curses::dos_color_to_curses(int color) const {
	switch(color) {
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

int curses::linear(int x, int y, int xsize) const {
	return(y * xsize + x);
}

bool curses::prepare_colors() {

	// There are 256 colors: 16 foreground colors and 16 background ones.
	// However, the lower 8 fg are turned into the higher 8 by setting
	// A_BOLD, and the lower 8 bg into the higher 8 bg by setting A_BLINK.
	// Thus we only need 8x8 and will handle the rest behind the scenes.

	// The +1 is needed because color pairs start indexing at 1.

	for (int fg = 0; fg < 8; ++fg)
		for (int bg = 0; bg < 8; ++bg)
			init_pair(linear(fg, bg, 8) + 1,
					dos_color_to_curses(fg),
					dos_color_to_curses(bg));

	return(true);
}

void curses::use_color(int fg, int bg) const {

	attroff(A_BOLD | A_BLINK);

	int fg_high = 0, bg_high = 0;

	if (fg > 7)
		fg_high = A_BOLD;
	if (bg > 7)
		bg_high = A_BLINK;

	attron(COLOR_PAIR(linear(fg % 8, bg % 8, 8) + 1) | fg_high | bg_high);

	current_fg = fg;
	current_bg = bg;

	return;
}

curses::curses() {
	window = initscr();

	if (window == NULL) {
		throw(NCURSES_INIT_FAILURE);
	}

	start_color();
	prepare_colors();
}

curses::~curses() {
	// If we have an allocated screen, clean it up.

	if (window != NULL) {
		delwin(window);
		endwin();
		redraw();
	}
}

void curses::set_dos_color(int fg, int bg) const {
	use_color(fg, bg);
}

void curses::set_dos_text_color(int fg) const {
	use_color(fg, current_bg);
}

void curses::set_dos_background_color(int bg) const {
	use_color(current_fg, bg);
}

// TODO: Use exceptions instead.

bool curses::print(const std::string to_print) const {
	// TODO: Unicode stuff.
	int errval = wprintw(window, to_print.c_str());

	return (errval != ERR);
}

bool curses::print(int x, int y, const char * str) const {
	return(print(x, y, str, strlen(str)));
}

bool curses::print(int x, int y, const char * str, size_t maxlen) const {
	bool done = true;
	for (int counter = 0; counter < std::min(strlen(str), maxlen); ++counter)
		done &= print_ch(x + counter, y, str[counter]);

	return(done);
}

bool curses::print(int x, int y, std::string str) const {
	return(print(x, y, str.c_str(), str.size()));
}

bool curses::print_ch(int x, int y, char to_print) const {
	int errval = mvwprintw(window, y, x,
			interpreter.unicode(to_print).c_str());

	return (errval != ERR);
}

bool curses::print_ch(int x, int y, int fg, int bg,
		char to_print) const {

	// Set our color pair to the desired color (might want to use preset
	// color pairs later).

	// also TODO: Permit DOS colors to be passed to fg and bg.

	use_color(fg, bg);
	bool worked = print_ch(x, y, to_print);
	return(worked);
}

bool curses::print_col(int x, int y, int fg, int bg, std::string str) const {

	use_color(fg, bg);
	return (print(x, y, str));
}

bool curses::print_ext(int x, int y, int fg, int bg,
		const std::vector<short> & ext) const {

	use_color(fg, bg);

	bool retval = true;

	for (int counter = 0; counter < ext.size(); ++counter)
		 retval &= mvwaddch(window, y, x+counter, ext[counter]);

	return(retval);
}