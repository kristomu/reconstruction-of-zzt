#include "ptoc.h"

/*
	Copyright (c) 2020 Adrian Siekierka

	Based on a reconstruction of code from ZZT,
	Copyright 1991 Epic MegaGames, used with permission.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

/*$I-*/
#define __TxtWind_implementation__

#include "tools.h"
#include "txtwind.h"
#include "serialization.h"

/*#include "Crt.h"*/
/*#include "Printer.h"*/
#include "gamevars.h"
#include "fileops.h"

#include "video.h"
#include "hardware.h"

string UpCaseString(string input) {
	integer i;

	string UpCaseString_result;
	for (i = 1; i <= length(input); i ++) {
		input[i] = upcase(input[i]);
	}
	UpCaseString_result = input;
	return UpCaseString_result;
}

void TextWindowInitState(TTextWindowState & state) {
	state.LineCount = 0;
	state.LinePos = 1;
	state.LoadedFilename = "";
}

void TextWindowDrawTitle(integer color, TTextWindowLine title) {
	video.write(TextWindowX + 2, TextWindowY + 1, color,
		text_window_str_inner_empty);
	video.write(TextWindowX + ((TextWindowWidth - length(title)) / 2),
		TextWindowY + 1, color, title);
}

void TextWindowDrawOpen(TTextWindowState & state) {
	int ix, iy;

	video.Copy(TextWindowX, TextWindowY, TextWindowWidth,
		TextWindowHeight+1, false);

	for (iy = (TextWindowHeight / 2); iy >= 0; iy --) {
		video.write(TextWindowX, TextWindowY + iy + 1, 0xf,
			text_window_str_text);
		video.write(TextWindowX, TextWindowY + TextWindowHeight - iy - 1,
			0xf, text_window_str_text);
		video.write(TextWindowX, TextWindowY + iy, 0xf,
			text_window_str_top);
		video.write(TextWindowX, TextWindowY + TextWindowHeight - iy, 0xf,
			text_window_str_bottom);
		Delay(25);
		video.Refresh();
	}

	video.write(TextWindowX, TextWindowY + 2, 0xf,
		text_window_str_sep);
	TextWindowDrawTitle(0x1e, state.Title);
}

void TextWindowDrawClose(TTextWindowState & state) {
	integer ix, iy;
	integer unk1, unk2;

	for (iy = 0; iy <= (TextWindowHeight / 2); iy ++) {
		video.write(TextWindowX, TextWindowY + iy, 0xf,
			text_window_str_top);
		video.write(TextWindowX, TextWindowY + TextWindowHeight - iy, 0xf,
			text_window_str_bottom);
		Delay(18);
		/* Replace upper line with background. */
		video.Copy(TextWindowX, TextWindowY + iy, TextWindowWidth, 1,
			true);
		/* Replace lower line with background. */
		video.Copy(TextWindowX, TextWindowY + TextWindowHeight - iy,
			TextWindowWidth, 1, true);
		video.Refresh();
	}
}

void TextWindowDrawLine(TTextWindowState & state, integer lpos,
	boolean withoutFormatting, boolean viewingFile) {
	integer lineY;
	integer textOffset, textColor, textX;

	lineY = ((TextWindowY + lpos) - state.LinePos) + (TextWindowHeight / 2) +
		1;
	if (lpos == state.LinePos)
		video.write(TextWindowX + 2, lineY, 0x1c,
			text_window_str_inner_arrows);
	else
		video.write(TextWindowX + 2, lineY, 0x1e,
			text_window_str_inner_empty);
	if ((lpos > 0) && (lpos <= state.LineCount))  {
		if (withoutFormatting)  {
			video.write(TextWindowX + 4, lineY, 0x1e, *state.Lines[lpos]);
		} else {
			textOffset = 1;
			textColor = 0x1e;
			textX = TextWindowX + 4;
			if (length(*state.Lines[lpos]) > 0)  {
				switch ((*state.Lines[lpos])[1]) {
					case '!': {
						textOffset = pos(";", *state.Lines[lpos]) + 1;
						video.write(textX + 2, lineY, 0x1d, "\20");
						textX = textX + 5;
						textColor = 0x1f;
					}
					break;
					case ':': {
						textOffset = pos(";", *state.Lines[lpos]) + 1;
						textColor = 0x1f;
					}
					break;
					case '$': {
						textOffset = 2;
						textColor = 0x1f;
						textX = (textX - 4) + ((TextWindowWidth - length(*state.Lines[lpos])) / 2);
					}
					break;
				}
			}
			if (textOffset > 0)  {
				video.write(textX, lineY, textColor,
					copy(*state.Lines[lpos], textOffset,
						length(*state.Lines[lpos]) - textOffset + 1));
			}
		}
	} else if ((lpos == 0) || (lpos == (state.LineCount + 1)))  {
		video.write(TextWindowX + 2, lineY, 0x1e,
			text_window_str_inner_sep);
	} else if ((lpos == -4) && viewingFile)  {
		video.write(TextWindowX + 2, lineY, 0x1a,
			"   Use            to view text,");
		video.write(TextWindowX + 2 + 7, lineY, 0x1f, "\30 \31, Enter");
	} else if ((lpos == -3) && viewingFile)  {
		video.write(TextWindowX + 2 + 1, lineY, 0x1a,
			"                 to print.");
		video.write(TextWindowX + 2 + 12, lineY, 0x1f, "Alt-P");
	}
}

void TextWindowDraw(TTextWindowState & state, boolean withoutFormatting,
	boolean viewingFile) {
	integer i;
	integer unk1;

	for (i = 0; i <= (TextWindowHeight - 4); i ++)
		TextWindowDrawLine(state, state.LinePos - (TextWindowHeight / 2) + i + 2,
			withoutFormatting, viewingFile);
	TextWindowDrawTitle(0x1e, state.Title);
}

void TextWindowAppend(TTextWindowState & state, TTextWindowLine line) {
	state.LineCount = state.LineCount + 1;
	state.Lines[state.LineCount] = new TTextWindowLine;
	*state.Lines[state.LineCount] = line;
}

void TextWindowFree(TTextWindowState & state) {
	while (state.LineCount > 0)  {
		delete state.Lines[state.LineCount];
		state.LineCount = state.LineCount - 1;
	}
	state.LoadedFilename = "";
}

void TextWindowPrint(TTextWindowState & state) {
	integer iLine, iChar;
	string line;

	std::ofstream printout = OpenForWrite("PRINTOUT.DAT");
	if (errno != 0) {
		return;
	}

	for (iLine = 1; iLine <= state.LineCount; iLine ++) {
		line = *state.Lines[iLine];
		if (length(line) > 0)  {
			switch (line[1]) {
				case '$': {
					Delete(line, 1, 1);
					for (iChar = ((80 - length(line)) / 2); iChar >= 1; iChar --) {
						line = string(' ') + line;
					}
				}
				break;
				case '!': case ':': {
					iChar = pos(";", line);
					if (iChar > 0) {
						Delete(line, 1, iChar);
					} else {
						line = "";
					}
				}
				break;
				default:
					line = string("          ") + line;
			}
		}
		printout << line << "\n";
		if (errno != 0)  {
			return;
		}
	}
	if (state.LoadedFilename == "ORDER.HLP")  {
		printout << order_print_id << "\n";
	}
	printout << chr(12); /* form feed */
}

void TextWindowSelect(TTextWindowState & state, boolean hyperlinkAsSelect,
	boolean viewingFile) {
	integer newLinePos;
	integer unk1;
	integer iLine, iChar;
	varying_string<20> pointerStr;

	TextWindowRejected = false;
	state.Hyperlink = "";
	TextWindowDraw(state, false, viewingFile);
	do {
		InputReadWaitKey();
		newLinePos = state.LinePos;
		if (InputDeltaY != 0)  {
			newLinePos = newLinePos + InputDeltaY;
		} else if (InputShiftPressed || (InputKeyPressed == E_KEY_ENTER))  {
			InputShiftAccepted = true;
			/* IMP: Fix potential out-of-bounds access. */
			if ((length(*state.Lines[state.LinePos]) > 0)
				&& (((*state.Lines[state.LinePos])[1]) == '!'))  {
				pointerStr = copy(*state.Lines[state.LinePos], 2,
						length(*state.Lines[state.LinePos]) - 1);

				if (pos(";", pointerStr) > 0)  {
					pointerStr = copy(pointerStr, 1, pos(";", pointerStr) - 1);
				}

				if (pointerStr[1] == '-')  {
					Delete(pointerStr, 1, 1);
					TextWindowFree(state);
					TextWindowOpenFile(pointerStr.body, state);
					if (state.LineCount == 0) {
						return;
					} else {
						viewingFile = true;
						newLinePos = state.LinePos;
						TextWindowDraw(state, false, viewingFile);
						InputKeyPressed = '\0';
						InputShiftPressed = false;
					}
				} else {
					if (hyperlinkAsSelect)  {
						state.Hyperlink = pointerStr;
					} else {
						pointerStr = string(':') + pointerStr;
						for (iLine = 1; iLine <= state.LineCount; iLine ++) {
							if (length(pointerStr) > length(*state.Lines[iLine]))  {
								;
							} else {
								for (iChar = 1; iChar <= length(pointerStr); iChar ++) {
									if (upcase(pointerStr[iChar]) != upcase((*state.Lines[iLine])[iChar])) {
										goto LLabelNotMatched;
									}
								}
								newLinePos = iLine;
								InputKeyPressed = '\0';
								InputShiftPressed = false;
								goto LLabelMatched;
LLabelNotMatched:;
							}
						}
					}
				}
			}
		} else {
			if (InputKeyPressed == E_KEY_PAGE_UP) {
				newLinePos = state.LinePos - TextWindowHeight + 4;
			} else if (InputKeyPressed == E_KEY_PAGE_DOWN) {
				newLinePos = state.LinePos + TextWindowHeight - 4;
			} else if (InputKeyPressed == E_KEY_ALT_P) {
				TextWindowPrint(state);
			}
		}

LLabelMatched:
		if (newLinePos < 1) {
			newLinePos = 1;
		} else if (newLinePos > state.LineCount) {
			newLinePos = state.LineCount;
		}

		if (newLinePos != state.LinePos)  {
			state.LinePos = newLinePos;
			TextWindowDraw(state, false, viewingFile);
			/* IMP: Fix potential out-of-bounds access.*/
			if ((length(*state.Lines[state.LinePos]) > 0)
				&& (((*state.Lines[state.LinePos])[1]) == '!')) {
				if (hyperlinkAsSelect) {
					TextWindowDrawTitle(0x1e, "\256Press ENTER to select this\257");
				} else {
					TextWindowDrawTitle(0x1e, "\256Press ENTER for more info\257");
				}
			}
		}
		if (InputJoystickMoved)  {
			Delay(35);
		}
	} while (!((InputKeyPressed == E_KEY_ESCAPE)
			|| (InputKeyPressed == E_KEY_ENTER) || InputShiftPressed));
	if (InputKeyPressed == E_KEY_ESCAPE)  {
		InputKeyPressed = '\0';
		TextWindowRejected = true;
	}
}

void TextWindowSort(TTextWindowState & state);


static void Swap(TTextWindowLine* & a, TTextWindowLine* & b) {
	TTextWindowLine* c;

	c = a;
	a = b;
	b = c;
}



/* Returns the location of the pivot after separating. */
static integer Partition(TTextWindowState & state, integer low,
	integer high) {
	TTextWindowLine* pivot;
	integer i, j;

	/* Choose a random pivot. A cannae be bothered to do anything
	  faster/more sophisticated. */
	integer Partition_result;
	pivot = state.Lines[Random(high-low) + low];
	i = low;
	j = high;

	do {
		while (*state.Lines[i] < *pivot) {
			i += 1;
		}
		while (*state.Lines[j] > *pivot) {
			j -= 1;
		}

		if (i <= j)  {
			Swap(state.Lines[i], state.Lines[j]);

			i += 1;
			j -= 1;
		}
	} while (!(i > j));

	Partition_result = i;
	return Partition_result;
}


static void Quicksort(TTextWindowState & state, integer low,
	integer high) {
	integer pivotPt;

	if (low < high)  {
		pivotPt = Partition(state, low, high);
		Quicksort(state, low, pivotPt-1);
		Quicksort(state, pivotPt, high);
	}
}

void TextWindowSort(TTextWindowState & state) {
	integer i, j;
	integer smallestIdx;


	Quicksort(state, 1, state.LineCount);
}

void TextWindowEdit(TTextWindowState & state);

static integer newLinePos;

static void DeleteCurrLine(TTextWindowState & state) {
	integer i;

	if (state.LineCount > 1)  {
		delete state.Lines[state.LinePos];
		for (i = (state.LinePos + 1); i <= state.LineCount; i ++) {
			state.Lines[i - 1] = state.Lines[i];
		}

		state.LineCount = state.LineCount - 1;
		if (state.LinePos > state.LineCount) {
			newLinePos = state.LineCount;
		} else {
			TextWindowDraw(state, true, false);
		}
	} else {
		*state.Lines[1] = "";
	}
}

void TextWindowEdit(TTextWindowState & state) {
	boolean insertMode;
	integer charPos;
	integer i;

	if (state.LineCount == 0) {
		TextWindowAppend(state, "");
	}
	insertMode = true;
	state.LinePos = 1;
	charPos = 1;
	TextWindowDraw(state, true, false);
	do {
		if (insertMode) {
			video.write(77, 14, 0x1e, "on ");
		} else {
			video.write(77, 14, 0x1e, "off");
		}

		if (charPos >= (length(*state.Lines[state.LinePos]) + 1))  {
			charPos = length(*state.Lines[state.LinePos]) + 1;
			video.write(charPos + TextWindowX + 3,
				TextWindowY + (TextWindowHeight / 2) + 1,
				0x70, " ");
		} else {
			video.write(charPos + TextWindowX + 3,
				TextWindowY + (TextWindowHeight / 2) + 1,
				0x70, (*state.Lines[state.LinePos])[charPos]);
		}
		InputReadWaitKey();
		newLinePos = state.LinePos;
		switch (InputKeyPressed) {
			case E_KEY_UP: newLinePos = state.LinePos - 1; break;
			case E_KEY_DOWN: newLinePos = state.LinePos + 1; break;
			case E_KEY_PAGE_UP: newLinePos = state.LinePos - TextWindowHeight + 4;
				break;
			case E_KEY_PAGE_DOWN: newLinePos = state.LinePos + TextWindowHeight - 4;
				break;
			/*IMP: END and HOME*/
			case E_KEY_END: charPos = length(*state.Lines[state.LinePos]) + 1; break;
			case E_KEY_HOME: charPos = 1; break;
			case E_KEY_RIGHT: {
				charPos = charPos + 1;
				if (charPos > (length(*state.Lines[state.LinePos]) + 1))  {
					charPos = 1;
					newLinePos = state.LinePos + 1;
				}
			}
			break;
			case E_KEY_LEFT: {
				charPos = charPos - 1;
				if (charPos < 1)  {
					charPos = TextWindowWidth;
					newLinePos = state.LinePos - 1;
				}
			}
			break;
			case E_KEY_ENTER: {
				if (state.LineCount < MAX_TEXT_WINDOW_LINES)  {
					for (i = state.LineCount; i >= (state.LinePos + 1); i --) {
						state.Lines[i + 1] = state.Lines[i];
					}

					state.Lines[state.LinePos + 1] = new TTextWindowLine;
					*state.Lines[state.LinePos + 1]
						= copy(*state.Lines[state.LinePos], charPos,
								length(*state.Lines[state.LinePos]) - charPos + 1);
					*state.Lines[state.LinePos]
						= copy(*state.Lines[state.LinePos], 1, charPos - 1);

					newLinePos = state.LinePos + 1;
					charPos = 1;
					state.LineCount = state.LineCount + 1;
				}
			}
			break;
			case E_KEY_BACKSPACE: {
				if (charPos > 1)  {
					*state.Lines[state.LinePos] =
						copy(*state.Lines[state.LinePos], 1, charPos - 2)
						+ copy(*state.Lines[state.LinePos], charPos,
							length(*state.Lines[state.LinePos]) - charPos + 1);
					charPos = charPos - 1;
				} else if (length(*state.Lines[state.LinePos]) == 0)  {
					DeleteCurrLine(state);
					newLinePos = state.LinePos - 1;
					charPos = TextWindowWidth;
				}
			}
			break;
			case E_KEY_INSERT: {
				insertMode = ! insertMode;
			}
			break;
			case E_KEY_DELETE: {
				*state.Lines[state.LinePos] =
					copy(*state.Lines[state.LinePos], 1, charPos - 1)
					+ copy(*state.Lines[state.LinePos], charPos + 1,
						length(*state.Lines[state.LinePos]) - charPos);
			}
			break;
			case E_KEY_CTRL_Y: {
				DeleteCurrLine(state);
			}
			break;
			default:
				if ((InputKeyPressed >= '\40') && (charPos < (TextWindowWidth - 7)))  {
					if (! insertMode)  {
						*state.Lines[state.LinePos] = copy(*state.Lines[state.LinePos], 1,
								charPos - 1)
							+ (char)(InputKeyPressed)
							+ copy(*state.Lines[state.LinePos], charPos + 1,
								length(*state.Lines[state.LinePos]) - charPos);
						charPos = charPos + 1;
					} else {
						if (length(*state.Lines[state.LinePos]) < (TextWindowWidth - 8))  {
							*state.Lines[state.LinePos] = copy(*state.Lines[state.LinePos], 1,
									charPos - 1)
								+ (char)InputKeyPressed
								+ copy(*state.Lines[state.LinePos], charPos,
									length(*state.Lines[state.LinePos]) - charPos + 1);
							charPos = charPos + 1;
						}
					}
				}
		}

		if (newLinePos < 1) {
			newLinePos = 1;
		} else if (newLinePos > state.LineCount) {
			newLinePos = state.LineCount;
		}

		if (newLinePos != state.LinePos)  {
			state.LinePos = newLinePos;
			TextWindowDraw(state, true, false);
		} else {
			TextWindowDrawLine(state, state.LinePos, true, false);
		}
	} while (!(InputKeyPressed == E_KEY_ESCAPE));

	if (length(*state.Lines[state.LineCount]) == 0)  {
		delete state.Lines[state.LineCount];
		state.LineCount = state.LineCount - 1;
	}
}

void TextWindowOpenFile(std::string filename,
	TTextWindowState & state) {
	std::ifstream f, tf;
	size_t i;
	integer entryPos;
	boolean retVal;
	TTextWindowLine* line;
	byte lineLen;

	retVal = true;
	for (char c: filename) {
		retVal = retVal && (c != '.');
	}

	if (retVal) {
		filename = filename + ".HLP";
	}

	if (filename[0] == '*')  {
		filename = filename.substr(1);
		entryPos = -1;
	} else {
		entryPos = 0;
	}

	TextWindowInitState(state);
	state.LoadedFilename = UpCaseString(filename.c_str());

	// If the resource file hasn't yet been opened... IMP: try and try
	// again if the file wasn't openable.
	if (ResourceDataHeader.EntryCount <= 0)  {
		// First there's a file count of 2 bytes. Then there are
		// MAX_RESOURCE_DATA_FILES records of 50-long pascal strings; and
		// then MAX_RESOURCE_DATA_FILES 32-bit ints giving their position.
		// That is a total of 2 + MAX_RESOURCE_DATA_FILES * (50+1+4).
		// Fix potential off-by-one later.
		f = OpenForRead(ResourceDataFileName);
		if (errno == 0) {
			size_t bufsize = 2 + MAX_RESOURCE_DATA_FILES * 55;
			char buffer[bufsize];
			// TODO: Deal with packing problems that will probably ensue.
			// (And ensue, they did.)
			//f.read((char*)&ResourceDataHeader, sizeof(ResourceDataHeader));
			f.read(buffer, bufsize);

			if (errno != 0) {
				ResourceDataHeader.EntryCount = -1;
				return;
			}

			char * buf_ptr = buffer;
			buf_ptr = load_lsb_element(buf_ptr, ResourceDataHeader.EntryCount);

			bool truncated = false;

			// Names
			for (i = 0; i < ResourceDataHeader.EntryCount && !truncated; ++i) {
				buf_ptr = get_pascal_string(buf_ptr, buffer + bufsize,
						50, true, ResourceDataHeader.Name[i], truncated);
			}
			// Skip the junk at the end.
			buf_ptr += (MAX_RESOURCE_DATA_FILES -
					ResourceDataHeader.EntryCount) * 51;

			if (truncated) {
				ResourceDataHeader.EntryCount = -1;
				return;
			}

			for (i = 0; i < ResourceDataHeader.EntryCount; ++i) {
				buf_ptr = load_lsb_element(buf_ptr,
						ResourceDataHeader.FileOffset[i]);
			}
			f.close();

		} else {
			ResourceDataHeader.EntryCount = -1;
			return;
		}
	}

	if (entryPos == 0)  {
		for (i = 0; i < ResourceDataHeader.EntryCount; ++i) {
			if (str_toupper(ResourceDataHeader.Name[i]) == str_toupper(filename)) {
				entryPos = i;
			}
		}
	}

	// User-specified file
	if (entryPos < 0)  {
		tf = OpenForRead(filename);
		if (errno == 0)  {
			while ((errno == 0) && (! tf.eof()))  {
				state.LineCount += 1;
				state.Lines[state.LineCount] = new TTextWindowLine;
				std::string in_between;
				tf >> in_between;
				*state.Lines[state.LineCount] = in_between.c_str();
			}
			tf.close();
		}
		// Internal file
	} else {
		f = OpenForRead(ResourceDataFileName);

		size_t length, pos = ResourceDataHeader.FileOffset[entryPos];
		if (entryPos+1 == ResourceDataHeader.EntryCount) {
			f.seekg(0, std::ios_base::end);
			length = (size_t)f.tellg() - ResourceDataHeader.FileOffset[entryPos];
		} else {
			length = ResourceDataHeader.FileOffset[entryPos+1] -
				ResourceDataHeader.FileOffset[entryPos];
		}

		// Read off the virtual file into a buffer.
		char virtual_file[length];
		f.seekg(pos);
		f.read(virtual_file, length);
		char * vbuf_ptr = virtual_file;
		bool truncated;

		if (errno == 0)  {
			bool virtual_file_done = false;
			while (errno == 0 && !virtual_file_done &&
				vbuf_ptr < virtual_file + length)  {

				// TODO: Handle the memory leak that'll (probably) happen here.
				// This will require some more serious restructuring.
				state.LineCount += 1;
				state.Lines[state.LineCount] = new TTextWindowLine;

				std::string this_line;
				vbuf_ptr = get_pascal_string(vbuf_ptr,
						virtual_file + length, 50, false, this_line, truncated);

				line = state.Lines[state.LineCount] + 1;
				lineLen = this_line.size();
				if (lineLen == 0)  {
					*state.Lines[state.LineCount] = "";
				} else {
					*state.Lines[state.LineCount] = this_line.c_str();
				}

				// A lone @ Pascal string marks the end of the virtual file.
				if (this_line == "@")  {
					virtual_file_done = true;
					*state.Lines[state.LineCount] = "";
				}
			}
			f.close();
		}
	}
}

void TextWindowSaveFile(TTextWindowLine filename,
	TTextWindowState & state) {
	integer i;

	std::ofstream f = OpenForWrite(filename.body);
	if (errno != 0) {
		return;
	}

	for (i = 1; i <= state.LineCount; i ++) {
		f << *state.Lines[i] << "\n";
		if (errno != 0) {
			return;
		}
	}

	f.close();
}

void TextWindowDisplayFile(std::string filename, string title) {
	TTextWindowState state;

	state.Title = title;
	TextWindowOpenFile(filename, state);
	state.Selectable = false;
	if (state.LineCount > 0)  {
		TextWindowDrawOpen(state);
		TextWindowSelect(state, false, true);
		TextWindowDrawClose(state);
	}
	TextWindowFree(state);
}

void TextWindowInit(integer x, integer y, integer width, integer height) {
	integer i;

	TextWindowX = x;
	TextWindowWidth = width;
	TextWindowY = y;
	TextWindowHeight = height;
	text_window_str_inner_empty = std::string(TextWindowWidth-5, ' ');
	text_window_str_inner_line = std::string(TextWindowWidth-5, '\315');
	text_window_str_top    = "\306\321" + text_window_str_inner_line +
		"\321\265";
	text_window_str_bottom = "\306\317" + text_window_str_inner_line +
		"\317\265";
	text_window_str_sep    = " \306" + text_window_str_inner_line + "\265 ";
	text_window_str_text   = " \263" + text_window_str_inner_empty + "\263 ";
	text_window_str_inner_arrows = text_window_str_inner_empty;
	text_window_str_inner_arrows[0] = '\257';
	*text_window_str_inner_arrows.rbegin() = '\256';
	text_window_str_inner_sep = text_window_str_inner_empty;
	for (i = 1; i < (TextWindowWidth / 5); i ++) {
		text_window_str_inner_sep[i * 5 + ((TextWindowWidth % 5) / 2) - 1] = '\7';
	}
}

class unit_TxtWind_initialize {
	public: unit_TxtWind_initialize();
};
static unit_TxtWind_initialize TxtWind_constructor;

unit_TxtWind_initialize::unit_TxtWind_initialize() {
	ResourceDataFileName = "";
	ResourceDataHeader.EntryCount = 0;
}
