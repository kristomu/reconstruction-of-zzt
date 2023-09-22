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
/*$V-*/

#include "ptoc.h"
#include "serialization.h"
#include "imemstream.h"

#include <dirent.h>
#include <iterator>

#define __Game_implementation__

#include "game.h"
#include "tools.h"

#include "world.h"
#include "video.h"
#include "sounds.h"
#include "elements.h"
#include "editor.h"
#include "oop.h"
#include "fileops.h"
#include "minmax.h"

#include "hardware.h"
#include "testing.h"

#include <iostream>

std::vector<char> preloaded_world_data;

boolean ValidCoord(integer x, integer y) {
	if ((x < 0) || (y < 0))  {
		return false;
	}
	if ((x > BOARD_WIDTH+1) || (y > BOARD_HEIGHT+1))  {
		return false;
	}
	return true;
}

boolean CoordInsideViewport(integer x, integer y) {
	boolean CoordInsideViewport_result;
	if ((x <= 0) || (y <= 0))  {
		CoordInsideViewport_result = false;
		return CoordInsideViewport_result;
	}
	if ((x > BOARD_WIDTH) || (y > BOARD_HEIGHT))  {
		CoordInsideViewport_result = false;
		return CoordInsideViewport_result;
	}
	CoordInsideViewport_result = true;
	return CoordInsideViewport_result;
}

void SidebarClearLine(integer y) {
	video.write(60, y, 0x11, "\263                   ");
}

void SidebarClear() {
	integer i;

	for (i = 3; i <= 24; i ++) {
		SidebarClearLine(i);
	}
}

void InitWorld() {
	// WARNING: this might create two different ElementInfo
	// structures bound to different pointers. It's an ugly hack
	// and really should be fixed by making Game an class that
	// contains its own world.

	elem_info_ptr = std::make_shared<ElementInfo>();
	game_world = std::make_shared<TWorld>(elem_info_ptr);
}

void GenerateTransitionTable() {
	integer ix, iy;
	TCoord t;

	TransitionTableSize = 0;
	for (iy = 1; iy <= BOARD_HEIGHT; iy ++)
		for (ix = 1; ix <= BOARD_WIDTH; ix ++) {
			TransitionTableSize = TransitionTableSize + 1;
			TransitionTable[TransitionTableSize].X = ix;
			TransitionTable[TransitionTableSize].Y = iy;
		}

	/* shuffle */
	for (ix = 1; ix <= TransitionTableSize; ix ++) {
		iy = rnd.randint(TransitionTableSize) + 1;
		t = TransitionTable[iy];
		TransitionTable[iy] = TransitionTable[ix];
		TransitionTable[ix] = t;
	}
}

void BoardClose(boolean showTruncationNote) {
	std::string out_load_error;
	game_world->BoardData[game_world->Info.CurrentBoardIdx] =
		game_world->currentBoard.dump_and_truncate(
			out_load_error);

	if (showTruncationNote && out_load_error != "") {
		DisplayTruncationNote();
	}
}

/* Set worldIsDamaged to true if the BoardOpen is from a world load and
the world metadata is wrong; this will make the corruption notification
show up regardless of whether the board itself is damaged. */
void BoardOpen(integer boardId, boolean worldIsDamaged) {

	byte * ptr;
	integer i, ix, iy;
	TRleTile rle;
	integer bytesRead;
	boolean boardIsDamaged;

	if (boardId > game_world->BoardCount) {
		boardId = game_world->Info.CurrentBoardIdx;
	}

	std::string load_error = game_world->currentBoard.load(
			game_world->BoardData[boardId],
			boardId, game_world->BoardCount);

	game_world->Info.CurrentBoardIdx = boardId;

	// TODO: Distinguish between truncation and corruption as in
	// FPC. Also be more sensible about what kind of corruption
	// notices to show -- probably just one at the very end.
	if (load_error != "" || worldIsDamaged) {
		DisplayCorruptionNote(load_error);
	}

}

void BoardChange(integer boardId) {
	game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X]
	[game_world->currentBoard.Stats[0].Y].Element = E_PLAYER;
	game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X]
	[game_world->currentBoard.Stats[0].Y].Color =
		elem_info_ptr->defs[E_PLAYER].Color;
	if (boardId != game_world->Info.CurrentBoardIdx)  {
		BoardClose(true);
		BoardOpen(boardId, false);
	}
}

void BoardCreate() {
	// Retain the title screen label if we're overwriting a title screen.
	// Required to perform the same way as Pascal on TRUNCATE.ZZT.
	if (game_world->currentBoard.Name == "Title screen") {
		game_world->currentBoard.create();
		game_world->currentBoard.Name = "Title screen";
	} else {
		game_world->currentBoard.create();
		game_world->currentBoard.Name = "";
	}
}

void WorldCreate() {
	integer i;

	InitElementsGame();
	game_world->BoardCount = 0;
	game_world->BoardData[0] = std::vector<unsigned char>();
	InitEditorStatSettings();
	ResetMessageNotShownFlags();
	BoardCreate();
	game_world->Info.clear();
	BoardChange(0);
	game_world->currentBoard.Name = "Title screen";
	LoadedGameFileName = "";
	game_world->Info.Name = "";
}

void TransitionDrawToFill(char chr_, integer color) {
	integer i;

	for (i = 1; i <= TransitionTableSize; ++i) {
		video.write(TransitionTable[i].X - 1, TransitionTable[i].Y - 1,
			color, chr_);
	}
}

void BoardDrawTile(integer x, integer y) {
	byte ch;

	if (!CoordInsideViewport(x, y)) {
		return;
	}

	TTile & with = game_world->currentBoard.Tiles[x][y];
	if (! game_world->currentBoard.Info.IsDark
		|| (elem_info_ptr->defs[game_world->currentBoard.Tiles[x][y].Element].VisibleInDark)
		|| (
			(game_world->Info.TorchTicks > 0)
			&& ((sqr(game_world->currentBoard.Stats[0].X - x) + sqr(
						game_world->currentBoard.Stats[0].Y - y) * 2) <
				TORCH_DIST_SQR)
		) || ForceDarknessOff) {
		if (with.Element == E_EMPTY) {
			video.write(x - 1, y - 1, 0xf, " ");
		} else if ((with.Element < E_TEXT_MIN)
			&& ElementProcDefs[with.Element].HasDrawProc)  {
			ElementProcDefs[with.Element].DrawProc(x, y, ch);
			video.write(x - 1, y - 1, with.Color, chr(ch));
		} else if (with.Element < E_TEXT_MIN)
			video.write(x - 1, y - 1, with.Color,
				elem_info_ptr->defs[with.Element].Character);
		else {
			/* Text drawing */
			if (with.Element == E_TEXT_WHITE) {
				video.write(x - 1, y - 1, 0xf,
					chr(game_world->currentBoard.Tiles[x][y].Color));
			} else if (video.is_monochrome())
				video.write(x - 1, y - 1,
					((with.Element - E_TEXT_MIN) + 1) * 16,
					chr(game_world->currentBoard.Tiles[x][y].Color));
			else
				video.write(x - 1, y - 1,
					(((with.Element - E_TEXT_MIN) + 1) * 16) + 0xf,
					chr(game_world->currentBoard.Tiles[x][y].Color));
		}
	} else {
		/* Darkness */
		video.write(x - 1, y - 1, 0x7, "\260");
	}
}

void BoardDrawBorder() {
	integer ix, iy;

	for (ix = 1; ix <= BOARD_WIDTH; ix ++) {
		BoardDrawTile(ix, 1);
		BoardDrawTile(ix, BOARD_HEIGHT);
	}

	for (iy = 1; iy <= BOARD_HEIGHT; iy ++) {
		BoardDrawTile(1, iy);
		BoardDrawTile(BOARD_WIDTH, iy);
	}
}

void TransitionDrawToBoard() {
	integer i;

	BoardDrawBorder();

	for (i = 1; i <= TransitionTableSize; i ++) {
		TCoord & with = TransitionTable[i];
		BoardDrawTile(with.X, with.Y);
	}
}

void SidebarPromptCharacter(boolean editable, integer x, integer y,
	TString50 prompt, byte & value) {
	integer i, newValue;

	SidebarClearLine(y);
	video.write(x, y, (integer)(editable) + 0x1e, prompt);
	SidebarClearLine(y + 1);
	video.write(x + 5, y + 1, 0x9f, "\37");
	SidebarClearLine(y + 2);

	do {
		for (i = (value - 4); i <= (value + 4); i ++)
			video.write(((x + i) - value) + 5, y + 2, 0x1e,
				chr((i + 0x100) % 0x100));

		if (editable)  {
			// All of these can be replaced with a blocking read later.
			// It'll only make things more responsive.
			Delay(10);
			keyboard.wait_for_key();
			if (keyboard.InputKeyPressed == E_KEY_TAB) {
				keyboard.InputDeltaX = 9;
			}

			newValue = value + keyboard.InputDeltaX;
			if (value != newValue)  {
				value = (newValue + 0x100) % 0x100;
				SidebarClearLine(y + 2);
			}
		}
	} while (!((keyboard.InputKeyPressed == E_KEY_ENTER)
			|| (keyboard.InputKeyPressed == E_KEY_ESCAPE) || ! editable
			|| keyboard.InputShiftPressed));

	video.write(x + 5, y + 1, 0x1f, "\37");
}

void SidebarPromptSlider(boolean editable, integer x, integer y,
	string prompt, byte & value, integer maximum) {
	integer newValue;
	boolean newValInBounds, oldValInBounds;
	char startChar, endChar;
	string S;

	if (prompt[length(prompt) - 2] == ';')  {
		startChar = prompt[length(prompt) - 1];
		endChar = prompt[length(prompt)];
		prompt = copy(prompt, 1, length(prompt) - 3);
	} else {
		startChar = '1';
		endChar = '9';
	}

	SidebarClearLine(y);
	video.write(x, y, (integer)(editable) + 0x1e, prompt);
	SidebarClearLine(y + 1);
	SidebarClearLine(y + 2);
	video.write(x, y + 2, 0x1e, string(startChar) + "....:...." + endChar);

	do {
		if (editable)  {
			if (value > 8)  {
				str(value, S);
				video.write(x, y + 2, 0x1e,
					string(startChar) + "  (" + S + ")  " + endChar);
			} else {
				video.write(x, y + 2, 0x1e, string(startChar) + "....:...." + endChar);
				video.write(x + value + 1, y + 1, 0x9f, "\37");
			}

			Delay(10);

			keyboard.update();
			if ((keyboard.InputKeyPressed >= '1')
				&& (keyboard.InputKeyPressed <= '9'))  {
				value = ord(keyboard.InputKeyPressed) - 49;
				SidebarClearLine(y + 1);
			} else {
				newValue = value + keyboard.InputDeltaX;
				newValInBounds = (newValue >= 0) && (newValue <= 8);
				oldValInBounds = (value >= 0) && (value <= 8);
				if ((value != newValue) && (newValue <= maximum) && ((! oldValInBounds)
						|| (newValInBounds && oldValInBounds)))  {
					value = newValue % 256;
					SidebarClearLine(y + 1);
				}
			}
		}
	} while (!((keyboard.InputKeyPressed == E_KEY_ENTER)
			|| (keyboard.InputKeyPressed == E_KEY_ESCAPE) || ! editable
			|| keyboard.InputShiftPressed));

	if (value <= 8) {
		video.write(x + value + 1, y + 1, 0x1f, "\37");
	}
}

void SidebarPromptChoice(boolean editable, integer y, string prompt,
	string choiceStr, byte & result) {
	integer i, j, choiceCount;
	integer newResult;

	SidebarClearLine(y);
	SidebarClearLine(y + 1);
	SidebarClearLine(y + 2);
	video.write(63, y, (integer)(editable) + 0x1e, prompt);
	video.write(63, y + 2, 0x1e, choiceStr);

	choiceCount = 1;
	for (i = 1; i <= length(choiceStr); i ++)
		if (choiceStr[i] == ' ') {
			choiceCount = choiceCount + 1;
		}

	do {
		j = 0;
		i = 1;
		while ((j < result) && (i < length(choiceStr)))  {
			if (choiceStr[i] == ' ') {
				j = j + 1;
			}
			i = i + 1;
		}

		if (editable)  {
			video.write(62 + i, y + 1, 0x9f, "\37");
			Delay(35);
			keyboard.update();

			newResult = result + keyboard.InputDeltaX;
			if ((result != newResult) && (newResult >= 0)
				&& (newResult <= (choiceCount - 1)))  {
				result = newResult;
				SidebarClearLine(y + 1);
			}
		}
	} while (!((keyboard.InputKeyPressed == E_KEY_ENTER)
			|| (keyboard.InputKeyPressed == E_KEY_ESCAPE) || ! editable
			|| keyboard.InputShiftPressed));

	video.write(62 + i, y + 1, 0x1f, "\37");
}

void SidebarPromptDirection(boolean editable, integer y,
	string prompt,
	integer & deltaX, integer & deltaY) {
	byte choice;

	if (deltaY == -1) {
		choice = 0;
	} else if (deltaY == 1) {
		choice = 1;
	} else if (deltaX == -1) {
		choice = 2;
	} else {
		choice = 3;
	}
	SidebarPromptChoice(editable, y, prompt, "\30 \31 \33 \32", choice);
	deltaX = NeighborDeltaX[choice];
	deltaY = NeighborDeltaY[choice];
}

void PromptString(integer x, integer y, integer arrowColor,
	integer color,
	integer width, byte mode, TString50 & buffer) {
	integer i;
	string oldBuffer;
	boolean firstKeyPress;

	oldBuffer = buffer;
	firstKeyPress = true;

	if (test_mode_disable_text_input) {
		buffer = "Fuzz mode";
		return;
	}

	do {
		for (i = 0; i <= (width - 1); i ++) {
			video.write(x + i, y, color, " ");
			video.write(x + i, y - 1, arrowColor, " ");
		}
		video.write(x + width, y - 1, arrowColor, " ");
		video.write(x + length(buffer), y - 1,
			(arrowColor / 0x10) * 16 + 0xf,
			"\37");
		video.write(x, y, color, buffer);

		keyboard.wait_for_key();

		if ((length(buffer) < width) && (keyboard.InputKeyPressed >= '\40')
			&& (! keyboard.InputSpecialKeyPressed))  {
			if (firstKeyPress) {
				buffer = "";
			}
			switch (mode) {
				case PROMPT_NUMERIC: {
					if (set::of(range('0', '9'), eos).has(keyboard.InputKeyPressed))  {
						buffer = buffer + (char)keyboard.InputKeyPressed;
					}
				}
				break;
				case PROMPT_ANY: {
					buffer = buffer + (char)keyboard.InputKeyPressed;
				}
				break;
				case PROMPT_ALPHANUM: {
					if ((set::of(range('A', 'Z'),
								eos).has(keyUpCase(keyboard.InputKeyPressed)))
						|| (set::of(range('0', '9'), eos).has(keyboard.InputKeyPressed))
						|| (keyboard.InputKeyPressed == '-')) {
						buffer = buffer + (char)keyUpCase(keyboard.InputKeyPressed);
					}
				}
				break;
			}
		} else if ((keyboard.InputKeyPressed == E_KEY_LEFT)
			|| (keyboard.InputKeyPressed == E_KEY_BACKSPACE))  {
			buffer = copy(buffer, 1, length(buffer) - 1);
			/*IMP: Clear the whole line if Home is pressed.*/
		} else if (keyboard.InputKeyPressed == E_KEY_HOME)  {
			buffer = "";
		}

		firstKeyPress = false;
	} while (!((keyboard.InputKeyPressed == E_KEY_ENTER)
			|| (keyboard.InputKeyPressed == E_KEY_ESCAPE)));
	if (keyboard.InputKeyPressed == E_KEY_ESCAPE)  {
		buffer = oldBuffer;
	}
}

boolean SidebarPromptYesNo(string message, boolean defaultReturn) {
	boolean SidebarPromptYesNo_result;
	SidebarClearLine(3);
	SidebarClearLine(4);
	SidebarClearLine(5);
	video.write(63, 5, 0x1f, message);
	video.write(63 + length(message), 5, 0x9e, "_");

	do {
		keyboard.wait_for_key();
	} while (!(set::of(E_KEY_ESCAPE, 'N', 'Y',
				eos).has(keyUpCase(keyboard.InputKeyPressed))));
	if (keyUpCase(keyboard.InputKeyPressed) == 'Y') {
		defaultReturn = true;
	} else {
		defaultReturn = false;
	}

	SidebarClearLine(5);
	SidebarPromptYesNo_result = defaultReturn;
	return SidebarPromptYesNo_result;
}

void SidebarPromptString(string prompt, TString50 extension,
	TString50 & filename, byte promptMode) {
	SidebarClearLine(3);
	SidebarClearLine(4);
	SidebarClearLine(5);
	video.write(75 - length(prompt), 3, 0x1f, prompt);
	video.write(63, 5, 0xf, string("        ") + extension);

	PromptString(63, 5, 0x1e, 0xf, 8, promptMode, filename);

	SidebarClearLine(3);
	SidebarClearLine(4);
	SidebarClearLine(5);
}

void PauseOnError() {
	video.redraw();
	SoundQueue(1, SoundParse("s004x114x9"));
	Delay(2000);
}

boolean DisplayIOError(bool is_error) {
	varying_string<50> errorNumStr;
	TTextWindowState textWindow;

	if (!is_error)  {
		return false;		// no error
	}

	std::string error_title;

	if (errno == 0) {
		error_title = "Error";
	} else {
		error_title = "Error:" + std::string(strerror(errno));
	}

	/*IMP: Use explanations instead of numeric error codes. */
	textWindow.Title = error_title.c_str();
	TextWindowInitState(textWindow);
	TextWindowAppend(textWindow, "$DOS Error: ");
	TextWindowAppend(textWindow, "");
	TextWindowAppend(textWindow, "This may be caused by missing");
	TextWindowAppend(textWindow, "ZZT files or a bad disk.  If");
	TextWindowAppend(textWindow, "you are trying to save a game,");
	TextWindowAppend(textWindow, "your disk may be full -- try");
	TextWindowAppend(textWindow, "using a blank, formatted disk");
	TextWindowAppend(textWindow, "for saving the game!");

	TextWindowDrawOpen(textWindow);
	TextWindowSelect(textWindow, false, false);
	TextWindowDrawClose(textWindow);
	TextWindowFree(textWindow);

	// So we don't trigger twice on the same error.
	errno = 0;

	return true;
}

boolean DisplayIOError() {
	return DisplayIOError(is_IO_error());
}

bool display_io_error(const std::istream & stream,
	size_t expected_bytes_read) {

	return DisplayIOError(is_IO_error() ||
			stream.gcount() != expected_bytes_read);
}

void DisplayTruncationNote() {
	TTextWindowState textWindow;

	textWindow.Title = "Warning: Potential data loss";
	TextWindowInitState(textWindow);
	TextWindowAppend(textWindow, "$Warning:");
	TextWindowAppend(textWindow, "");
	TextWindowAppend(textWindow,
		"A board that was just saved was too large");
	TextWindowAppend(textWindow,
		"and some data had to be cut. This might");
	TextWindowAppend(textWindow,
		"lead to data loss. If you haven't saved");
	TextWindowAppend(textWindow,
		"yet, do so under another name and make");
	TextWindowAppend(textWindow, "the board smaller!");
	TextWindowAppend(textWindow, "");
	TextWindowAppend(textWindow,
		"If you're just playing, tell the author");
	TextWindowAppend(textWindow, "of the world that you're playing.");

	TextWindowDrawOpen(textWindow);
	TextWindowSelect(textWindow, false, false);
	TextWindowDrawClose(textWindow);
	TextWindowFree(textWindow);
}

void DisplayCorruptionNote(std::string corruption_type) {
	TTextWindowState textWindow;

	textWindow.Title = "Warning: Corruption detected";
	TextWindowInitState(textWindow);
	TextWindowAppend(textWindow, "$Warning:");
	TextWindowAppend(textWindow, "");
	TextWindowAppend(textWindow,
		"The file or board that was just loaded");
	TextWindowAppend(textWindow, "contained some damaged information.");
	TextWindowAppend(textWindow, "This might be caused by a bad file");
	TextWindowAppend(textWindow, "or disk corruption. ZZT has tried");
	TextWindowAppend(textWindow, "to undo the damage, but some data");
	TextWindowAppend(textWindow, "might be lost.");

	if (corruption_type != "") {
		TextWindowAppend(textWindow, "");
		TextWindowAppend(textWindow, "This was corrupted:");
		TextWindowAppend(textWindow, (" - " + corruption_type + ".").c_str());
	}

	TextWindowDrawOpen(textWindow);
	TextWindowSelect(textWindow, false, false);
	TextWindowDrawClose(textWindow);
	TextWindowFree(textWindow);
}

void WorldUnload() {
	integer i;

	/* no need to show any notices if the world's to be unloaded. */
	BoardClose(false);
	for (i = 0; i <= game_world->BoardCount; i ++) {
		game_world->BoardData[i] = {};
	}
}

static integer loadProgress;

static void SidebarAnimateLoading() {
	video.write(69, 5, ProgressAnimColors[loadProgress],
		ProgressAnimStrings[loadProgress]);
	loadProgress = (loadProgress + 1) % 8;
}

// Returns true if loading went okay, false otherwise.
bool load_board_from_file(std::istream & f, bool is_final_board,
	std::vector<unsigned char> & out_packed_board) {

	short board_len = 0, actually_read;
	bool successful_read = true;

	load_lsb_from_file(f, board_len);

	/* Sanity check. If the board length is less than zero, stop here. */
	if (is_IO_error() || board_len < 0)  {
		board_len = 0;
		// TODO? Not do this so that the board already at this position
		// is untouched if loading fails, e.g. when importing a .BRD?
		// But then we need some other way to signal board_len == 0.
		out_packed_board.resize(0);
		return false;
	} else {
		/* If it's the last board, get everything we can.
			This recovers the last Super Lock-corrupted game_world->currentBoard.
			actuallyRead below will adjust the board length back
			if we're dealing with an ordinary world. */
		if (is_final_board) {
			board_len = MAX_BOARD_LEN;
		}

		out_packed_board.resize(board_len);
		f.read((char *)out_packed_board.data(), board_len);
		actually_read = f.gcount();

		/* SANITY: If reading the whole board would lead to an
			overflow down the line, pretend we only read the
			MAX_BOARD_LEN first. */
		if (actually_read > MAX_BOARD_LEN)  {
			actually_read = MAX_BOARD_LEN;
			successful_read = false;
		}

		out_packed_board.resize(actually_read);
	}

	return successful_read;
}

bool WorldLoad(std::istream & f, const std::string world_name) {
	std::vector<unsigned char>::const_iterator ptr;
	integer boardId;
	word actuallyRead;
	integer firstZero;
	integer i;
	boolean worldIsDamaged;

	boolean WorldLoad_result;
	WorldLoad_result = false;
	worldIsDamaged = false;
	loadProgress = 0;

	SidebarClearLine(4);
	SidebarClearLine(5);
	SidebarClearLine(5);
	video.write(62, 5, 0x1f, "Loading.....");

	// Handle a AFL-QEMU + FreePascal error where errno doesn't get set.
	// BLUESKY: Report this error somehow because it was very hard to find.
	errno = 0;

	if (world_name == "")  {
		return WorldLoad_result;
	}

	if (! DisplayIOError())  {
		// Unload the current error. TODO: Do RAII-style.
		WorldUnload();

		// Buffer.
		std::vector<unsigned char> world_header(512);

		f.read((char *)world_header.data(), 512);
		ptr = world_header.begin();

		// If it's either an IO error or attempted read past end of file,
		// signal error (as TP would).
		if (display_io_error(f, 512)) {
			return false;
		} else {
			ptr = load_lsb_element(ptr, game_world->BoardCount);

			// If the file starts FF, then the board count is next.
			// Otherwise, if below zero, that indicates another version
			// of ZZT. Otherwise (positive count), that's the board count.
			if (game_world->BoardCount < 0)  {
				if (game_world->BoardCount != -1)  {
					video.write(63, 5, 0x1e, "You need a newer");
					video.write(63, 6, 0x1e, " version of ZZT!");
					return WorldLoad_result;
				} else {
					ptr = load_lsb_element(ptr, game_world->BoardCount);
				}
			}

			// TODO: I know these aren't vector pointers. fix later.
			ptr = game_world->Info.load(ptr, world_header.end());

			/* If the board count is negative, set it to zero. This should
			also signal that the world is corrupt. Another option would be
			to make all the fields unsigned, but who needs worlds with more
			than 32 thousand boards anyway? Besides, they'd crash DOS ZZT. */
			if (game_world->BoardCount < 0)  {
				game_world->BoardCount = 0;
				worldIsDamaged = true;
			}

			/* If there are too many boards, ditto. (That's a more serious
			problem, as it may cut off boards outright.) */
			if (game_world->BoardCount > MAX_BOARD)  {
				game_world->BoardCount = MAX_BOARD;
				worldIsDamaged = true;
			}

			/* Don't accept CurrentBoard values that are too large or
			small. */
			if (game_world->Info.CurrentBoardIdx > game_world->BoardCount ||
				game_world->Info.CurrentBoardIdx < 0)  {

				game_world->Info.CurrentBoardIdx = Max(0, Min(game_world->BoardCount,
							game_world->Info.CurrentBoardIdx));
				worldIsDamaged = true;
			}

			for (boardId = 0; boardId <= game_world->BoardCount; boardId ++) {
				SidebarAnimateLoading();

				if (boardId > game_world->BoardCount)  {
					continue;
				}

				bool is_final_board = (boardId == game_world->BoardCount);

				bool successful_board_read = load_board_from_file(
						f, is_final_board, game_world->BoardData[boardId]);

				worldIsDamaged |= !successful_board_read;

				// Display IO errors if any occurred during the board read.
				DisplayIOError();

				// If the board is empty, something went wrong,
				if (game_world->BoardData[boardId].size() == 0) {
					if (boardId == 0) {
						WorldUnload();
						return WorldLoad_result;
					}
					game_world->BoardCount = boardId - 1;
					/* No more boards to be had, so break. */
					break;
				}
			}

			/* More sanity checks. If the current board number is negative
				or too high, set it to zero. (Maybe instead set it to the
				actual number of boards read?) */
			if ((game_world->Info.CurrentBoardIdx < 0) ||
				(game_world->Info.CurrentBoardIdx > Min(MAX_BOARD,
						game_world->BoardCount))) {
				game_world->Info.CurrentBoardIdx = 0;
			}

			BoardOpen(game_world->Info.CurrentBoardIdx, worldIsDamaged);
			LoadedGameFileName = world_name.c_str();

			// XXX: Once we add in editor.cxx
			HighScoresLoad();

			SidebarClearLine(5);
		}
	}
	return true;
}

bool WorldLoad(std::string filename, std::string extension) {
	// Fuzz hack: if preloaded_world_data is set, load from it instead.
	if (!preloaded_world_data.empty()) {
		return WorldLoad(preloaded_world_data, filename + extension);
	}
	std::string full_filename = std::string(filename + extension);
	std::ifstream f = OpenForRead(full_filename);
	return WorldLoad(f, filename.c_str());
}

bool WorldLoad(uint8_t * input, size_t len,
	std::string full_filename) {
	imemstream stream((const char*)input, len);
	return WorldLoad(stream, full_filename);
}

bool WorldLoad(const std::vector<char> & input,
	std::string full_filename) {
	imemstream stream((const char*)input.data(), input.size());
	return WorldLoad(stream, full_filename);
}

void WorldSave(std::string filename, std::string extension) {
	integer i;
	integer unk1;
	TIoTmpBuf * ptr;
	integer version;

	BoardClose(true);
	video.write(63, 5, 0x1f, "Saving...");

	std::string full_filename = std::string(filename + extension);
	std::ofstream out_file = OpenForWrite(full_filename);

	// TODO IMP? Perhaps write to a temporary filename and then move it over
	// the original to create some kind of atomicity?
	// https://en.cppreference.com/w/cpp/io/c/tmpnam etc.

	if (! DisplayIOError())  {
		try {
			game_world->save(out_file, false);
			out_file.close();
		} catch (std::runtime_error & e) {
			out_file.close();
			std::remove(full_filename.c_str()); // Delete the corrupted file.

			BoardOpen(game_world->Info.CurrentBoardIdx, false);
			SidebarClearLine(5);
			DisplayIOError();
		}
	}

	BoardOpen(game_world->Info.CurrentBoardIdx, false);
	SidebarClearLine(5);
	return;
}

std::vector<char> WorldSaveVector() {
	int i;

	BoardClose(true);

	std::vector<unsigned char> output;

	append_lsb_element((short)-1, output); // Version
	append_lsb_element(game_world->BoardCount, output);
	game_world->Info.dump(output);

	// Pad to 512
	output.resize(512, 0);

	for (i = 0; i <= game_world->BoardCount; i ++) {
		// TODO: Replace with a serialization procedure that's
		// machine endian agnostic.
		unsigned short board_len = game_world->BoardData[i].size();
		append_lsb_element(board_len, output);
		std::copy(game_world->BoardData[i].begin(), game_world->BoardData[i].end(),
			std::back_inserter(output));
	}

	BoardOpen(game_world->Info.CurrentBoardIdx, false);
	std::vector<char> out_signed;
	out_signed.assign(output.begin(), output.end());
	return out_signed;
}

void GameWorldSave(TString50 prompt, TString50 & filename,
	TString50 extension) {
	TString50 newFilename;

	newFilename = filename;
	SidebarPromptString(prompt, extension, newFilename, PROMPT_ALPHANUM);
	if ((keyboard.InputKeyPressed != E_KEY_ESCAPE)
		&& (length(newFilename) != 0))  {
		filename = newFilename;
		if (extension == ".ZZT") {
			game_world->Info.Name = filename;
		}
		WorldSave(filename.str(), extension.str());
	}
}

boolean GameWorldLoad(TString50 extension) {
	TTextWindowState textWindow;
	string entryName;
	integer i;

	boolean GameWorldLoad_result;
	TextWindowInitState(textWindow);
	if (extension == ".ZZT") {
		textWindow.Title = "ZZT Worlds";
	} else {
		textWindow.Title = "Saved Games";
	}
	GameWorldLoad_result = false;
	textWindow.Selectable = true;

	// https://stackoverflow.com/questions/612097

	DIR * dir;
	struct dirent * ent;
	if ((dir = opendir(".")) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			std::string filename = ent->d_name;
			// If it doesn't have the right extension, skip.
			// TODO: Make case-insensitive. But the way extensions are
			// handled currently makes that a pain.
			if (filename.find(extension) == std::string::npos) {
				continue;
			}

			// Strip the extension
			filename = filename.substr(0, filename.find(extension));
			TextWindowAppend(textWindow, filename.c_str());
		}
		closedir(dir);
	}

	TextWindowSort(textWindow); /* Sort the file names. */
	TextWindowAppend(textWindow, "Exit");

	TextWindowDrawOpen(textWindow);
	TextWindowSelect(textWindow, false, false);
	TextWindowDrawClose(textWindow);

	if ((textWindow.LinePos < textWindow.LineCount)
		&& ! TextWindowRejected)  {
		entryName = *textWindow.Lines[textWindow.LinePos];
		if (pos(" ", entryName) != 0) {
			entryName = copy(entryName, 1, pos(" ", entryName) - 1);
		}

		GameWorldLoad_result = WorldLoad(std::string(entryName),
				std::string(extension));
		TransitionDrawToFill('\333', 0x44);
	}

	TextWindowFree(textWindow);
	return GameWorldLoad_result;
}

void CopyStatDataToTextWindow(integer statId,
	TTextWindowState & state) {
	string dataStr;
	char dataChr;
	integer i;

	TStat & with = game_world->currentBoard.Stats[statId];
	TextWindowInitState(state);
	dataStr = "";

	/* IMP: Fix off-by-one: Don't start counting
	from 0 when copying data. */
	for (i = 0; i < with.DataLen; i ++) {
		dataChr = with.data.get()[i];
		if (dataChr == '\r')  {
			TextWindowAppend(state, dataStr);
			dataStr = "";
		} else {
			dataStr = dataStr + dataChr;
		}
	}
}

void AddStat(integer tx, integer ty, byte element, integer color,
	integer tcycle, TStat template_) {
	/* First of all: check if we have space. If not, no can do! */
	if (game_world->currentBoard.get_packed_size() +
		template_.upper_bound_packed_size() >
		MAX_BOARD_LEN) {
		return;
	}

	game_world->currentBoard.add_stat(tx, ty, element, color, tcycle,
		template_);

	if (CoordInsideViewport(tx, ty)) {
		BoardDrawTile(tx, ty);
	}
}

void RemoveStat(integer statId) {
	int x, y;

	if (!game_world->currentBoard.remove_stat(statId, x, y,
			CurrentStatTicked)) {
		return;
	}

	if (CoordInsideViewport(x, y)) {
		BoardDrawTile(x, y);
	}
}

integer GetStatIdAt(integer x, integer y) {
	integer i;

	i = -1;
	do {
		i = i + 1;
	} while (!(((game_world->currentBoard.Stats[i].X == x)
				&& (game_world->currentBoard.Stats[i].Y == y))
			|| (i > game_world->currentBoard.StatCount)));

	if (i > game_world->currentBoard.StatCount) {
		return -1;
	} else {
		return i;
	}
}

boolean BoardPrepareTileForPlacement(integer x, integer y) {
	integer statId;
	boolean result;

	boolean BoardPrepareTileForPlacement_result;

	if (! ValidCoord(x, y)) {
		return false;
	}

	statId = GetStatIdAt(x, y);
	if (statId > 0)  {
		RemoveStat(statId);
		result = true;
	} else if (statId < 0)  {
		if (! elem_info_ptr->defs[game_world->currentBoard.Tiles[x][y].Element].PlaceableOnTop) {
			game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
		}
		result = true;
	} else {       /* statId = 0 (player) cannot be modified */
		result = false;
	}
	BoardDrawTile(x, y);
	BoardPrepareTileForPlacement_result = result;
	return BoardPrepareTileForPlacement_result;
}

timespec monotime, first_monotime, subtracted_monotime,
	last_skip, paused, unpaused;
bool got_first_monotime = false, got_first_skip = false;
bool set_paused = false;

template<typename T> void print_timedelta(timespec before, timespec after,
	T & stream_out) {

	int64_t duration = (int64_t)(after.tv_sec - before.tv_sec) * 
		(int64_t)1000000000UL + (int64_t)(after.tv_nsec - before.tv_nsec);

	int64_t seconds = duration/1000000000UL,
	nanosecs = duration % 1000000000UL;

	stream_out << seconds << "." << nanosecs;
}

void MoveStat(integer statId, integer newX, integer newY) {
	TTile iUnder;
	integer ix, iy;
	integer oldX, oldY;
	integer oldBgColor;

	// HACK
	// Print a debug message when the player is moved, also giving the
	// time when that happened. This will be used to check puzzle solving
	// times when solving ZZT puzzles; I can just read off the first time
	// the player gets to the tile just in front of the next puzzle and use
	// the deltas to figure out how long it took me to solve the puzzle.

	// TODO: Instead of triggering on red fake, trigger on transports (new x,y more
	// than 1 away in some direction). The player may need to step on the red
	// fake to move around in the puzzle, but should only be doing transports
	// when exiting or entering a puzzle (or moving around the board).

	// Also TODO: Use the board name, not the world name!

	if (statId == 0) { // player
		clock_gettime(CLOCK_MONOTONIC, &monotime);
		if (!got_first_monotime) {
			got_first_monotime = true;
			first_monotime = monotime;
		}

		// Make the output a little bit cleaner by subtracting the
		// first_monotime.

		std::cerr << game_world->currentBoard.Name << ": Player to " << newX << ", " << newY <<
			" at ";
		print_timedelta(first_monotime, monotime, std::cerr);
		std::cerr << "\n";

		TTile dest = game_world->currentBoard.Tiles[newX][newY];

		int oldX = game_world->currentBoard.Stats[statId].X;
		int oldY = game_world->currentBoard.Stats[statId].Y;

		int max_distance = std::max(abs(newX - oldX), abs(newY - oldY));

		if (max_distance > 1) {
			std::cerr << game_world->currentBoard.Name << 
				": Player skip to " << newX 
				<< ", " << newY << "  ";

			if (got_first_skip) {
				std::cerr << "time elapsed: ";
				print_timedelta(last_skip, monotime, std::cerr);
				last_skip = monotime;
			} else {
				std::cerr << "first time.";
				last_skip = monotime;
				got_first_skip = true;
			}
			std::cerr << " Absolute time: ";
			print_timedelta(first_monotime, monotime, std::cerr);
			std::cerr << "\n";
		}
	}

	if (statId > MAX_STAT) {
		throw std::logic_error("Trying to move statId greater than MAX_STAT");
	}
	if (statId == -1) {
		throw std::logic_error("Trying to move noexisting stat (-1)");
	}
	// No point in moving a stat to its own tile
	if ((game_world->currentBoard.Stats[statId].X == newX)
		&& (game_world->currentBoard.Stats[statId].Y == newY)) {
		return;
	}
	// And not allowed to move something outside of the viewport.
	if (!CoordInsideViewport(newX, newY)) {
		return;
	}

	TStat & with = game_world->currentBoard.Stats[statId];
	oldBgColor = game_world->currentBoard.Tiles[newX][newY].Color & 0xf0;

	iUnder = game_world->currentBoard.Stats[statId].Under;
	game_world->currentBoard.Stats[statId].Under =
		game_world->currentBoard.Tiles[newX][newY];

	/* If trying to move atop the player, reject this. The object
	   is simply destroyed instead, as if a player was set on top
	   afterwards. */
	if ((newX == game_world->currentBoard.Stats[0].X)
		&& (newY == game_world->currentBoard.Stats[0].Y))  {
		RemoveStat(statId);
		return;
	}

	if (game_world->currentBoard.Tiles[with.X][with.Y].Element == E_PLAYER) {
		game_world->currentBoard.Tiles[newX][newY].Color =
			game_world->currentBoard.Tiles[with.X][with.Y].Color;
	} else if (game_world->currentBoard.Tiles[newX][newY].Element == E_EMPTY)
		game_world->currentBoard.Tiles[newX][newY].Color =
			game_world->currentBoard.Tiles[with.X][with.Y].Color &
			0xf;
	else
		game_world->currentBoard.Tiles[newX][newY].Color =
			(game_world->currentBoard.Tiles[with.X][with.Y].Color &
				0xf) + (game_world->currentBoard.Tiles[newX][newY].Color & 0x70);

	game_world->currentBoard.Tiles[newX][newY].Element =
		game_world->currentBoard.Tiles[with.X][with.Y].Element;
	/* Don't remove the player i
	f he's at the old position. This can
	happen with multiple stats with the same coordinate. */
	if ((statId == 0) || (with.X != game_world->currentBoard.Stats[0].X)
		|| (with.Y != game_world->currentBoard.Stats[0].Y)) {
		game_world->currentBoard.Tiles[with.X][with.Y] = iUnder;
	}

	oldX = with.X;
	oldY = with.Y;
	with.X = newX;
	with.Y = newY;

	BoardDrawTile(with.X, with.Y);
	BoardDrawTile(oldX, oldY);

	if ((statId == 0) && game_world->currentBoard.Info.IsDark
		&& (game_world->Info.TorchTicks > 0))  {
		if ((sqr(oldX-with.X) + sqr(oldY-with.Y)) == 1)  {
			for (ix = (with.X - TORCH_DX - 3); ix <= (with.X + TORCH_DX + 3);
				ix ++)
				if ((ix >= 1) && (ix <= BOARD_WIDTH))
					for (iy = (with.Y - TORCH_DY - 3); iy <= (with.Y + TORCH_DY + 3);
						iy ++)
						if ((iy >= 1) && (iy <= BOARD_HEIGHT))
							if ((((sqr(ix-oldX))+(sqr(iy-oldY)*2)) < TORCH_DIST_SQR) ^
								(((sqr(ix-newX))+(sqr(iy-newY)*2)) < TORCH_DIST_SQR)) {
								BoardDrawTile(ix, iy);
							}
		} else {
			DrawPlayerSurroundings(oldX, oldY, 0);
			DrawPlayerSurroundings(with.X, with.Y, 0);
		}
	}
}

void PopupPromptString(string question, TString50 & buffer) {
	integer x, y;

	// TODO: Move to txtwind.cxx
	video.write(3, 18, 0x4f, text_window_str_top);
	video.write(3, 19, 0x4f, text_window_str_text);
	video.write(3, 20, 0x4f, text_window_str_sep);
	video.write(3, 21, 0x4f, text_window_str_text);
	video.write(3, 22, 0x4f, text_window_str_text);
	video.write(3, 23, 0x4f, text_window_str_bottom);
	// TODO: "CenterText" function in txtwind.cxx
	video.write(4 + (TextWindowWidth - length(question)) / 2, 19,
		0x4f, question);
	buffer = "";
	PromptString(10, 22, 0x4f, 0x4e, TextWindowWidth - 16, PROMPT_ANY,
		buffer);
	for (y = 18; y <= 23; y ++)
		for (x = 3; x <= (TextWindowWidth + 3); x ++) {
			BoardDrawTile(x + 1, y + 1);
		}
}

void popup_prompt_string(const std::string question,
	std::string & buffer) {

	TString50 buf;
	PopupPromptString(question.c_str(), buf);
	buffer = buf;
}

integer Signum(integer val) {
	integer Signum_result;
	if (val > 0) {
		Signum_result = 1;
	} else if (val < 0) {
		Signum_result = -1;
	} else {
		Signum_result = 0;
	}
	return Signum_result;
}

integer Difference(integer a, integer b) {
	integer Difference_result;
	if ((a - b) >= 0) {
		Difference_result = a - b;
	} else {
		Difference_result = b - a;
	}
	return Difference_result;
}

void GameUpdateSidebar() {
	varying_string<8> numStr;
	integer i;

	if (GameStateElement == E_PLAYER)  {
		if (game_world->currentBoard.Info.TimeLimitSec > 0)  {
			video.write(64, 6, 0x1e, "   Time:");
			str(game_world->currentBoard.Info.TimeLimitSec -
				game_world->Info.BoardTimeSec,
				numStr);
			video.write(72, 6, 0x1e, numStr + ' ');
		} else {
			SidebarClearLine(6);
		}

		if (game_world->Info.Health < 0) {
			game_world->Info.Health = 0;
		}

		str(game_world->Info.Health, numStr);
		video.write(72, 7, 0x1e, numStr + ' ');
		str(game_world->Info.Ammo, numStr);
		video.write(72, 8, 0x1e, numStr + "  ");
		str(game_world->Info.Torches, numStr);
		video.write(72, 9, 0x1e, numStr + ' ');
		str(game_world->Info.Gems, numStr);
		video.write(72, 10, 0x1e, numStr + ' ');
		str(game_world->Info.Score, numStr);
		video.write(72, 11, 0x1e, numStr + ' ');

		if (game_world->Info.TorchTicks == 0) {
			video.write(75, 9, 0x16, "    ");
		} else {
			for (i = 2; i <= 5; i ++) {
				if (i <= ((game_world->Info.TorchTicks * 5) / TORCH_DURATION)) {
					video.write(73 + i, 9, 0x16, "\261");
				} else {
					video.write(73 + i, 9, 0x16, "\260");
				}
			}
		}

		for (i = 1; i <= 7; i ++) {
			if (game_world->Info.HasKey(i))
				video.write(71 + i, 12, 0x18 + i,
					elem_info_ptr->defs[E_KEY].Character);
			else {
				video.write(71 + i, 12, 0x1f, " ");
			}
		}

		if (SoundEnabled) {
			video.write(65, 15, 0x1f, " Be quiet");
		} else {
			video.write(65, 15, 0x1f, " Be noisy");
		}

		if (DebugEnabled)  {
			/* TODO: Replace with some interesting stat
			on Linux.*/
			numStr = "lots";
			video.write(69, 4, 0x1e, string('m') + numStr + ' ');
		}
	}
}

void DisplayMessage(integer time, string message) {
	if (GetStatIdAt(0, 0) != -1)  {
		RemoveStat(GetStatIdAt(0, 0));
		BoardDrawBorder();
	}

	if (length(message) != 0)  {
		AddStat(0, 0, E_MESSAGE_TIMER, 0, 1, StatTemplateDefault);
		/*IMP: P2 is a byte, so it can hold a max value of 255.*/
		game_world->currentBoard.Stats[game_world->currentBoard.StatCount].P2 =
			Min(255,
				time / (TickTimeDuration + 1));
		game_world->currentBoard.Info.Message = message;
	}
}

void DamageStat(integer attackerStatId) {
	integer oldX, oldY;

	TStat & with = game_world->currentBoard.Stats[attackerStatId];
	if (attackerStatId == 0)  {
		if (game_world->Info.Health > 0)  {
			game_world->Info.Health = game_world->Info.Health - 10;

			GameUpdateSidebar();
			DisplayMessage(100, "Ouch!");

			game_world->currentBoard.Tiles[with.X][with.Y].Color = 0x70 +
				(elem_info_ptr->defs[4].Color %
					0x10);

			if (game_world->Info.Health > 0)  {
				game_world->Info.BoardTimeSec = 0;
				if (game_world->currentBoard.Info.ReenterWhenZapped)  {
					SoundQueue(4, "\40\1\43\1\47\1\60\1\20\1");

					/* Move player to start */
					oldX = with.X;
					oldY = with.Y;
					if (ValidCoord(game_world->currentBoard.Info.StartPlayerX,
							game_world->currentBoard.Info.StartPlayerY)) {
						MoveStat(0, game_world->currentBoard.Info.StartPlayerX,
							game_world->currentBoard.Info.StartPlayerY);
					}
					BoardDrawTile(oldX, oldY);
					DrawPlayerSurroundings(oldX, oldY, 0);
					DrawPlayerSurroundings(with.X, with.Y, 0);

					//if (! FuzzMode) {
					clock_gettime(CLOCK_MONOTONIC, &paused);
					GamePaused = true;
					//}
				}
				SoundQueue(4, "\20\1\40\1\23\1\43\1");
			} else {
				SoundQueue(5,
					"\40\3\43\3\47\3\60\3\47\3\52\3\62\3\67\3\65\3\70\3\100\3\105\3\20\n");
			}
		}
	} else {
		switch (game_world->currentBoard.Tiles[with.X][with.Y].Element) {
			case E_BULLET: SoundQueue(3, "\40\1"); break;
			case E_OBJECT: {; } break;
			default:
				SoundQueue(3, "\100\1\20\1\120\1\60\1");
		}
		RemoveStat(attackerStatId);
	}
}

void BoardDamageTile(integer x, integer y) {
	integer statId;

	statId = GetStatIdAt(x, y);
	if (statId != -1)  {
		DamageStat(statId);
	} else {
		game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
		BoardDrawTile(x, y);
	}
}

void BoardAttack(integer attackerStatId, integer x, integer y) {
	if ((attackerStatId == 0) && (game_world->Info.EnergizerTicks > 0))  {
		game_world->Info.Score =
			elem_info_ptr->defs[game_world->currentBoard.Tiles[x][y].Element].ScoreValue
			+
			game_world->Info.Score;
		GameUpdateSidebar();
	} else {
		DamageStat(attackerStatId);
	}

	if ((attackerStatId > 0) && (attackerStatId <= CurrentStatTicked)) {
		CurrentStatTicked = CurrentStatTicked - 1;
	}

	if ((game_world->currentBoard.Tiles[x][y].Element == E_PLAYER)
		&& (game_world->Info.EnergizerTicks > 0))  {
		game_world->Info.Score =
			elem_info_ptr->defs[game_world->currentBoard.Tiles[game_world->currentBoard.Stats[attackerStatId].X]
				[game_world->currentBoard.Stats[attackerStatId].Y].Element]
			.ScoreValue + game_world->Info.Score;
		GameUpdateSidebar();
	} else {
		BoardDamageTile(x, y);
		SoundQueue(2, "\20\1");
	}
}

boolean BoardShoot(byte element, integer tx, integer ty,
	integer deltaX,
	integer deltaY, integer source) {
	boolean BoardShoot_result;

	if (! ValidCoord(tx + deltaX, ty + deltaY)) {
		return false;
	}

	if (elem_info_ptr->defs[game_world->currentBoard.Tiles[tx + deltaX][ty +
												  deltaY].Element].Walkable
		|| (game_world->currentBoard.Tiles[tx + deltaX][ty + deltaY].Element ==
			E_WATER)) {
		AddStat(tx + deltaX, ty + deltaY, element,
			elem_info_ptr->defs[element].Color,
			1,
			StatTemplateDefault);
		{
			TStat & with =
				game_world->currentBoard.Stats[game_world->currentBoard.StatCount];
			with.P1 = source;
			with.StepX = deltaX;
			with.StepY = deltaY;
			with.P2 = 100;
		}
		BoardShoot_result = true;
	} else if ((game_world->currentBoard.Tiles[tx + deltaX][ty +
				deltaY].Element ==
			E_BREAKABLE)
		|| (
			elem_info_ptr->defs[game_world->currentBoard.Tiles[tx + deltaX][ty +
													  deltaY].Element].Destructible
			&& ((game_world->currentBoard.Tiles[tx + deltaX][ty + deltaY].Element ==
					E_PLAYER) ==
				(boolean)(source))
			&& (game_world->Info.EnergizerTicks <= 0)
		)) {
		BoardDamageTile(tx + deltaX, ty + deltaY);
		SoundQueue(2, "\20\1");
		BoardShoot_result = true;
	} else {
		BoardShoot_result = false;
	}
	return BoardShoot_result;
}

void CalcDirectionRnd(integer & deltaX, integer & deltaY) {
	deltaX = rnd.randint(3) - 1;

	if (deltaX == 0) {
		deltaY = rnd.randint(2) * 2 - 1;
	} else {
		deltaY = 0;
	}
}

void CalcDirectionSeek(integer x, integer y, integer & deltaX,
	integer & deltaY) {
	deltaX = 0;
	deltaY = 0;

	if ((rnd.randint(2) < 1) || (game_world->currentBoard.Stats[0].Y == y)) {
		deltaX = Signum(game_world->currentBoard.Stats[0].X - x);
	}

	if (deltaX == 0) {
		deltaY = Signum(game_world->currentBoard.Stats[0].Y - y);
	}

	if (game_world->Info.EnergizerTicks > 0)  {
		deltaX = -deltaX;
		deltaY = -deltaY;
	}
}

void TransitionDrawBoardChange() {
	// This is a hotspot, so skip if we've disabled video.
	if (test_mode_disable_video) {
		return;
	}

	TransitionDrawToFill('\333', 0x5);
	TransitionDrawToBoard();
}

void BoardEnter() {
	game_world->currentBoard.Info.StartPlayerX =
		game_world->currentBoard.Stats[0].X;
	game_world->currentBoard.Info.StartPlayerY =
		game_world->currentBoard.Stats[0].Y;

	if (game_world->currentBoard.Info.IsDark && MessageHintTorchNotShown)  {
		DisplayMessage(200, "Room is dark - you need to light a torch!");
		MessageHintTorchNotShown = false;
	}

	game_world->Info.BoardTimeSec = 0;
	GameUpdateSidebar();
}

void BoardPassageTeleport(integer x, integer y) {
	integer oldBoard;
	byte col;
	integer ix, iy;
	integer newX, newY;

	col = game_world->currentBoard.Tiles[x][y].Color;

	oldBoard = game_world->Info.CurrentBoardIdx;

	/* Handle passages without stats. */
	if (GetStatIdAt(x, y) < 0) {
		BoardChange(oldBoard);
	} else {
		BoardChange(game_world->currentBoard.Stats[GetStatIdAt(x, y)].P3);
	}

	/* Set a default position that's outside the viewport, so that if
	there's no passage of the appropriate color at the destination
	board, the player appears at his initial location at the
	destination. (Do what DOS ZZT does, but without
	relying on out-of-bounds memory access...) */
	newX = 0;
	newY = 0;

	for (ix = 1; ix <= BOARD_WIDTH; ix ++) {
		for (iy = 1; iy <= BOARD_HEIGHT; iy ++) {
			if ((game_world->currentBoard.Tiles[ix][iy].Element == E_PASSAGE)
				&& (game_world->currentBoard.Tiles[ix][iy].Color == col)) {
				newX = ix;
				newY = iy;
			}
		}
	}

	/* Move the player onto the passage. */
	MoveStat(0, newX, newY);

	clock_gettime(CLOCK_MONOTONIC, &paused);
	GamePaused = true;
	SoundQueue(4,
		"\60\1\64\1\67\1\61\1\65\1\70\1\62\1\66\1\71\1\63\1\67\1\72\1\64\1\70\1\100\1");
	TransitionDrawBoardChange();
	BoardEnter();
}

void GameDebugPrompt() {
	TString50 input;
	integer i;
	boolean toggle;

	input = "";
	SidebarClearLine(4);
	SidebarClearLine(5);

	PromptString(63, 5, 0x1e, 0xf, 11, PROMPT_ANY, input);
	for (i = 1; i <= length(input); i ++) {
		input[i] = upcase(input[i]);
	}

	toggle = true;
	if ((input[1] == '+') || (input[1] == '-'))  {
		if (input[1] == '-') {
			toggle = false;
		}
		input = copy(input, 2, length(input) - 1);

		if (toggle == true) {
			WorldSetFlag(input);
		} else {
			WorldClearFlag(input);
		}
	}

	DebugEnabled = WorldGetFlagPosition("DEBUG") >= 0;

	if (input == "HEALTH") {
		game_world->Info.Health = game_world->Info.Health + 50;
	} else if (input == "AMMO") {
		game_world->Info.Ammo = game_world->Info.Ammo + 5;
	} else if (input == "KEYS")
		for (i = 1; i <= 7; i ++) {
			game_world->Info.GiveKey(i);
		} else if (input == "TORCHES") {
		game_world->Info.Torches = game_world->Info.Torches + 3;
	} else if (input == "TIME") {
		game_world->Info.BoardTimeSec = game_world->Info.BoardTimeSec - 30;
	} else if (input == "GEMS") {
		game_world->Info.Gems = game_world->Info.Gems + 5;
	} else if (input == "DARK")  {
		game_world->currentBoard.Info.IsDark = toggle;
		TransitionDrawToBoard();
	} else if (input == "ZAP")  {
		for (i = 0; i <= 3; i ++) {
			BoardDamageTile(game_world->currentBoard.Stats[0].X + NeighborDeltaX[i],
				game_world->currentBoard.Stats[0].Y + NeighborDeltaY[i]);
			game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X +
				NeighborDeltaX[i]][game_world->currentBoard.Stats[0].Y +
					NeighborDeltaY[i]].Element = E_EMPTY;
			BoardDrawTile(game_world->currentBoard.Stats[0].X + NeighborDeltaX[i],
				game_world->currentBoard.Stats[0].Y + NeighborDeltaY[i]);
		}
	}

	SoundQueue(10, "\47\4");
	SidebarClearLine(4);
	SidebarClearLine(5);
	GameUpdateSidebar();
}

void GameAboutScreen() {
	TextWindowDisplayFile("ABOUT.HLP", "About ZZT...");
}

static void GameDrawSidebar() {
	SidebarClear();
	SidebarClearLine(0);
	SidebarClearLine(1);
	SidebarClearLine(2);
	video.write(61, 0, 0x1f, "    - - - - -      ");
	video.write(62, 1, 0x70, "      ZZT      ");
	video.write(61, 2, 0x1f, "    - - - - -      ");
	if (GameStateElement == E_PLAYER)  {
		video.write(64, 7, 0x1e, " Health:");
		video.write(64, 8, 0x1e, "   Ammo:");
		video.write(64, 9, 0x1e, "Torches:");
		video.write(64, 10, 0x1e, "   Gems:");
		video.write(64, 11, 0x1e, "  Score:");
		video.write(64, 12, 0x1e, "   Keys:");
		video.write(62, 7, 0x1f, elem_info_ptr->defs[E_PLAYER].Character);
		video.write(62, 8, 0x1b, elem_info_ptr->defs[E_AMMO].Character);
		video.write(62, 9, 0x16, elem_info_ptr->defs[E_TORCH].Character);
		video.write(62, 10, 0x1b, elem_info_ptr->defs[E_GEM].Character);
		video.write(62, 12, 0x1f, elem_info_ptr->defs[E_KEY].Character);
		video.write(62, 14, 0x70, " T ");
		video.write(65, 14, 0x1f, " Torch");
		video.write(62, 15, 0x30, " B ");
		video.write(62, 16, 0x70, " H ");
		video.write(65, 16, 0x1f, " Help");
		video.write(67, 18, 0x30, " \30\31\32\33 ");
		video.write(72, 18, 0x1f, " Move");
		video.write(61, 19, 0x70, " Shift \30\31\32\33 ");
		video.write(72, 19, 0x1f, " Shoot");
		video.write(62, 21, 0x70, " S ");
		video.write(65, 21, 0x1f, " Save game");
		video.write(62, 22, 0x30, " P ");
		video.write(65, 22, 0x1f, " Pause");
		video.write(62, 23, 0x70, " Q ");
		video.write(65, 23, 0x1f, " Quit");
	} else if (GameStateElement == E_MONITOR)  {
		SidebarPromptSlider(false, 66, 21, "Game speed:;FS", TickSpeed, 256);
		video.write(62, 21, 0x70, " S ");
		video.write(62, 7, 0x30, " W ");
		video.write(65, 7, 0x1e, " World:");

		if (game_world->Info.Name.size() != 0) {
			video.write(69, 8, 0x1f, game_world->Info.Name);
		} else {
			video.write(69, 8, 0x1f, "Untitled");
		}

		video.write(62, 11, 0x70, " P ");
		video.write(65, 11, 0x1f, " Play");
		video.write(62, 12, 0x30, " R ");
		video.write(65, 12, 0x1e, " Restore game");
		video.write(62, 13, 0x70, " Q ");
		video.write(65, 13, 0x1e, " Quit");
		video.write(62, 16, 0x30, " A ");
		video.write(65, 16, 0x1f, " About ZZT!");
		video.write(62, 17, 0x70, " H ");
		video.write(65, 17, 0x1e, " High Scores");

		if (EditorEnabled)  {
			video.write(62, 18, 0x30, " E ");
			video.write(65, 18, 0x1e, " Board Editor");
		}
	}
}

void GamePlayLoop(boolean boardChanged) {
	boolean exitLoop;
	boolean pauseBlink;

	GameDrawSidebar();
	GameUpdateSidebar();

	if (JustStarted)  {
		GameAboutScreen();
		if (length(StartupWorldFileName) != 0)  {
			SidebarClearLine(8);
			video.write(69, 8, 0x1f, StartupWorldFileName);
			if (! WorldLoad(std::string(StartupWorldFileName),
					".ZZT"))  {
				WorldCreate();
			}
		}
		ReturnBoardId = game_world->Info.CurrentBoardIdx;
		BoardChange(0);
		JustStarted = false;
	}

	game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X]
	[game_world->currentBoard.Stats[0].Y].Element = GameStateElement;
	game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X]
	[game_world->currentBoard.Stats[0].Y].Color =
		elem_info_ptr->defs[GameStateElement].Color;

	if (GameStateElement == E_MONITOR)  {
		DisplayMessage(0, "");
		video.write(62, 5, 0x1b, "Pick a command:");
	}

	if (boardChanged) {
		TransitionDrawBoardChange();
	}

	TickTimeDuration = TickSpeed * 2;
	GamePlayExitRequested = false;
	exitLoop = false;

	// if (!FuzzMode) {
	CurrentTick = rnd.randint(100);
	CurrentStatTicked = game_world->currentBoard.StatCount + 1;
	// }

	pauseBlink = true;

	int i = 0;

	do {
		if (GamePaused)  {
			if (!set_paused) {
				clock_gettime(CLOCK_MONOTONIC, &paused);
				set_paused = true;
			}

			if (SoundHasTimeElapsed(TickTimeCounter, 25)) {
				pauseBlink = !pauseBlink;
			}

			keyboard.update();

			// Don't blink an out-of-bounds player.
			if (pauseBlink && CoordInsideViewport(game_world->currentBoard.Stats[0].X,
					game_world->currentBoard.Stats[0].Y)) {
				video.write(game_world->currentBoard.Stats[0].X - 1,
					game_world->currentBoard.Stats[0].Y - 1,
					elem_info_ptr->defs[E_PLAYER].Color,
					elem_info_ptr->defs[E_PLAYER].Character);
			} else {
				if (CoordInsideViewport(game_world->currentBoard.Stats[0].X,
						game_world->currentBoard.Stats[0].Y)
					&& game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Element
					==
					E_PLAYER)
					video.write(game_world->currentBoard.Stats[0].X - 1,
						game_world->currentBoard.Stats[0].Y - 1, 0xf,
						" ");
				else {
					BoardDrawTile(game_world->currentBoard.Stats[0].X,
						game_world->currentBoard.Stats[0].Y);
				}
			}

			video.write(64, 5, 0x1f, "Pausing...");

			if (keyboard.InputKeyPressed == E_KEY_ESCAPE) {
				GamePromptEndPlay();
			}

			if (((keyboard.InputDeltaX != 0) || (keyboard.InputDeltaY != 0)) &&
				ValidCoord(game_world->currentBoard.Stats[0].X + keyboard.InputDeltaX,
					game_world->currentBoard.Stats[0].Y + keyboard.InputDeltaY))  {
				ElementProcDefs[game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X
						+
						keyboard.InputDeltaX]
					[game_world->currentBoard.Stats[0].Y +
						keyboard.InputDeltaY].Element].TouchProc(
							game_world->currentBoard.Stats[0].X + keyboard.InputDeltaX,
							game_world->currentBoard.Stats[0].Y + keyboard.InputDeltaY, 0,
							keyboard.InputDeltaX, keyboard.InputDeltaY);
			}

			if (((keyboard.InputDeltaX != 0) || (keyboard.InputDeltaY != 0))
				&& ValidCoord(game_world->currentBoard.Stats[0].X +keyboard.InputDeltaX,
					game_world->currentBoard.Stats[0].Y + keyboard.InputDeltaY)
				&& elem_info_ptr->defs[game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X
						+keyboard.InputDeltaX][game_world->currentBoard.Stats[0].Y
						+ keyboard.InputDeltaY].Element].Walkable) {
				/* Move player */
				if (game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Element
					==
					E_PLAYER)
					MoveStat(0, game_world->currentBoard.Stats[0].X + keyboard.InputDeltaX,
						game_world->currentBoard.Stats[0].Y + keyboard.InputDeltaY);
				else {
					BoardDrawTile(game_world->currentBoard.Stats[0].X,
						game_world->currentBoard.Stats[0].Y);
					game_world->currentBoard.Stats[0].X = game_world->currentBoard.Stats[0].X +
						keyboard.InputDeltaX;
					game_world->currentBoard.Stats[0].Y = game_world->currentBoard.Stats[0].Y +
						keyboard.InputDeltaY;
					game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Element
						= E_PLAYER;
					game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Color
						=
							elem_info_ptr->defs[E_PLAYER].Color;
					BoardDrawTile(game_world->currentBoard.Stats[0].X,
						game_world->currentBoard.Stats[0].Y);
					DrawPlayerSurroundings(game_world->currentBoard.Stats[0].X,
						game_world->currentBoard.Stats[0].Y, 0);
					DrawPlayerSurroundings(game_world->currentBoard.Stats[0].X -
						keyboard.InputDeltaX,
						game_world->currentBoard.Stats[0].Y - keyboard.InputDeltaY, 0);
				}

				/* Unpause */
				set_paused = false;
				clock_gettime(CLOCK_MONOTONIC, &unpaused);
				std::cerr << game_world->Info.Name << ": Player unpaused. Pause duration: ";
				print_timedelta(paused, unpaused, std::cerr);
				std::cerr << "\n";
				GamePaused = false;
				SidebarClearLine(5);
				CurrentTick = rnd.randint(100);
				CurrentStatTicked = game_world->currentBoard.StatCount + 1;
				game_world->Info.IsSave = true;
			}

		} else {       /* not GamePaused */
			if (CurrentStatTicked <= game_world->currentBoard.StatCount)  {
				TStat & with = game_world->currentBoard.Stats[CurrentStatTicked];
				/* IMP: The game element (at stat 0) can always call a tick,
				   but it can only act - affect the world - if the cycle is
				   right. See ElementPlayerTick for more info. */
				if ((CurrentStatTicked == 0) ||
					((with.Cycle != 0)
						&& ((CurrentTick % with.Cycle) == (CurrentStatTicked % with.Cycle))))

				{
					ElementProcDefs[game_world->currentBoard.Tiles[with.X][with.Y].Element].TickProc(
						CurrentStatTicked);
				}

				CurrentStatTicked = CurrentStatTicked + 1;
			}
		}

		if ((CurrentStatTicked > game_world->currentBoard.StatCount)
			&& ! GamePlayExitRequested)  {
			if (SoundHasTimeElapsed(TickTimeCounter, TickTimeDuration))  {
				/* next cycle */
				CurrentTick = CurrentTick + 1;
				if (CurrentTick > MAX_CYCLE) {
					CurrentTick = 1;
				}
				CurrentStatTicked = 0;

				keyboard.update();
				video.redraw();
			}
		}

		/* Crash if the invariant that the player (or monitor) must exist
		    and be at the X,Y given by stat 0 is violated. We have to check
		    for both player and monitor no matter what the GameStateElement
		    is in order to support Chronos' Forced Play hack. */
		if (!ValidCoord(game_world->currentBoard.Stats[0].X,
				game_world->currentBoard.Stats[0].Y)) {
			throw std::logic_error("game.cxx: Player or Monitor is off-board."
				" This should never happen.");
		}

		byte playerTileElem =
			game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Element;
		if (playerTileElem != E_PLAYER && playerTileElem != E_MONITOR) {
			throw std::logic_error("game.cxx: Board has no Player or Monitor."
				" This should never happen.");
		}

	} while (!((exitLoop || GamePlayExitRequested)
			&& GamePlayExitRequested));

	SoundClearQueue();

	if (GameStateElement == E_PLAYER)  {
		// XXX: Once we add in editor.cxx
		// Should high score routines be in editor anyway?
		/*if (game_world->Info.Health <= 0)  {
			HighScoresAdd(game_world->Info.Score);
		}*/
	} else if (GameStateElement == E_MONITOR)  {
		SidebarClearLine(5);
	}

	game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Element
		= E_PLAYER;
	game_world->currentBoard.Tiles[game_world->currentBoard.Stats[0].X][game_world->currentBoard.Stats[0].Y].Color
		=
			elem_info_ptr->defs[E_PLAYER].Color;

	SoundBlockQueueing = false;
}

void GameTitleLoop() {
	boolean boardChanged;
	boolean startPlay;
	boolean wasSaveGame = game_world->Info.IsSave;

	GameTitleExitRequested = false;
	ReturnBoardId = 0;
	boardChanged = true;
	do {
		BoardChange(0);
		do {
			GameStateElement = E_MONITOR;
			startPlay = false;
			GamePaused = false;
			GamePlayLoop(boardChanged);
			boardChanged = false;

			switch (keyUpCase(keyboard.InputKeyPressed)) {
				case 'W': {
					if (GameWorldLoad(".ZZT"))  {
						ReturnBoardId = game_world->Info.CurrentBoardIdx;
						boardChanged = true;
					}
				}
				break;
				case 'P': {
					if (game_world->Info.IsSave && ! DebugEnabled)  {
						startPlay = WorldLoad(game_world->Info.Name, ".ZZT");
						ReturnBoardId = game_world->Info.CurrentBoardIdx;
					} else {
						startPlay = true;
					}
					if (startPlay)  {
						BoardChange(ReturnBoardId);
						BoardEnter();
					}
				}
				break;
				case 'A': {
					GameAboutScreen();
				}
				break;
				case 'E': if (EditorEnabled)  {
						EditorLoop();
						ReturnBoardId = game_world->Info.CurrentBoardIdx;
						boardChanged = true;
					}
					break;
				case 'S': {
					SidebarPromptSlider(true, 66, 21, "Game speed:;FS", TickSpeed, 256);
					keyboard.InputKeyPressed = '\0';
				}
				break;
				case 'R': {
					if (GameWorldLoad(".SAV"))  {
						ReturnBoardId = game_world->Info.CurrentBoardIdx;
						BoardChange(ReturnBoardId);
						startPlay = true;
					}
				}
				break;
					// XXX: Once we add in editor.cxx
					/*case 'H': {
						HighScoresLoad();
						HighScoresDisplay(1);
					}*/
				break;
				case '|': {
					GameDebugPrompt();
				}
				break;
				case E_KEY_ESCAPE: case 'Q': {
					GameTitleExitRequested = SidebarPromptYesNo("Quit ZZT? ", true);
				}
				break;
			}

			if (startPlay)  {
				GameStateElement = E_PLAYER;
				clock_gettime(CLOCK_MONOTONIC, &paused);
				GamePaused = true;
				GamePlayLoop(true);
				boardChanged = true;
			}
		} while (!(boardChanged || GameTitleExitRequested));
	} while (!GameTitleExitRequested);

	/* IMP: When we stop playing, the world is no longer a save game
	   unless it was loaded as one. */
	game_world->Info.IsSave = wasSaveGame;
}

void GamePrintRegisterMessage() {
	std::string s;
	std::ifstream f;
	integer i;
	integer ix, iy;
	integer color;
	boolean isReading;
	string * strPtr;

//	SetCBreak(false);
	s = "END" + itos(49 + rnd.randint(4)) + ".MSG";
	iy = 0;
	color = 0xf;
	word actuallyRead;

	// TODO: Fix resource file reading.

	for (i = 1; i <= ResourceDataHeader.EntryCount; i ++) {
		if (s == ResourceDataHeader.Name[i])  {
			f = OpenForRead(ResourceDataFileName);
			f.seekg(ResourceDataHeader.FileOffset[i]); // * record length?

			isReading = true;
			while (errno == 0 && isReading)  {
				char scratch[256];
				f.read(scratch, 1); // is this also in record terms?
				s = scratch;
				if (s.size() == 0)  {
					color -= 1;
				} else {
					f.read(scratch+1, s.size());
					s = scratch;
					if (s[0] != '@') {
						video.write(0, iy, color, s);
					} else {
						isReading = false;
					}
				}
				iy += 1;
			}

			f.close();
			video.write(28, 24, 0x1f, "Press any key to exit...");
			video.TextColor(LightGray);

			keyboard.wait_for_key();

			video.write(28, 24, 0, "                        ");
			video.go_to_xy(0, 22);
		}
	}
}

class unit_Game_initialize {
	public: unit_Game_initialize();
};
static unit_Game_initialize Game_constructor;

unit_Game_initialize::unit_Game_initialize() {
	JustStarted = true;
}
