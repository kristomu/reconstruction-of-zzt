#ifndef __txtwind_h__
#define __txtwind_h__

#include "video.h"

const integer MAX_TEXT_WINDOW_LINES = 1024;
const integer MAX_RESOURCE_DATA_FILES = 24;
typedef varying_string<50> TTextWindowLine;

struct TTextWindowState {
	boolean Selectable;
	integer LineCount;
	integer LinePos;
	array<1, MAX_TEXT_WINDOW_LINES,TTextWindowLine*> Lines;
	string Hyperlink;
	TTextWindowLine Title;
	string LoadedFilename;
};

struct TResourceDataHeader {
	integer EntryCount;
	std::array<std::string, MAX_RESOURCE_DATA_FILES> Name;
	std::array<int, MAX_RESOURCE_DATA_FILES> FileOffset;
};

#ifdef __TxtWind_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN integer TextWindowX, TextWindowY;
EXTERN integer TextWindowWidth, TextWindowHeight;
EXTERN video_line text_window_str_inner_empty;
EXTERN video_line text_window_str_text;
EXTERN video_line text_window_str_inner_line;
EXTERN video_line text_window_str_top;
EXTERN video_line text_window_str_bottom;
EXTERN video_line text_window_str_sep;
EXTERN video_line text_window_str_inner_sep;
EXTERN video_line text_window_str_inner_arrows;
EXTERN bool TextWindowRejected;
EXTERN std::string ResourceDataFileName;
EXTERN TResourceDataHeader ResourceDataHeader;
EXTERN string* OrderPrintId;
#undef EXTERN
#define EXTERN extern

void TextWindowInitState(TTextWindowState& state);
void TextWindowDrawOpen(TTextWindowState& state);
void TextWindowDrawClose(TTextWindowState& state);
void TextWindowDraw(TTextWindowState& state, boolean withoutFormatting,
                    boolean viewingFile);
void TextWindowAppend(TTextWindowState& state, TTextWindowLine line);
void TextWindowFree(TTextWindowState& state);
void TextWindowSelect(TTextWindowState& state, boolean hyperlinkAsSelect,
                      boolean viewingFile);
void TextWindowSort(TTextWindowState& state);
void TextWindowEdit(TTextWindowState& state);
void TextWindowOpenFile(std::string filename, TTextWindowState& state);
void TextWindowSaveFile(TTextWindowLine filename, TTextWindowState& state);
void TextWindowDisplayFile(std::string filename, string title);
void TextWindowInit(integer x, integer y, integer width, integer height);

#endif
