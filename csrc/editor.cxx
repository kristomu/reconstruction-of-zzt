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
/*$V-*/
#define __Editor_implementation__


#include "editor.h"

/*#include "Dos.h"*/
/*#include "Crt.h"*/
#include "video.h"
#include "sounds.h"
#include "input.h"
#include "elements.h"
#include "oop.h"
#include "game.h"
#include "fileops.h"
#include "minmax.h"


enum TDrawMode {DrawingOff, DrawingOn, TextEntry, last_TDrawMode};
const array<0, 3,varying_string<20> > NeighborBoardStrs =
{{"       Board \30", "       Board \31", "       Board \33", "       Board \32"}};

void EditorAppendBoard() {
    if (World.BoardCount < MAX_BOARD)  {
        BoardClose(true);

        World.BoardCount = World.BoardCount + 1;
        World.Info.CurrentBoard = World.BoardCount;
        World.BoardLen[World.BoardCount] = 0;
        BoardCreate();

        TransitionDrawToBoard();

        do {
            PopupPromptString("Room\47s Title:", Board.Name);
        } while (!(length(Board.Name) != 0));

        TransitionDrawToBoard();
    }
}

void EditorLoop();

static boolean wasModified;

static TDrawMode drawMode;

static integer cursorX, cursorY;

static integer cursorPattern, cursorColor;

static TStat copiedStat;

static boolean copiedHasStat;

static byte* copiedData;

static integer copiedDataLen;

static TTile copiedTile;

static integer copiedX, copiedY;


static void EditorDrawSidebar() {
    integer i;
    byte copiedChr;

    SidebarClear();
    SidebarClearLine(1);
    VideoWriteText(61, 0, 0x1f, "     - - - -       ");
    VideoWriteText(62, 1, 0x70, "  ZZT Editor   ");
    VideoWriteText(61, 2, 0x1f, "     - - - -       ");
    VideoWriteText(61, 4, 0x70, " L ");
    VideoWriteText(64, 4, 0x1f, " Load");
    VideoWriteText(61, 5, 0x30, " S ");
    VideoWriteText(64, 5, 0x1f, " Save");
    VideoWriteText(70, 4, 0x70, " H ");
    VideoWriteText(73, 4, 0x1e, " Help");
    VideoWriteText(70, 5, 0x30, " Q ");
    VideoWriteText(73, 5, 0x1f, " Quit");
    VideoWriteText(61, 7, 0x70, " B ");
    VideoWriteText(65, 7, 0x1f, " Switch boards");
    VideoWriteText(61, 8, 0x30, " I ");
    VideoWriteText(65, 8, 0x1f, " Board Info");
    VideoWriteText(61, 10, 0x70, "  f1   ");
    VideoWriteText(68, 10, 0x1f, " Item");
    VideoWriteText(61, 11, 0x30, "  f2   ");
    VideoWriteText(68, 11, 0x1f, " Creature");
    VideoWriteText(61, 12, 0x70, "  f3   ");
    VideoWriteText(68, 12, 0x1f, " Terrain");
    VideoWriteText(61, 13, 0x30, "  f4   ");
    VideoWriteText(68, 13, 0x1f, " Enter text");
    VideoWriteText(61, 15, 0x70, " Space ");
    VideoWriteText(68, 15, 0x1f, " Plot");
    VideoWriteText(61, 16, 0x30, "  Tab  ");
    VideoWriteText(68, 16, 0x1f, " Draw mode");
    VideoWriteText(61, 18, 0x70, " P ");
    VideoWriteText(64, 18, 0x1f, " Pattern");
    VideoWriteText(61, 19, 0x30, " C ");
    VideoWriteText(64, 19, 0x1f, " Color:");

    /* Colors */
    for( i = 9; i <= 15; i ++)
        VideoWriteText(61 + i, 22, i, "\333");

    /* Patterns */
    for( i = 1; i <= EditorPatternCount; i ++)
        VideoWriteText(61 + i, 22, 0xf, ElementDefs[EditorPatterns[i]].Character);

    if (ElementDefs[copiedTile.Element].HasDrawProc)
        ElementDefs[copiedTile.Element].DrawProc(copiedX, copiedY, copiedChr);
    else
        copiedChr = ord(ElementDefs[copiedTile.Element].Character);
    VideoWriteText(62 + EditorPatternCount, 22, copiedTile.Color,
                   chr(copiedChr));

    VideoWriteText(61, 24, 0x1f, " Mode:");
}



static void EditorDrawTileAndNeighborsAt(integer x, integer y) {
    integer i, ix, iy;

    BoardDrawTile(x, y);
    for( i = 0; i <= 3; i ++) {
        ix = x + NeighborDeltaX[i];
        iy = y + NeighborDeltaY[i];
        if ((ix >= 1) && (ix <= BOARD_WIDTH) && (iy >= 1) && (iy <= BOARD_HEIGHT))
            BoardDrawTile(ix, iy);
    }
}



static void EditorUpdateSidebar() {
    if (drawMode == DrawingOn)
        VideoWriteText(68, 24, 0x9e, "Drawing on ");
    else if (drawMode == TextEntry)
        VideoWriteText(68, 24, 0x9e, "Text entry ");
    else if (drawMode == DrawingOff)
        VideoWriteText(68, 24, 0x1e, "Drawing off");

    VideoWriteText(72, 19, 0x1e, ColorNames[cursorColor - 8]);
    VideoWriteText(61 + cursorPattern, 21, 0x1f, "\37");
    VideoWriteText(61 + cursorColor, 21, 0x1f, "\37");
}



static void EditorDrawRefresh() {
    string boardNumStr;

    BoardDrawBorder();
    EditorDrawSidebar();
    str(World.Info.CurrentBoard, boardNumStr);
    TransitionDrawToBoard();

    if (length(Board.Name) != 0)
        VideoWriteText((59 - length(Board.Name)) / 2, 0, 0x70,
                       string(' ') + Board.Name + ' ');
    else
        VideoWriteText(26, 0, 0x70, " Untitled ");
}



static void EditorSetAndCopyTile(byte x, byte y, byte element,
                                 byte color) {
    Board.Tiles[x][y].Element = element;
    Board.Tiles[x][y].Color = color;

    copiedTile = Board.Tiles[x][y];
    copiedHasStat = false;
    copiedX = x;
    copiedY = y;

    EditorDrawTileAndNeighborsAt(x, y);
}



static void EditorAskSaveChanged() {
    InputKeyPressed = '\0';
    if (wasModified)
        if (SidebarPromptYesNo("Save first? ", true))
            if (InputKeyPressed != KEY_ESCAPE)
                GameWorldSave("Save world", LoadedGameFileName, ".ZZT");
    World.Info.Name = LoadedGameFileName;
}



static boolean EditorPrepareModifyTile(integer x, integer y) {
    boolean EditorPrepareModifyTile_result;
    wasModified = true;
    EditorPrepareModifyTile_result = BoardPrepareTileForPlacement(x, y);
    EditorDrawTileAndNeighborsAt(x, y);
    return EditorPrepareModifyTile_result;
}



static boolean EditorPrepareModifyStatAtCursor() {
    boolean EditorPrepareModifyStatAtCursor_result;
    if (Board.StatCount < MAX_STAT)
        EditorPrepareModifyStatAtCursor_result = EditorPrepareModifyTile(cursorX,
                cursorY);
    else
        EditorPrepareModifyStatAtCursor_result = false;
    return EditorPrepareModifyStatAtCursor_result;
}



static void EditorPlaceTile(integer x, integer y) {
    {
        TTile& with = Board.Tiles[x][y];
        if (cursorPattern <= EditorPatternCount)  {
            if (EditorPrepareModifyTile(x, y))  {
                with.Element = EditorPatterns[cursorPattern];
                with.Color = cursorColor;
            }
        } else if (copiedHasStat)  {
            if (EditorPrepareModifyStatAtCursor())  {
                /* AddStat automatically allocates new pointers,
                for any potential data, so no need to do that here.
                				 Instead, we can just copy the pointer over. */
                copiedStat.Data = copiedData;
                AddStat(x, y, copiedTile.Element, copiedTile.Color, copiedStat.Cycle,
                        copiedStat);
            }
        } else {
            if (EditorPrepareModifyTile(x, y))  {
                Board.Tiles[x][y] = copiedTile;
            }
        }

        EditorDrawTileAndNeighborsAt(x, y);
    }
}

static void EditorEditBoardInfo();


static string BoolToString(boolean val) {
    string BoolToString_result;
    if (val)
        BoolToString_result = "Yes";
    else
        BoolToString_result = "No ";
    return BoolToString_result;
}



static void EditorEditBoardInfo() {
    TTextWindowState state;
    integer i;
    varying_string<50> numStr;
    boolean exitRequested;


    state.Title = "Board Information";
    TextWindowDrawOpen(state);
    state.LinePos = 1;
    state.LineCount = 9;
    state.Selectable = true;
    exitRequested = false;
    for( i = 1; i <= state.LineCount; i ++)
        state.Lines[i] = new TTextWindowLine;

    do {
        state.Selectable = true;
        state.LineCount = 10;
        for( i = 1; i <= state.LineCount; i ++)
            state.Lines[i] = new TTextWindowLine;

        *state.Lines[1] = string("         Title: ") + Board.Name;

        str(Board.Info.MaxShots, numStr);
        *state.Lines[2] = string("      Can fire: ") + numStr + " shots.";

        *state.Lines[3] = string(" Board is dark: ") + BoolToString(
                              Board.Info.IsDark);

        for( i = 4; i <= 7; i ++) {
            *state.Lines[i] = NeighborBoardStrs[i - 4] + ": " +
                              EditorGetBoardName(Board.Info.NeighborBoards[i - 4], true);
        }

        *state.Lines[8] = string("Re-enter when zapped: ") + BoolToString(
                              Board.Info.ReenterWhenZapped);

        str(Board.Info.TimeLimitSec, numStr);
        *state.Lines[9] = string("  Time limit, 0=None: ") + numStr + " sec.";

        *state.Lines[10] = "          Quit!";

        TextWindowSelect(state, false, false);
        if ((InputKeyPressed == KEY_ENTER) && (state.LinePos >= 1)
                && (state.LinePos <= 8))
            wasModified = true;
        if (InputKeyPressed == KEY_ENTER)
            switch (state.LinePos) {
            case 1: {
                PopupPromptString("New title for board:", Board.Name);
                exitRequested = true;
                TextWindowDrawClose(state);
            }
            break;
            case 2: {
                str(Board.Info.MaxShots, numStr);
                SidebarPromptString("Maximum shots?", "", numStr, PROMPT_NUMERIC);
                if (length(numStr) != 0)
                    val(numStr, Board.Info.MaxShots, i);
                EditorDrawSidebar();
            }
            break;
            case 3: {
                Board.Info.IsDark = ! Board.Info.IsDark;
            }
            break;
            case 4: case 5: case 6: case 7: {
                Board.Info.NeighborBoards[state.LinePos - 4]
                    = EditorSelectBoard(
                          NeighborBoardStrs[state.LinePos - 4],
                          Board.Info.NeighborBoards[state.LinePos - 4],
                          true
                      );
                if (Board.Info.NeighborBoards[state.LinePos - 4] > World.BoardCount)
                    EditorAppendBoard();
                exitRequested = true;
            }
            break;
            case 8: {
                Board.Info.ReenterWhenZapped = ! Board.Info.ReenterWhenZapped;
            }
            break;
            case 9: {
                str(Board.Info.TimeLimitSec, numStr);
                SidebarPromptString("Time limit?", " Sec", numStr, PROMPT_NUMERIC);
                if (length(numStr) != 0)
                    val(numStr, Board.Info.TimeLimitSec, i);
                EditorDrawSidebar();
            }
            break;
            case 10: {
                exitRequested = true;
                TextWindowDrawClose(state);
            }
            break;
            } else {
            exitRequested = true;
            TextWindowDrawClose(state);
        }
    } while (!exitRequested);

    TextWindowFree(state);
}



static void EditorEditStatText(integer statId, string prompt) {
    TTextWindowState state;
    integer iLine, iChar;
    array<0, 51,byte> unk1;
    char dataChar;
    pointer dataPtr;

    {
        TStat& with = Board.Stats[statId];
        state.Title = prompt;
        TextWindowDrawOpen(state);
        state.Selectable = false;
        CopyStatDataToTextWindow(statId, state);

        if (with.DataLen > 0)  {
            FreeMem(with.Data, with.DataLen);
            with.DataLen = 0;
        }

        EditorOpenEditTextWindow(state);

        for( iLine = 1; iLine <= state.LineCount; iLine ++)
            with.DataLen = with.DataLen + length(*state.Lines[iLine]) + 1;
        GetMem(with.Data, with.DataLen);

        dataPtr = with.Data;
        for( iLine = 1; iLine <= state.LineCount; iLine ++) {
            for( iChar = 1; iChar <= length(*state.Lines[iLine]); iChar ++) {
                dataChar = (*state.Lines[iLine])[iChar];
                Move(dataChar, dataPtr, 1);
                AdvancePointer(dataPtr, 1);
            }

            dataChar = '\15';
            Move(dataChar, dataPtr, 1);
            AdvancePointer(dataPtr, 1);
        }

        TextWindowFree(state);
        TextWindowDrawClose(state);
        InputKeyPressed = '\0';
    }
}

static void EditorEditStat(integer statId);


static void EditorEditStatSettings(boolean selected, integer& statId,
                                   integer& iy, byte& element, byte& promptByte, byte& selectedBoard) {
    {
        TStat& with = Board.Stats[statId];
        InputKeyPressed = '\0';
        iy = 9;

        if (length(ElementDefs[element].Param1Name) != 0)  {
            if (length(ElementDefs[element].ParamTextName) == 0)  {
                SidebarPromptSlider(selected, 63, iy, ElementDefs[element].Param1Name,
                                    with.P1);
            } else {
                if (with.P1 == 0)
                    with.P1 = World.EditorStatSettings[element].P1;
                BoardDrawTile(with.X, with.Y);
                SidebarPromptCharacter(selected, 63, iy, ElementDefs[element].Param1Name,
                                       with.P1);
                BoardDrawTile(with.X, with.Y);
            }
            if (selected)
                World.EditorStatSettings[element].P1 = with.P1;
            iy = iy + 4;
        }

        if ((InputKeyPressed != KEY_ESCAPE) &&
                (length(ElementDefs[element].ParamTextName) != 0)) {
            if (selected)
                EditorEditStatText(statId, ElementDefs[element].ParamTextName);
        }

        if ((InputKeyPressed != KEY_ESCAPE) &&
                (length(ElementDefs[element].Param2Name) != 0)) {
            promptByte = (with.P2 % 0x80);
            SidebarPromptSlider(selected, 63, iy, ElementDefs[element].Param2Name,
                                promptByte);
            if (selected)  {
                with.P2 = (with.P2 & 0x80) + promptByte;
                World.EditorStatSettings[element].P2 = with.P2;
            }
            iy = iy + 4;
        }

        if ((InputKeyPressed != KEY_ESCAPE) &&
                (length(ElementDefs[element].ParamBulletTypeName) != 0)) {
            promptByte = (with.P2) / 0x80;
            SidebarPromptChoice(selected, iy, ElementDefs[element].ParamBulletTypeName,
                                "Bullets Stars", promptByte);
            if (selected)  {
                with.P2 = (with.P2 % 0x80) + (promptByte * 0x80);
                World.EditorStatSettings[element].P2 = with.P2;
            }
            iy = iy + 4;
        }

        if ((InputKeyPressed != KEY_ESCAPE) &&
                (length(ElementDefs[element].ParamDirName) != 0)) {
            SidebarPromptDirection(selected, iy, ElementDefs[element].ParamDirName,
                                   with.StepX, with.StepY);
            if (selected)  {
                World.EditorStatSettings[element].StepX = with.StepX;
                World.EditorStatSettings[element].StepY = with.StepY;
            }
            iy = iy + 4;
        }

        if ((InputKeyPressed != KEY_ESCAPE) &&
                (length(ElementDefs[element].ParamBoardName) != 0)) {
            if (selected)  {
                selectedBoard = EditorSelectBoard(ElementDefs[element].ParamBoardName,
                                                  with.P3, true);
                if (selectedBoard != 0)  {
                    with.P3 = selectedBoard;
                    World.EditorStatSettings[element].P3 = World.Info.CurrentBoard;
                    if (with.P3 > World.BoardCount)  {
                        EditorAppendBoard();
                        copiedHasStat = false;
                        copiedTile.Element = 0;
                        copiedTile.Color = 0xf;
                    }
                    World.EditorStatSettings[element].P3 = with.P3;
                } else {
                    InputKeyPressed = KEY_ESCAPE;
                }
                iy = iy + 4;
            } else {
                VideoWriteText(63, iy, 0x1f,
                               string("Room: ") + copy(EditorGetBoardName(with.P3, true), 1, 10));
            }
        }
    }
}



static void EditorEditStat(integer statId) {
    byte element;
    integer i;
    string categoryName;
    byte selectedBoard;
    integer iy;
    byte promptByte;


    {
        TStat& with = Board.Stats[statId];
        SidebarClear();

        element = Board.Tiles[with.X][with.Y].Element;
        wasModified = true;

        categoryName = "";
        for( i = 0; i <= element; i ++) {
            if ((ElementDefs[i].EditorCategory == ElementDefs[element].EditorCategory)
                    && (length(ElementDefs[i].CategoryName) != 0)) {
                categoryName = ElementDefs[i].CategoryName;
            }
        }

        VideoWriteText(64, 6, 0x1e, categoryName);
        VideoWriteText(64, 7, 0x1f, ElementDefs[element].Name);

        EditorEditStatSettings(false, statId, iy, element, promptByte,
                               selectedBoard);
        EditorEditStatSettings(true, statId, iy, element, promptByte,
                               selectedBoard);

        if (InputKeyPressed != KEY_ESCAPE)  {
            copiedHasStat = true;
            copiedStat = Board.Stats[statId];
            copiedTile = Board.Tiles[with.X][with.Y];

            /* Copy data into temporary store if the tile has any,
            so the object can be moved between boards. */

            if (copiedDataLen > 0)  {
                FreeMem(copiedData, copiedDataLen);
                copiedDataLen = 0;
            }
            if (with.DataLen > 0)  {
                GetMem(copiedData, with.DataLen);
                copiedDataLen = with.DataLen;
                Move(*with.Data, *copiedData, copiedDataLen);
            }
            copiedX = with.X;
            copiedY = with.Y;
        }
    }
}



static void EditorTransferBoard() {
    byte i;
    untyped_file f;


    i = 1;
    SidebarPromptChoice(true, 3, "Transfer board:", "Import Export", i);
    if (InputKeyPressed != KEY_ESCAPE)  {
        if (i == 0)  {
            SidebarPromptString("Import board", ".BRD", SavedBoardFileName,
                                PROMPT_ALPHANUM);
            if ((InputKeyPressed != KEY_ESCAPE)
                    && (length(SavedBoardFileName) != 0))  {
                assign(f, SavedBoardFileName + ".BRD");
                OpenForRead(f, 1);
                if (DisplayIOError())  goto LTransferEnd;

                /* The old board is toast; no need to warn about
                data loss. */
                BoardClose(false);
                FreeMem(World.BoardData[World.Info.CurrentBoard],
                        World.BoardLen[World.Info.CurrentBoard]);
                BlockRead(f, World.BoardLen[World.Info.CurrentBoard], 2);
                if (! DisplayIOError())  {
                    GetMem(World.BoardData[World.Info.CurrentBoard],
                           World.BoardLen[World.Info.CurrentBoard]);
                    BlockRead(f, World.BoardData[World.Info.CurrentBoard],
                              World.BoardLen[World.Info.CurrentBoard]);
                }

                if (DisplayIOError())  {
                    World.BoardLen[World.Info.CurrentBoard] = 0;
                    BoardCreate();
                    EditorDrawRefresh();
                } else {
                    BoardOpen(World.Info.CurrentBoard, false);
                    EditorDrawRefresh();
                    for( i = 0; i <= 3; i ++)
                        Board.Info.NeighborBoards[i] = 0;
                }
            }
        } else if (i == 1)  {
            SidebarPromptString("Export board", ".BRD", SavedBoardFileName,
                                PROMPT_ALPHANUM);
            if ((InputKeyPressed != KEY_ESCAPE)
                    && (length(SavedBoardFileName) != 0))  {
                assign(f, SavedBoardFileName + ".BRD");
                OpenForWrite(f, 1);
                if (DisplayIOError())  goto LTransferEnd;

                BoardClose(true);
                BlockWrite(f, World.BoardLen[World.Info.CurrentBoard], 2);
                BlockWrite(f, World.BoardData[World.Info.CurrentBoard],
                           World.BoardLen[World.Info.CurrentBoard]);
                BoardOpen(World.Info.CurrentBoard, false);

                if (DisplayIOError())  {
                    ;
                } else {
                    close(f);
                }
            }
        }
    }
LTransferEnd:
    EditorDrawSidebar();
}



static void EditorFloodFill(integer x, integer y, TTile from) {
    integer i;
    TTile tileAt;
    byte toFill, filled;
    array<0, 255,integer> xPosition;
    array<0, 255,integer> yPosition;

    toFill = 1;
    filled = 0;
    while (toFill != filled)  {
        tileAt = Board.Tiles[x][y];
        EditorPlaceTile(x, y);
        if ((Board.Tiles[x][y].Element != tileAt.Element)
                || (Board.Tiles[x][y].Color != tileAt.Color))
            for( i = 0; i <= 3; i ++) {
                TTile& with = Board.Tiles[x + NeighborDeltaX[i]][y + NeighborDeltaY[i]];
                if ((with.Element == from.Element)
                        && ((from.Element == 0) || (with.Color == from.Color))) {
                    xPosition[toFill] = x + NeighborDeltaX[i];
                    yPosition[toFill] = y + NeighborDeltaY[i];
                    toFill = toFill + 1;
                }
            }

        filled = filled + 1;
        x = xPosition[filled];
        y = yPosition[filled];
    }
}

void EditorLoop() {
    integer selectedCategory;
    integer elemMenuColor;
    boolean editorExitRequested;
    integer i, iElem;
    boolean canModify;
    array<0, 49,byte> unk1;
    integer cursorBlinker;


    if (World.Info.IsSave || (WorldGetFlagPosition("SECRET") >= 0))  {
        WorldUnload();
        WorldCreate();
    }
    InitElementsEditor();
    CurrentTick = 0;
    wasModified = false;
    cursorX = 30;
    cursorY = 12;
    drawMode = DrawingOff;
    cursorPattern = 1;
    cursorColor = 0xe;
    cursorBlinker = 0;
    copiedHasStat = false;
    copiedTile.Element = 0;
    copiedDataLen = 0;
    copiedTile.Color = 0xf;

    if (World.Info.CurrentBoard != 0)
        BoardChange(World.Info.CurrentBoard);

    EditorDrawRefresh();
    if (World.BoardCount == 0)
        EditorAppendBoard();

    editorExitRequested = false;
    do {
        if (drawMode == DrawingOn)
            EditorPlaceTile(cursorX, cursorY);
        InputUpdate();
        if ((InputKeyPressed == '\0') && (InputDeltaX == 0) && (InputDeltaY == 0)
                && ! InputShiftPressed)  {
            if (SoundHasTimeElapsed(TickTimeCounter, 15))
                cursorBlinker = (cursorBlinker + 1) % 3;
            if (cursorBlinker == 0)
                BoardDrawTile(cursorX, cursorY);
            else
                VideoWriteText(cursorX - 1, cursorY - 1, 0xf, "\305");
            EditorUpdateSidebar();
        } else {
            BoardDrawTile(cursorX, cursorY);
        }

        if (drawMode == TextEntry)  {
            if ((InputKeyPressed >= '\40') && (InputKeyPressed < '\200'))  {
                if (EditorPrepareModifyTile(cursorX, cursorY))  {
                    Board.Tiles[cursorX][cursorY].Element = (cursorColor - 9) + E_TEXT_MIN;
                    Board.Tiles[cursorX][cursorY].Color = ord(InputKeyPressed);
                    EditorDrawTileAndNeighborsAt(cursorX, cursorY);
                    InputDeltaX = 1;
                    InputDeltaY = 0;
                }
                InputKeyPressed = '\0';
            } else if ((InputKeyPressed == KEY_BACKSPACE) && (cursorX > 1)
                       && EditorPrepareModifyTile(cursorX - 1, cursorY)) {
                cursorX = cursorX - 1;
            } else if ((InputKeyPressed == KEY_ENTER)
                       || (InputKeyPressed == KEY_ESCAPE))  {
                drawMode = DrawingOff;
                InputKeyPressed = '\0';
            }
        }

        {
            TTile& with = Board.Tiles[cursorX][cursorY];
            if (InputShiftPressed || (InputKeyPressed == ' '))  {
                InputShiftAccepted = true;
                if ((with.Element == 0)
                        || (ElementDefs[with.Element].PlaceableOnTop && copiedHasStat
                            && (cursorPattern > EditorPatternCount))
                        || (InputDeltaX != 0) || (InputDeltaY != 0)) {
                    EditorPlaceTile(cursorX, cursorY);
                } else {
                    canModify = EditorPrepareModifyTile(cursorX, cursorY);
                    if (canModify)
                        Board.Tiles[cursorX][cursorY].Element = 0;
                }
            }

            if ((InputDeltaX != 0) || (InputDeltaY != 0))  {
                cursorX = cursorX + InputDeltaX;
                if (cursorX < 1)
                    cursorX = 1;
                if (cursorX > BOARD_WIDTH)
                    cursorX = BOARD_WIDTH;

                cursorY = cursorY + InputDeltaY;
                if (cursorY < 1)
                    cursorY = 1;
                if (cursorY > BOARD_HEIGHT)
                    cursorY = BOARD_HEIGHT;

                VideoWriteText(cursorX - 1, cursorY - 1, 0xf, "\305");
                if ((InputKeyPressed == '\0') && InputJoystickEnabled)
                    Delay(70);
                InputShiftAccepted = false;
            }

            switch (upcase(InputKeyPressed)) {
            case '`': EditorDrawRefresh(); break;
            case 'P': {
                VideoWriteText(62, 21, 0x1f, "       ");
                if (cursorPattern <= EditorPatternCount)
                    cursorPattern = cursorPattern + 1;
                else
                    cursorPattern = 1;
            }
            break;
            case 'C': {
                VideoWriteText(72, 19, 0x1e, "       ");
                VideoWriteText(69, 21, 0x1f, "        ");
                if ((cursorColor % 0x10) != 0xf)
                    cursorColor = cursorColor + 1;
                else
                    cursorColor = ((cursorColor / 0x10) * 0x10) + 9;
            }
            break;
            case 'L': {
                EditorAskSaveChanged();
                if ((InputKeyPressed != KEY_ESCAPE) && GameWorldLoad(".ZZT"))  {
                    if (World.Info.IsSave || (WorldGetFlagPosition("SECRET") >= 0))  {
                        if (! DebugEnabled)  {
                            SidebarClearLine(3);
                            SidebarClearLine(4);
                            SidebarClearLine(5);
                            VideoWriteText(63, 4, 0x1e, "Can not edit");
                            if (World.Info.IsSave)
                                VideoWriteText(63, 5, 0x1e, "a saved game!");
                            else
                                VideoWriteText(63, 5, 0x1e, string("  ") + World.Info.Name + '!');
                            PauseOnError();
                            WorldUnload();
                            WorldCreate();
                        }
                    }
                    wasModified = false;
                    EditorDrawRefresh();
                }
                EditorDrawSidebar();
            }
            break;
            case 'S': {
                GameWorldSave("Save world:", LoadedGameFileName, ".ZZT");
                if (InputKeyPressed != KEY_ESCAPE)
                    wasModified = false;
                EditorDrawSidebar();
            }
            break;
            case 'Z': {
                if (SidebarPromptYesNo("Clear board? ", false))  {
                    for( i = Board.StatCount; i >= 1; i --)
                        RemoveStat(i);
                    BoardCreate();
                    EditorDrawRefresh();
                } else {
                    EditorDrawSidebar();
                }
            }
            break;
            case 'N': {
                if (SidebarPromptYesNo("Make new world? ", false)
                        && (InputKeyPressed != KEY_ESCAPE))  {
                    EditorAskSaveChanged();
                    if (InputKeyPressed != KEY_ESCAPE)  {
                        WorldUnload();
                        WorldCreate();
                        EditorDrawRefresh();
                        wasModified = false;
                    }
                }
                EditorDrawSidebar();
            }
            break;
            case 'Q': case KEY_ESCAPE: {
                editorExitRequested = true;
            }
            break;
            case 'B': {
                i = EditorSelectBoard("Switch boards", World.Info.CurrentBoard, false);
                if (InputKeyPressed != KEY_ESCAPE)  {
                    if (i > World.BoardCount)
                        if (SidebarPromptYesNo("Add new board? ", false))
                            EditorAppendBoard();
                    BoardChange(i);
                    EditorDrawRefresh();
                }
                EditorDrawSidebar();
            }
            break;
            case '?': {
                GameDebugPrompt();
                EditorDrawSidebar();
            }
            break;
            case KEY_TAB: {
                if (drawMode == DrawingOff)
                    drawMode = DrawingOn;
                else
                    drawMode = DrawingOff;
            }
            break;
            case KEY_F1: case KEY_F2: case KEY_F3: {
                VideoWriteText(cursorX - 1, cursorY - 1, 0xf, "\305");
                for( i = 3; i <= 20; i ++)
                    SidebarClearLine(i);
                switch (InputKeyPressed) {
                case KEY_F1: selectedCategory = CATEGORY_ITEM; break;
                case KEY_F2: selectedCategory = CATEGORY_CREATURE; break;
                case KEY_F3: selectedCategory = CATEGORY_TERRAIN; break;
                }
                i = 3;  /* Y position for text writing */
                for( iElem = 0; iElem <= MAX_ELEMENT; iElem ++) {
                    if (ElementDefs[iElem].EditorCategory == selectedCategory)  {
                        if (length(ElementDefs[iElem].CategoryName) != 0)  {
                            i = i + 1;
                            VideoWriteText(65, i, 0x1e, ElementDefs[iElem].CategoryName);
                            i = i + 1;
                        }

                        VideoWriteText(61, i, ((i % 2) << 6) + 0x30,
                                       string(' ') + ElementDefs[iElem].EditorShortcut + ' ');
                        VideoWriteText(65, i, 0x1f, ElementDefs[iElem].Name);
                        if (ElementDefs[iElem].Color == COLOR_CHOICE_ON_BLACK)
                            elemMenuColor = (cursorColor % 0x10) + 0x10;
                        else if (ElementDefs[iElem].Color == COLOR_WHITE_ON_CHOICE)
                            elemMenuColor = (cursorColor * 0x10) - 0x71;
                        else if (ElementDefs[iElem].Color == COLOR_CHOICE_ON_CHOICE)
                            elemMenuColor = ((cursorColor - 8) * 0x11) + 8;
                        else if ((ElementDefs[iElem].Color & 0x70) == 0)
                            elemMenuColor = (ElementDefs[iElem].Color % 0x10) + 0x10;
                        else
                            elemMenuColor = ElementDefs[iElem].Color;
                        VideoWriteText(78, i, elemMenuColor, ElementDefs[iElem].Character);

                        i = i + 1;
                    }
                }
                InputReadWaitKey();
                for( iElem = 1; iElem <= MAX_ELEMENT; iElem ++) {
                    if ((ElementDefs[iElem].EditorCategory == selectedCategory)
                            && (ElementDefs[iElem].EditorShortcut == upcase(InputKeyPressed))) {
                        if (iElem == E_PLAYER)  {
                            if (EditorPrepareModifyTile(cursorX, cursorY))
                                MoveStat(0, cursorX, cursorY);
                        } else {
                            if (ElementDefs[iElem].Color == COLOR_CHOICE_ON_BLACK)
                                elemMenuColor = cursorColor;
                            else if (ElementDefs[iElem].Color == COLOR_WHITE_ON_CHOICE)
                                elemMenuColor = (cursorColor * 0x10) - 0x71;
                            else if (ElementDefs[iElem].Color == COLOR_CHOICE_ON_CHOICE)
                                elemMenuColor = ((cursorColor - 8) * 0x11) + 8;
                            else
                                elemMenuColor = ElementDefs[iElem].Color;

                            if (ElementDefs[iElem].Cycle == -1)  {
                                if (EditorPrepareModifyTile(cursorX, cursorY))
                                    EditorSetAndCopyTile(cursorX, cursorY, iElem, elemMenuColor);
                            } else {
                                if (EditorPrepareModifyStatAtCursor())  {
                                    AddStat(cursorX, cursorY, iElem, elemMenuColor,
                                            ElementDefs[iElem].Cycle, StatTemplateDefault);
                                    {
                                        TStat& with1 = Board.Stats[Board.StatCount];
                                        if (length(ElementDefs[iElem].Param1Name) != 0)
                                            with1.P1 = World.EditorStatSettings[iElem].P1;
                                        if (length(ElementDefs[iElem].Param2Name) != 0)
                                            with1.P2 = World.EditorStatSettings[iElem].P2;
                                        if (length(ElementDefs[iElem].ParamDirName) != 0)  {
                                            with1.StepX = World.EditorStatSettings[iElem].StepX;
                                            with1.StepY = World.EditorStatSettings[iElem].StepY;
                                        }
                                        if (length(ElementDefs[iElem].ParamBoardName) != 0)
                                            with1.P3 = World.EditorStatSettings[iElem].P3;
                                    }
                                    EditorEditStat(Board.StatCount);
                                    if (InputKeyPressed == KEY_ESCAPE)
                                        RemoveStat(Board.StatCount);
                                }
                            }
                        }
                    }
                }
                EditorDrawSidebar();
            }
            break;
            case KEY_F4: {
                if (drawMode != TextEntry)
                    drawMode = TextEntry;
                else
                    drawMode = DrawingOff;
            }
            break;
            case 'H': {
                TextWindowDisplayFile("editor.hlp", "World editor help");
            }
            break;
            case 'X': {
                EditorFloodFill(cursorX, cursorY, Board.Tiles[cursorX][cursorY]);
            }
            break;
            case '!': {
                EditorEditHelpFile();
                EditorDrawSidebar();
            }
            break;
            case 'T': {
                EditorTransferBoard();
            }
            break;
            case KEY_ENTER: {
                if (GetStatIdAt(cursorX, cursorY) >= 0)  {
                    EditorEditStat(GetStatIdAt(cursorX, cursorY));
                    EditorDrawSidebar();
                } else {
                    copiedHasStat = false;
                    copiedTile = Board.Tiles[cursorX][cursorY];
                }
            }
            break;
            case 'I': {
                EditorEditBoardInfo();
                TransitionDrawToBoard();
            }
            break;
            }
        }

        if (editorExitRequested)  {
            EditorAskSaveChanged();
            if (InputKeyPressed == KEY_ESCAPE)  {
                editorExitRequested = false;
                EditorDrawSidebar();
            }
        }
    } while (!editorExitRequested);

    /* Deallocate copied data if there is any. */

    if (copiedDataLen > 0)  {
        FreeMem(copiedData, copiedDataLen);
        copiedDataLen = 0;
    }

    InputKeyPressed = '\0';
    InitElementsGame();
}

void HighScoresLoad() {
    file<THighScoreList> f;
    integer i;

    assign(f, World.Info.Name + ".HI");
    OpenForRead(f);
    if (ioResult == 0)  {
        f >> HighScoreList;
    }
    close(f);
    if (ioResult != 0)  {
        for( i = 1; i <= 30; i ++) {
            HighScoreList[i].Name = "";
            HighScoreList[i].Score = -1;
        }
    }
}

void HighScoresSave() {
    file<THighScoreList> f;

    assign(f, World.Info.Name + ".HI");
    OpenForWrite(f);
    f << HighScoreList;
    close(f);
    if (DisplayIOError())  {
        ;
    } else {
        ;
    }
}

/*$F+*/

void HighScoresInitTextWindow(TTextWindowState& state) {
    integer i;
    string scoreStr;

    TextWindowInitState(state);
    TextWindowAppend(state, "Score  Name");
    TextWindowAppend(state, "-----  ----------------------------------");
    for( i = 1; i <= HIGH_SCORE_COUNT; i ++) {
        if (length(HighScoreList[i].Name) != 0)  {
            str(HighScoreList[i].Score,5, scoreStr);
            TextWindowAppend(state, scoreStr + "  " + HighScoreList[i].Name);
        }
    }
}

void HighScoresDisplay(integer linePos) {
    TTextWindowState state;

    state.LinePos = linePos;
    HighScoresInitTextWindow(state);
    if (state.LineCount > 2)  {
        state.Title = string("High scores for ") + World.Info.Name;
        TextWindowDrawOpen(state);
        TextWindowSelect(state, false, true);
        TextWindowDrawClose(state);
    }
    TextWindowFree(state);
}

void EditorOpenEditTextWindow(TTextWindowState& state) {
    SidebarClear();
    VideoWriteText(61, 4, 0x30, " Return ");
    VideoWriteText(64, 5, 0x1f, " Insert line");
    VideoWriteText(61, 7, 0x70, " Ctrl-Y ");
    VideoWriteText(64, 8, 0x1f, " Delete line");
    VideoWriteText(61, 10, 0x30, " Cursor keys ");
    VideoWriteText(64, 11, 0x1f, " Move cursor");
    VideoWriteText(61, 13, 0x70, " Insert ");
    VideoWriteText(64, 14, 0x1f, " Insert mode: ");
    VideoWriteText(61, 16, 0x30, " Delete ");
    VideoWriteText(64, 17, 0x1f, " Delete char");
    VideoWriteText(61, 19, 0x70, " Escape ");
    VideoWriteText(64, 20, 0x1f, " Exit editor");
    TextWindowEdit(state);
}

void EditorEditHelpFile() {
    TTextWindowState textWindow;
    varying_string<50> filename;

    filename = "";
    SidebarPromptString("File to edit", ".HLP", filename, PROMPT_ALPHANUM);
    if (length(filename) != 0)  {
        TextWindowOpenFile(string('*') + filename + ".HLP", textWindow);
        textWindow.Title = string("Editing ") + filename;
        TextWindowDrawOpen(textWindow);
        EditorOpenEditTextWindow(textWindow);
        TextWindowSaveFile(filename + ".HLP", textWindow);
        TextWindowFree(textWindow);
        TextWindowDrawClose(textWindow);
    }
}

void HighScoresAdd(integer score) {
    TTextWindowState textWindow;
    varying_string<50> name;
    integer i, listPos;

    listPos = 1;
    while ((listPos <= 30) && (score < HighScoreList[listPos].Score))
        listPos = listPos + 1;
    if ((listPos <= 30) && (score > 0))  {
        for( i = 29; i >= listPos; i --)
            HighScoreList[i + 1] = HighScoreList[i];
        HighScoreList[listPos].Score = score;
        HighScoreList[listPos].Name = "-- You! --";

        HighScoresInitTextWindow(textWindow);
        textWindow.LinePos = listPos;
        textWindow.Title = string("New high score for ") + World.Info.Name;
        TextWindowDrawOpen(textWindow);
        TextWindowDraw(textWindow, false, false);

        name = "";
        PopupPromptString("Congratulations!  Enter your name:", name);
        HighScoreList[listPos].Name = name;
        HighScoresSave();

        TextWindowDrawClose(textWindow);
        TransitionDrawToBoard();
        TextWindowFree(textWindow);
    }
}

TString50 EditorGetBoardName(integer boardId, boolean titleScreenIsNone) {
    byte* boardData;
    TString50 copiedName;

    TString50 EditorGetBoardName_result;
    if ((boardId == 0) && titleScreenIsNone)
        EditorGetBoardName_result = "None";
    else if (boardId == World.Info.CurrentBoard)
        EditorGetBoardName_result = Board.Name;
    else {
        boardData = World.BoardData[boardId];
        /* SANITY: Range check on board name length.*/
        *boardData = Min(*boardData, sizeof(copiedName)-1);
        *boardData = Min(*boardData, World.BoardLen[boardId]-1);

        Move(*boardData, copiedName, 1+(*boardData));
        EditorGetBoardName_result = copiedName;
    }
    return EditorGetBoardName_result;
}

integer EditorSelectBoard(string title, integer currentBoard,
                          boolean titleScreenIsNone) {
    string unk1;
    integer i;
    integer unk2;
    TTextWindowState textWindow;

    integer EditorSelectBoard_result;
    textWindow.Title = title;
    textWindow.LinePos = currentBoard + 1;
    textWindow.Selectable = true;
    textWindow.LineCount = 0;
    for( i = 0; i <= World.BoardCount; i ++) {
        TextWindowAppend(textWindow, EditorGetBoardName(i, titleScreenIsNone));
    }
    TextWindowAppend(textWindow, "Add new board");
    TextWindowDrawOpen(textWindow);
    TextWindowSelect(textWindow, false, false);
    TextWindowDrawClose(textWindow);
    TextWindowFree(textWindow);
    if (InputKeyPressed == KEY_ESCAPE)
        EditorSelectBoard_result = 0;
    else
        EditorSelectBoard_result = textWindow.LinePos - 1;
    return EditorSelectBoard_result;
}

class unit_Editor_initialize {
public: unit_Editor_initialize();
};
static unit_Editor_initialize Editor_constructor;

unit_Editor_initialize::unit_Editor_initialize() {
    ;
}