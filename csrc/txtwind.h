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
        array<1 , MAX_TEXT_WINDOW_LINES,TTextWindowLine*> Lines;
        asciiz Hyperlink;
        TTextWindowLine Title;
        asciiz LoadedFilename;
};
struct TResourceDataHeader {
        integer EntryCount;
        array<1 , MAX_RESOURCE_DATA_FILES,asciiz> Name;
        array<1 , MAX_RESOURCE_DATA_FILES,longint> FileOffset;
};

#ifdef __TxtWind_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN TVideoBuffer ScreenCopy;
EXTERN integer TextWindowX, TextWindowY;
EXTERN integer TextWindowWidth, TextWindowHeight;
EXTERN TVideoLine TextWindowStrInnerEmpty;
EXTERN TVideoLine TextWindowStrText;
EXTERN TVideoLine TextWindowStrInnerLine;
EXTERN TVideoLine TextWindowStrTop;
EXTERN TVideoLine TextWindowStrBottom;
EXTERN TVideoLine TextWindowStrSep;
EXTERN TVideoLine TextWindowStrInnerSep;
EXTERN TVideoLine TextWindowStrInnerArrows;
EXTERN boolean TextWindowRejected;
EXTERN varying_string<50> ResourceDataFileName;
EXTERN TResourceDataHeader ResourceDataHeader;
EXTERN string* OrderPrintId;
#undef EXTERN
#define EXTERN extern

        void TextWindowInitState(TTextWindowState& state);
        void TextWindowDrawOpen(TTextWindowState& state);
        void TextWindowDrawClose(TTextWindowState& state);
        void TextWindowDraw(TTextWindowState& state, boolean withoutFormatting, boolean viewingFile);
        void TextWindowAppend(TTextWindowState& state, TTextWindowLine line);
        void TextWindowFree(TTextWindowState& state);
        void TextWindowSelect(TTextWindowState& state, boolean hyperlinkAsSelect, boolean viewingFile);
        void TextWindowSort(TTextWindowState& state);
        void TextWindowEdit(TTextWindowState& state);
        void TextWindowOpenFile(TTextWindowLine filename, TTextWindowState& state);
        void TextWindowSaveFile(TTextWindowLine filename, TTextWindowState& state);
        void TextWindowDisplayFile(string filename, string title);
        void TextWindowInit(integer x, integer y, integer width, integer height);

#endif
