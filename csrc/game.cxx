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
#define __Game_implementation__


#include "game.h"

/*#include "Dos.h"*/
/*#include "Crt.h"*/
#include "Video.h"
#include "sounds.h"
#include "input.h"
#include "elements.h"
#include "editor.h"
#include "oop.h"
#include "minmax.h"
#include "fileops.h"


boolean ValidCoord(integer x, integer y) {
    boolean ValidCoord_result;
    if ((x < 0) || (y < 0))  {
        ValidCoord_result = false;
        return ValidCoord_result;
    }
    if ((x > BOARD_WIDTH+1) || (y > BOARD_HEIGHT+1))  {
        ValidCoord_result = false;
        return ValidCoord_result;
    }
    ValidCoord_result = true;
    return ValidCoord_result;
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
    VideoWriteText(60, y, 0x11, "\263                   ");
}

void SidebarClear() {
    integer i;

    for( i = 3; i <= 24; i ++)
        SidebarClearLine(i);
}

void GenerateTransitionTable() {
    integer ix, iy;
    TCoord t;

    TransitionTableSize = 0;
    for( iy = 1; iy <= BOARD_HEIGHT; iy ++)
        for( ix = 1; ix <= BOARD_WIDTH; ix ++) {
            TransitionTableSize = TransitionTableSize + 1;
            TransitionTable[TransitionTableSize].X = ix;
            TransitionTable[TransitionTableSize].Y = iy;
        }

    /* shuffle */
    for( ix = 1; ix <= TransitionTableSize; ix ++) {
        iy = Random(TransitionTableSize) + 1;
        t = TransitionTable[iy];
        TransitionTable[iy] = TransitionTable[ix];
        TransitionTable[ix] = t;
    }
}

void AdvancePointer(pointer& address, integer count) {
    address = address + count;
}

void BoardClose(boolean showTruncationNote) {
    integer ix, iy;
    pointer ptr;
    pointer ptrStart;
    TRleTile rle;
    boolean cleanupNeeded;

    ptr = IoTmpBuf;
    ptrStart = IoTmpBuf;
    cleanupNeeded = false;

    Move(Board.Name, ptr, sizeof(Board.Name));
    AdvancePointer(ptr, sizeof(Board.Name));

    ix = 1;
    iy = 1;
    rle.Count = 1;
    rle.Tile = Board.Tiles[ix][iy];
    do {
        ix = ix + 1;
        if (ix > BOARD_WIDTH)  {
            ix = 1;
            iy = iy + 1;
        }
        if ((Board.Tiles[ix][iy].Color == rle.Tile.Color) &&
                (Board.Tiles[ix][iy].Element == rle.Tile.Element) &&
                (rle.Count < 255) && (iy <= BOARD_HEIGHT)) {
            rle.Count = rle.Count + 1;
        } else {
            Move(rle, ptr, sizeof(rle));
            AdvancePointer(ptr, sizeof(rle));
            rle.Tile = Board.Tiles[ix][iy];
            rle.Count = 1;
        }
    } while (!(iy > BOARD_HEIGHT));

    Move(Board.Info, ptr, sizeof(Board.Info));
    AdvancePointer(ptr, sizeof(Board.Info));

    Move(Board.StatCount, ptr, sizeof(Board.StatCount));
    AdvancePointer(ptr, sizeof(Board.StatCount));

    for( ix = 0; ix <= Board.StatCount; ix ++) {
        {
            TStat& with = Board.Stats[ix];
            if (with.DataLen > 0)  {
                for( iy = 1; iy <= (ix - 1); iy ++) {
                    /* IMP: Make all bound objects link to the same one. */
                    if (Board.Stats[iy].Data == with.Data)  {
                        with.DataLen = -iy;
                        with.Data = nil;
                        flush();
                    }
                }
            }
            Move(Board.Stats[ix], ptr, sizeof(TStat));
            AdvancePointer(ptr, sizeof(TStat));
            if (with.DataLen > 0)  {
                Move(*with.Data, ptr, with.DataLen);
                FreeMem(with.Data, with.DataLen);
                AdvancePointer(ptr, with.DataLen);
            }
        }
    }

    /* For some reason, using @IoTmpBuf instead of ptrStart causes a range check error. */
    World.BoardLen[World.Info.CurrentBoard] = ptr - ptrStart;

    /* If we're using too much space, truncate the size, feed the
    whole thing back through BoardOpen to fix the inevitable
    	  corruption, then run BoardClose again.
    	  This smart-ass solution should allow us to keep all the smarts of
    	  board parsing in BoardOpen and nowt have to duplicate any logic. */
    /* Such a situation should *only* happen if RLE is too large (see
    RLEFLOW.ZZT), because AddStat should reject adding stats when
    	  there's no room. */
    if (World.BoardLen[World.Info.CurrentBoard] > MAX_BOARD_LEN)  {
        World.BoardLen[World.Info.CurrentBoard] = MAX_BOARD_LEN;
        cleanupNeeded = true;
    }

    /* LEAKFIX: Needs to be ReAllocMem instead of Get/Free because first time
    around the memory isn't allocated yet.*/
    ReAllocMem(World.BoardData[World.Info.CurrentBoard],
               World.BoardLen[World.Info.CurrentBoard]);
    Move(*IoTmpBuf, World.BoardData[World.Info.CurrentBoard],
         World.BoardLen[World.Info.CurrentBoard]);

    if (cleanupNeeded)  {
        BoardOpen(World.Info.CurrentBoard, false);
        BoardClose(false);
        if (showTruncationNote)  DisplayTruncationNote();
    }
}

/* Clean up stats by processing DataLen reference chains, clamping out-of-
  bounds stat values, and placing a player on the board if there is none
  already. */
void AdjustBoardStats() {
    integer ix, iy;


    /* SANITY: Process referential DataLen variables. This must be
    done after the former loop because otherwise it could be
    	  using incorrect data. */

    for( ix = 0; ix <= Board.StatCount; ix ++) {
        {
            TStat& with = Board.Stats[ix];
            if (with.DataLen < 0)  {
                /* Well-behaved reference chains do nothing in
                DOS ZZT, so cycles should do nothing too. If
                			  we're pointing at another reference or out of
                			  bounds, do nothing. */
                /* Furthermore, if we're the player, do nothing.
                Due to the way that BoardClose works, letting the
                			  player refer to a later object's data can't be
                			  allowed. Strictly speaking, referring to a later
                			  object is not allowed in general, but as long as
                			  the object doing the referring is not the player,
                			  we can pretend (in BoardClose) that the later object
                			  refers to the earlier's data instead. This is not
                			  possible with the player, because the reference
                			  DataLen would then be -0, which is just 0.*/
                if ((ix == 0) || (-with.DataLen > Board.StatCount) ||
                        (Board.Stats[-with.DataLen].DataLen < 0))
                    with.DataLen = 0;

                if (with.DataLen < 0)  {
                    with.Data = Board.Stats[-with.DataLen].Data;
                    with.DataLen = Board.Stats[-with.DataLen].DataLen;
                }

                /* If it's part of a chain, break the chain. */
                /* Can we do this?? */
                if (with.DataLen < 0)
                    with.DataLen = 0;
            }
        }
    }

    /* SANITY: Positive Leader and Follower values must be indices
    to stats. If they're too large, they're corrupt: set them to
    	  zero.
    	  Furthermore, there's no need for StepX and StepY to be out of
    	  range of the board area, and clamping these values helps
    	  avoid a ton of over/underflow problems whose fixes would
    	  otherwise clutter up the code... */
    for( ix = 0; ix <= Board.StatCount; ix ++) {
        {
            TStat& with = Board.Stats[ix];
            if (with.Follower > Board.StatCount)  with.Follower = 0;
            if (with.Leader > Board.StatCount)  with.Leader = 0;

            if (with.StepX < -BOARD_WIDTH)  with.StepX = -BOARD_WIDTH;
            if (with.StepX > BOARD_WIDTH)  with.StepX = BOARD_WIDTH;

            if (with.StepY < -BOARD_HEIGHT)  with.StepY = -BOARD_HEIGHT;
            if (with.StepY > BOARD_HEIGHT)  with.StepY = BOARD_HEIGHT;
        }
    }

    /* SANITY: If there's neither a player nor a monitor at the position
    indicated by stats 0, place a player there to keep the invariant
    	  that one should always exist on every board. */
    {
        TStat& with = Board.Stats[0];
        if ((Board.Tiles[with.X][with.Y].Element != E_PLAYER) &&
                (Board.Tiles[with.X][with.Y].Element != E_MONITOR))
            Board.Tiles[with.X][with.Y].Element = E_PLAYER;
    }
}

/* Set worldIsDamaged to true if the BoardOpen is from a world load and
the world metadata is wrong; this will make the corruption notification
show up regardless of whether the board itself is damaged. */
void BoardOpen(integer boardId, boolean worldIsDamaged) {
    byte* ptr;
    integer i, ix, iy;
    TRleTile rle;
    integer bytesRead;
    boolean boardIsDamaged;

    if (boardId > World.BoardCount)
        boardId = World.Info.CurrentBoard;

    ptr = World.BoardData[boardId];
    bytesRead = 0;

    /* Create a default yellow border board, because we might need
    to abort before the board is fully specced. */
    BoardCreate();
    boardIsDamaged = worldIsDamaged;

    /* Check that the sanity check on board titles have been executed. */

    /*SANITY: Reconstruct the title. We need at least a size of
    two bytes for the title: a size designation and the first
    	 letter of the title. If we don't even have that, let the
    	 title be blank.*/
    if (World.BoardLen[boardId] < sizeof(Board.Name))  {
        World.Info.CurrentBoard = boardId;

        if (World.BoardLen[boardId] > 1)  {
            *ptr = Min(*ptr, World.BoardLen[boardId]-1);
            Move(*ptr, Board.Name, *ptr);
        } else {
            Board.Name = "";
        }

        /* This board is damaged. */
        DisplayCorruptionNote();
        return;
    }

    /* SANITY: Range check on board name length. */
    *ptr = Min(*ptr, sizeof(Board.Name));

    Move(*ptr, Board.Name, sizeof(Board.Name));
    AdvancePointer(ptr, sizeof(Board.Name));
    bytesRead = bytesRead + sizeof(Board.Name);

    ix = 1;
    iy = 1;
    rle.Count = 0;
    do {
        /* ZZT used to have a "feature" where an RLE count of 0 would
        mean 256 repetitions of the tile. However, because it never
        writes those RLE counts itself, it would get desynchronized
        on a board close and crash. Therefore, we must simply ignore
        such rle count 0 bytes, even though that is not what DOS ZZT
        does. DOS ZZT would instead write past the bounds of the
        scratch space when cleaning up, which means authors can't use
        any RLE count 0 pairs without risking a glitch or crash
        anyway.

        BoardClose and BoardOpen may still desynchronize, but what
        BoardClose outputs will never be longer than what BoardOpen
        inputs, which is okay.

        If you absolutely need this functionality, you would have to
        increment an auxiliary counter by one every time you get an
        RLE count 0 byte and then allocate that much extra scratch
        space. But keeping two counts like that is a pain, so I don't.*/
        if (rle.Count <= 0)  {
            /* Not enough space? Get outta here. */
            if (bytesRead + sizeof(rle) > World.BoardLen[boardId])  flush();
            Move(*ptr, rle, sizeof(rle));
            AdvancePointer(ptr, sizeof(rle));
            bytesRead = bytesRead + sizeof(rle);
            if (rle.Count == 0)  {
                boardIsDamaged = true;
                continue_;
            }
        }

        /* SANITY: If the element is unknown, replace it with a normal. */

        if (rle.Tile.Element > MAX_ELEMENT)  {
            rle.Tile.Element = E_NORMAL;
            boardIsDamaged = true;
        }

        Board.Tiles[ix][iy] = rle.Tile;
        ix = ix + 1;
        if (ix > BOARD_WIDTH)  {
            ix = 1;
            iy = iy + 1;
        }

        rle.Count = rle.Count - 1;

    } while (!((iy > BOARD_HEIGHT) || (bytesRead >= World.BoardLen[boardId])));

    /* SANITY: If reading board info and the stats count byte would
    get us out of bounds, we have a board that's truncated too early.
    	  Do the best we can, then show the damaged board note and exit. */
    if ((sizeof(Board.Info) + sizeof(Board.StatCount) + bytesRead) >=
            World.BoardLen[boardId])  {
        World.Info.CurrentBoard = boardId;
        AdjustBoardStats();
        DisplayCorruptionNote();
        return;
    }

    Move(*ptr, Board.Info, sizeof(Board.Info));
    AdvancePointer(ptr, sizeof(Board.Info));
    bytesRead = bytesRead + sizeof(Board.Info);

    /* Clamp out-of-bounds Board.Info variables. They'll cause problems
    in the editor otherwise. */
    for( i = 0; i <= 3; i ++)
        /* This behavior is from elements.pas, BoardEdgeTouch. */
        if (Board.Info.NeighborBoards[i] > World.BoardCount)
            Board.Info.NeighborBoards[i] = boardId;

    if (! ValidCoord(Board.Info.StartPlayerX, Board.Info.StartPlayerY))  {
        Board.Info.StartPlayerX = 1;
        Board.Info.StartPlayerY = 1;
        boardIsDamaged = true;
    }

    Move(*ptr, Board.StatCount, sizeof(Board.StatCount));
    AdvancePointer(ptr, sizeof(Board.StatCount));
    bytesRead = bytesRead + sizeof(Board.StatCount);

    Board.StatCount = Max(0, Min(Board.StatCount, MAX_STAT));

    for( ix = 0; ix <= Board.StatCount; ix ++) {
        TStat& with = Board.Stats[ix];
        /* SANITY: Handle too few stats items for the stats count. */
        if ((bytesRead + sizeof(TStat)) > World.BoardLen[boardId])  {
            Board.StatCount = Max(ix - 1, 0);
            World.Info.CurrentBoard = boardId;
            boardIsDamaged = true;
            flush();
        }

        Move(*ptr, Board.Stats[ix], sizeof(TStat));
        AdvancePointer(ptr, sizeof(TStat));
        bytesRead = bytesRead + sizeof(TStat);

        /* SANITY: If the element underneath is unknown, replace it
        with a normal. */
        if (with.Under.Element > MAX_ELEMENT)  {
            with.Under.Element = E_NORMAL;
            boardIsDamaged = true;
        }

        /* SANITY: Handle objects that are out of bounds. */
        if (! ValidCoord(with.X, with.Y))  {
            with.X = Min(Max(with.X, 0), BOARD_WIDTH+1);
            with.Y = Min(Max(with.Y, 0), BOARD_HEIGHT+1);
            boardIsDamaged = true;
        }

        /* SANITY: (0,0) is not available: it's used by one-line
        messages. So if the stat is at (0,0) or another
        		  unavailable position, put it into (1,1). TODO? Make
        		  a note of which are thus placed, and place them on
        		  empty spots on the board instead if possible... */
        /* The compromise to the Postelic position is probably to
        be generous, but show a warning message that the board
        		  was corrupted and attempted fixed. */
        if ((with.X == 0) && (with.Y == 0))  {
            with.X = 1;
            with.Y = 1;
            boardIsDamaged = true;
        }

        /* SANITY: If DataLen is much too large, truncate. We'll
        then stop processing more objects next round around
        the loop. */
        if (bytesRead + with.DataLen > World.BoardLen[boardId])  {
            with.DataLen = World.BoardLen[boardId] - bytesRead;
            boardIsDamaged = true;
        }

        if (with.DataLen > 0)
            /* SANITY: If DataLen is too long, truncate it. */
            if (with.DataLen > World.BoardLen[boardId]-bytesRead)  {
                with.DataLen = World.BoardLen[boardId]-bytesRead;
                boardIsDamaged = true;
            }

        /* Only allocate if data length is still positive... */
        if (with.DataLen > 0)  {
            GetMem(with.Data, with.DataLen);
            Move(*ptr, *with.Data, with.DataLen);
            AdvancePointer(ptr, with.DataLen);
            bytesRead = bytesRead + with.DataLen;
        }

        /* Otherwise, clear Data to avoid potential leaks later. */
        if (with.DataLen == 0)  with.Data = nil;
    }

    AdjustBoardStats();
    World.Info.CurrentBoard = boardId;

    if (boardIsDamaged)
        DisplayCorruptionNote();
}

void BoardChange(integer boardId) {
    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = E_PLAYER;
    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color =
        ElementDefs[E_PLAYER].Color;
    if (boardId != World.Info.CurrentBoard)  {
        BoardClose(true);
        BoardOpen(boardId, false);
    }
}

void BoardCreate() {
    integer ix, iy, i;

    Board.Name = "";
    Board.Info.Message = "";
    Board.Info.MaxShots = 255;
    Board.Info.IsDark = false;
    Board.Info.ReenterWhenZapped = false;
    Board.Info.TimeLimitSec = 0;
    for( i = 0; i <= 3; i ++)
        Board.Info.NeighborBoards[i] = 0;

    for( ix = 0; ix <= BOARD_WIDTH+1; ix ++) {
        Board.Tiles[ix][0] = TileBoardEdge;
        Board.Tiles[ix][BOARD_HEIGHT+1] = TileBoardEdge;
    }
    for( iy = 0; iy <= BOARD_HEIGHT+1; iy ++) {
        Board.Tiles[0][iy] = TileBoardEdge;
        Board.Tiles[BOARD_WIDTH+1][iy] = TileBoardEdge;
    }

    for( ix = 1; ix <= BOARD_WIDTH; ix ++)
        for( iy = 1; iy <= BOARD_HEIGHT; iy ++) {
            Board.Tiles[ix][iy].Element = E_EMPTY;
            Board.Tiles[ix][iy].Color = 0;
        }

    for( ix = 1; ix <= BOARD_WIDTH; ix ++) {
        Board.Tiles[ix][1] = TileBorder;
        Board.Tiles[ix][BOARD_HEIGHT] = TileBorder;
    }
    for( iy = 1; iy <= BOARD_HEIGHT; iy ++) {
        Board.Tiles[1][iy] = TileBorder;
        Board.Tiles[BOARD_WIDTH][iy] = TileBorder;
    }

    Board.Tiles[BOARD_WIDTH / 2][BOARD_HEIGHT / 2].Element = E_PLAYER;
    Board.Tiles[BOARD_WIDTH / 2][BOARD_HEIGHT / 2].Color =
        ElementDefs[E_PLAYER].Color;
    Board.StatCount = 0;
    Board.Stats[0].X = BOARD_WIDTH / 2;
    Board.Stats[0].Y = BOARD_HEIGHT / 2;
    Board.Stats[0].Cycle = 1;
    Board.Stats[0].Under.Element = E_EMPTY;
    Board.Stats[0].Under.Color = 0;
    Board.Stats[0].Data = nil;
    Board.Stats[0].DataLen = 0;
}

void WorldCreate() {
    integer i;

    InitElementsGame();
    World.BoardCount = 0;
    World.BoardLen[0] = 0;
    InitEditorStatSettings();
    ResetMessageNotShownFlags();
    BoardCreate();
    World.Info.IsSave = false;
    World.Info.CurrentBoard = 0;
    World.Info.Ammo = 0;
    World.Info.Gems = 0;
    World.Info.Health = 100;
    World.Info.EnergizerTicks = 0;
    World.Info.Torches = 0;
    World.Info.TorchTicks = 0;
    World.Info.Score = 0;
    World.Info.BoardTimeSec = 0;
    World.Info.BoardTimeHsec = 0;
    for( i = 1; i <= 7; i ++)
        World.Info.Keys[i] = false;
    for( i = 1; i <= 10; i ++)
        World.Info.Flags[i] = "";
    BoardChange(0);
    Board.Name = "Title screen";
    LoadedGameFileName = "";
    World.Info.Name = "";
}

void TransitionDrawToFill(char chr_, integer color) {
    integer i;

    for( i = 1; i <= TransitionTableSize; i ++)
        VideoWriteText(TransitionTable[i].X - 1, TransitionTable[i].Y - 1, color,
                       chr_);
}

void BoardDrawTile(integer x, integer y) {
    byte ch;

    {
        TTile& with = Board.Tiles[x][y];
        if (! Board.Info.IsDark
                || (ElementDefs[Board.Tiles[x][y].Element].VisibleInDark)
                || (
                    (World.Info.TorchTicks > 0)
                    && ((sqr(Board.Stats[0].X - x) + sqr(Board.Stats[0].Y - y) * 2) <
                        TORCH_DIST_SQR)
                ) || ForceDarknessOff) {
            if (with.Element == E_EMPTY)
                VideoWriteText(x - 1, y - 1, 0xf, " ");
            else if ((with.Element < E_TEXT_MIN)
                     && ElementDefs[with.Element].HasDrawProc)  {
                ElementDefs[with.Element].DrawProc(x, y, ch);
                VideoWriteText(x - 1, y - 1, with.Color, chr(ch));
            } else if (with.Element < E_TEXT_MIN)
                VideoWriteText(x - 1, y - 1, with.Color,
                               ElementDefs[with.Element].Character);
            else {
                /* Text drawing */
                if (with.Element == E_TEXT_WHITE)
                    VideoWriteText(x - 1, y - 1, 0xf, chr(Board.Tiles[x][y].Color));
                else if (VideoMonochrome)
                    VideoWriteText(x - 1, y - 1, ((with.Element - E_TEXT_MIN) + 1) * 16,
                                   chr(Board.Tiles[x][y].Color));
                else
                    VideoWriteText(x - 1, y - 1,
                                   (((with.Element - E_TEXT_MIN) + 1) * 16) + 0xf,
                                   chr(Board.Tiles[x][y].Color));
            }
        } else {
            /* Darkness */
            VideoWriteText(x - 1, y - 1, 0x7, "\260");
        }
    }
}

void BoardDrawBorder() {
    integer ix, iy;

    for( ix = 1; ix <= BOARD_WIDTH; ix ++) {
        BoardDrawTile(ix, 1);
        BoardDrawTile(ix, BOARD_HEIGHT);
    }

    for( iy = 1; iy <= BOARD_HEIGHT; iy ++) {
        BoardDrawTile(1, iy);
        BoardDrawTile(BOARD_WIDTH, iy);
    }
}

void TransitionDrawToBoard() {
    integer i;

    BoardDrawBorder();

    for( i = 1; i <= TransitionTableSize; i ++) {
        TCoord& with = TransitionTable[i];
        BoardDrawTile(with.X, with.Y);
    }
}

void SidebarPromptCharacter(boolean editable, integer x, integer y,
                            TString50 prompt, byte& value) {
    integer i, newValue;

    SidebarClearLine(y);
    VideoWriteText(x, y, (integer)(editable) + 0x1e, prompt);
    SidebarClearLine(y + 1);
    VideoWriteText(x + 5, y + 1, 0x9f, "\37");
    SidebarClearLine(y + 2);

    do {
        for( i = (value - 4); i <= (value + 4); i ++)
            VideoWriteText(((x + i) - value) + 5, y + 2, 0x1e,
                           chr((i + 0x100) % 0x100));

        if (editable)  {
            Delay(25);
            InputUpdate();
            if (InputKeyPressed == KEY_TAB)
                InputDeltaX = 9;

            newValue = value + InputDeltaX;
            if (value != newValue)  {
                value = (newValue + 0x100) % 0x100;
                SidebarClearLine(y + 2);
            }
        }
    } while (!((InputKeyPressed == KEY_ENTER)
               || (InputKeyPressed == KEY_ESCAPE) || ! editable || InputShiftPressed));

    VideoWriteText(x + 5, y + 1, 0x1f, "\37");
}

void SidebarPromptSlider(boolean editable, integer x, integer y,
                         string prompt, byte& value) {
    integer newValue;
    char startChar, endChar;

    if (prompt[length(prompt) - 2] == ';')  {
        startChar = prompt[length(prompt) - 1];
        endChar = prompt[length(prompt)];
        prompt = copy(prompt, 1, length(prompt) - 3);
    } else {
        startChar = '1';
        endChar = '9';
    }

    SidebarClearLine(y);
    VideoWriteText(x, y, (integer)(editable) + 0x1e, prompt);
    SidebarClearLine(y + 1);
    SidebarClearLine(y + 2);
    VideoWriteText(x, y + 2, 0x1e, string(startChar) + "....:...." + endChar);

    do {
        if (editable)  {
            if (InputJoystickMoved)
                Delay(45);
            VideoWriteText(x + value + 1, y + 1, 0x9f, "\37");

            InputUpdate();
            if ((InputKeyPressed >= '1') && (InputKeyPressed <= '9'))  {
                value = ord(InputKeyPressed) - 49;
                SidebarClearLine(y + 1);
            } else {
                newValue = value + InputDeltaX;
                if ((value != newValue) && (newValue >= 0) && (newValue <= 8))  {
                    value = newValue;
                    SidebarClearLine(y + 1);
                }
            }
        }
    } while (!((InputKeyPressed == KEY_ENTER)
               || (InputKeyPressed == KEY_ESCAPE) || ! editable || InputShiftPressed));

    VideoWriteText(x + value + 1, y + 1, 0x1f, "\37");
}

void SidebarPromptChoice(boolean editable, integer y, string prompt,
                         string choiceStr, byte& result) {
    integer i, j, choiceCount;
    integer newResult;

    SidebarClearLine(y);
    SidebarClearLine(y + 1);
    SidebarClearLine(y + 2);
    VideoWriteText(63, y, (integer)(editable) + 0x1e, prompt);
    VideoWriteText(63, y + 2, 0x1e, choiceStr);

    choiceCount = 1;
    for( i = 1; i <= length(choiceStr); i ++)
        if (choiceStr[i] == ' ')
            choiceCount = choiceCount + 1;

    do {
        j = 0;
        i = 1;
        while ((j < result) && (i < length(choiceStr)))  {
            if (choiceStr[i] == ' ')
                j = j + 1;
            i = i + 1;
        }

        if (editable)  {
            VideoWriteText(62 + i, y + 1, 0x9f, "\37");
            Delay(35);
            InputUpdate();

            newResult = result + InputDeltaX;
            if ((result != newResult) && (newResult >= 0)
                    && (newResult <= (choiceCount - 1)))  {
                result = newResult;
                SidebarClearLine(y + 1);
            }
        }
    } while (!((InputKeyPressed == KEY_ENTER)
               || (InputKeyPressed == KEY_ESCAPE) || ! editable || InputShiftPressed));

    VideoWriteText(62 + i, y + 1, 0x1f, "\37");
}

void SidebarPromptDirection(boolean editable, integer y, string prompt,
                            integer& deltaX, integer& deltaY) {
    byte choice;

    if (deltaY == -1)
        choice = 0;
    else if (deltaY == 1)
        choice = 1;
    else if (deltaX == -1)
        choice = 2;
    else
        choice = 3;
    SidebarPromptChoice(editable, y, prompt, "\30 \31 \33 \32", choice);
    deltaX = NeighborDeltaX[choice];
    deltaY = NeighborDeltaY[choice];
}

void PromptString(integer x, integer y, integer arrowColor, integer color,
                  integer width, byte mode, TString50& buffer) {
    integer i;
    string oldBuffer;
    boolean firstKeyPress;

    oldBuffer = buffer;
    firstKeyPress = true;

    do {
        for( i = 0; i <= (width - 1); i ++) {
            VideoWriteText(x + i, y, color, " ");
            VideoWriteText(x + i, y - 1, arrowColor, " ");
        }
        VideoWriteText(x + width, y - 1, arrowColor, " ");
        VideoWriteText(x + length(buffer), y - 1, (arrowColor / 0x10) * 16 + 0xf,
                       "\37");
        VideoWriteText(x, y, color, buffer);

        InputReadWaitKey();

        if ((length(buffer) < width) && (InputKeyPressed >= '\40')
                && (! InputSpecialKeyPressed))  {
            if (firstKeyPress)
                buffer = "";
            switch (mode) {
            case PROMPT_NUMERIC: {
                if (set::of(range('0', '9'), eos).has(InputKeyPressed))  {
                    buffer = buffer + InputKeyPressed;
                }
            }
            break;
            case PROMPT_ANY: {
                buffer = buffer + InputKeyPressed;
            }
            break;
            case PROMPT_ALPHANUM: {
                if ((set::of(range('A', 'Z'), eos).has(upcase(InputKeyPressed)))
                        || (set::of(range('0', '9'), eos).has(InputKeyPressed))
                        || (InputKeyPressed == '-')) {
                    buffer = buffer + upcase(InputKeyPressed);
                }
            }
            break;
            }
        } else if ((InputKeyPressed == KEY_LEFT)
                   || (InputKeyPressed == KEY_BACKSPACE))  {
            buffer = copy(buffer, 1, length(buffer) - 1);
            /*IMP: Clear the whole line if Home is pressed.*/
        } else if (InputKeyPressed == KEY_HOME)  {
            buffer = "";
        }

        firstKeyPress = false;
    } while (!((InputKeyPressed == KEY_ENTER)
               || (InputKeyPressed == KEY_ESCAPE)));
    if (InputKeyPressed == KEY_ESCAPE)  {
        buffer = oldBuffer;
    }
}

boolean SidebarPromptYesNo(string message, boolean defaultReturn) {
    boolean SidebarPromptYesNo_result;
    SidebarClearLine(3);
    SidebarClearLine(4);
    SidebarClearLine(5);
    VideoWriteText(63, 5, 0x1f, message);
    VideoWriteText(63 + length(message), 5, 0x9e, "_");

    do {
        InputReadWaitKey();
    } while (!(set::of(KEY_ESCAPE, 'N', 'Y',
                       eos).has(upcase(InputKeyPressed))));
    if (upcase(InputKeyPressed) == 'Y')
        defaultReturn = true;
    else
        defaultReturn = false;

    SidebarClearLine(5);
    SidebarPromptYesNo_result = defaultReturn;
    return SidebarPromptYesNo_result;
}

void SidebarPromptString(string prompt, TString50 extension,
                         string& filename, byte promptMode) {
    SidebarClearLine(3);
    SidebarClearLine(4);
    SidebarClearLine(5);
    VideoWriteText(75 - length(prompt), 3, 0x1f, prompt);
    VideoWriteText(63, 5, 0xf, string("        ") + extension);

    PromptString(63, 5, 0x1e, 0xf, 8, promptMode, filename);

    SidebarClearLine(3);
    SidebarClearLine(4);
    SidebarClearLine(5);
}

void PauseOnError() {
    SoundQueue(1, SoundParse("s004x114x9"));
    Delay(2000);
}

boolean DisplayIOError() {
    word ioResVal;
    varying_string<50> errorNumStr;
    TTextWindowState textWindow;

    boolean DisplayIOError_result;
    ioResVal = ioResult;
    if (ioResVal == 0)  {
        DisplayIOError_result = false;
        return DisplayIOError_result;
    }

    DisplayIOError_result = true;

    /*IMP: Use explanations instead of numeric error codes if possible.*/
    textWindow.Title = string("Error: ") + ErrorString(ioResVal);
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
    return DisplayIOError_result;
}

void DisplayTruncationNote() {
    TTextWindowState textWindow;

    textWindow.Title = "Warning: Potential data loss";
    TextWindowInitState(textWindow);
    TextWindowAppend(textWindow, "$Warning:");
    TextWindowAppend(textWindow, "");
    TextWindowAppend(textWindow, "A board that was just saved was too large");
    TextWindowAppend(textWindow, "and some data had to be cut. This might");
    TextWindowAppend(textWindow, "lead to data loss. If you haven't saved");
    TextWindowAppend(textWindow, "yet, do so under another name and make");
    TextWindowAppend(textWindow, "the board smaller!");
    TextWindowAppend(textWindow, "");
    TextWindowAppend(textWindow, "If you're just playing, tell the author");
    TextWindowAppend(textWindow, "of the world that you're playing.");

    TextWindowDrawOpen(textWindow);
    TextWindowSelect(textWindow, false, false);
    TextWindowDrawClose(textWindow);
    TextWindowFree(textWindow);
}

void DisplayCorruptionNote() {
    TTextWindowState textWindow;

    textWindow.Title = "Warning: Corruption detected";
    TextWindowInitState(textWindow);
    TextWindowAppend(textWindow, "$Warning:");
    TextWindowAppend(textWindow, "");
    TextWindowAppend(textWindow, "The file or board that was just loaded");
    TextWindowAppend(textWindow, "contained some damaged information.");
    TextWindowAppend(textWindow, "This might be caused by a bad file");
    TextWindowAppend(textWindow, "or disk corruption. ZZT has tried");
    TextWindowAppend(textWindow, "to undo the damage, but some data");
    TextWindowAppend(textWindow, "might be lost.");

    TextWindowDrawOpen(textWindow);
    TextWindowSelect(textWindow, false, false);
    TextWindowDrawClose(textWindow);
    TextWindowFree(textWindow);
}

void WorldUnload() {
    integer i;

    /* no need to show any notices if the world's to be unloaded. */
    BoardClose(false);
    /* Reallocating to 0 is better than freeing because it's then
    possible to reallocate back to a higher level later, if
    required. That BoardLen is 0 also makes it obvious that the
    boards have been unloaded. */
    for( i = 0; i <= World.BoardCount; i ++) {
        World.BoardLen[i] = 0;
        ReAllocMem(World.BoardData[i], World.BoardLen[i]);
    }
}

boolean WorldLoad(TString50 filename, TString50 extension);

static integer loadProgress;

static void SidebarAnimateLoading() {
    VideoWriteText(69, 5, ProgressAnimColors[loadProgress],
                   ProgressAnimStrings[loadProgress]);
    loadProgress = (loadProgress + 1) % 8;
}

boolean WorldLoad(TString50 filename, TString50 extension) {
    untyped_file f;
    pointer ptr;
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
    VideoWriteText(62, 5, 0x1f, "Loading.....");

    /* filenames must be C strings, which means that they are terminated
    at the first 00, and thus we must remove everything from the first
    	  00 out. Not doing this can cause Assign to assign stdin to
    	  the file handle, with predictable results. */

    firstZero = length(filename)+1;
    for( i = length(filename); i >= 1; i --)
        if (filename[i] == '\0')
            firstZero = i;

    SetLength(filename, firstZero-1);

    if (filename + extension == "")  return WorldLoad_result;

    assign(f, filename + extension);
    OpenForRead(f, 1);

    if (! DisplayIOError())  {
        WorldUnload();
        BlockRead(f, *IoTmpBuf, 512);

        if (! DisplayIOError())  {
            ptr = IoTmpBuf;
            Move(ptr, World.BoardCount, sizeof(World.BoardCount));
            AdvancePointer(ptr, sizeof(World.BoardCount));

            if (World.BoardCount < 0)  {
                if (World.BoardCount != -1)  {
                    VideoWriteText(63, 5, 0x1e, "You need a newer");
                    VideoWriteText(63, 6, 0x1e, " version of ZZT!");
                    return WorldLoad_result;
                } else {
                    Move(ptr, World.BoardCount, sizeof(World.BoardCount));
                    AdvancePointer(ptr, sizeof(World.BoardCount));
                }
            }

            Move(ptr, World.Info, sizeof(World.Info));
            AdvancePointer(ptr, sizeof(World.Info));

            /* If the board count is negative, set it to zero. This should
            also signal that the world is corrupt. Another option
            		  would be to make all the fields unsigned, but who needs
            		  worlds with >32k boards anyway? Besides, they'd crash
            		  DOS ZZT. */
            if (World.BoardCount < 0)  {
                World.BoardCount = 0;
                worldIsDamaged = true;
            }

            /* If there are too many boards, ditto. (That's a more serious
            problem, as it may cut off boards outright.) */
            if (World.BoardCount > MAX_BOARD)  {
                World.BoardCount = MAX_BOARD;
                worldIsDamaged = true;
            }

            /* Don't accept CurrentBoard values that are too large or
            small. */
            if ((World.Info.CurrentBoard < 0)
                    || (World.Info.CurrentBoard > World.BoardCount))  {
                World.Info.CurrentBoard = Max(0, Min(World.BoardCount,
                                                     World.Info.CurrentBoard));
                worldIsDamaged = true;
            }

            for( boardId = 0; boardId <= World.BoardCount; boardId ++) {
                SidebarAnimateLoading();

                if (boardId > World.BoardCount)  continue_;

                BlockRead(f, World.BoardLen[boardId], 2);

                /* Sanity check. Abort at this position so that any
                boards before the corrupted one can still be
                			  recovered.*/
                if ((DisplayIOError()) || (World.BoardLen[boardId] < 0))  {
                    World.BoardLen[boardId] = 0;
                    worldIsDamaged = true;
                    if (boardId == 0)  {
                        WorldUnload();
                        return WorldLoad_result;
                    }
                    World.BoardCount = boardId - 1;
                    /* No more boards to be had, so break. */
                    flush();
                } else {
                    /* If it's the last board, get everything we can.
                    This recovers the last Super Lock-corrupted board.
                    				  actuallyRead below will adjust the board length back
                    				  if we're dealing with an ordinary world. */
                    if (boardId == World.BoardCount)
                        World.BoardLen[boardId] = MAX_BOARD_LEN;

                    GetMem(World.BoardData[boardId], World.BoardLen[boardId]);
                    BlockRead(f, World.BoardData[boardId], World.BoardLen[boardId],
                              actuallyRead);
                    /* SANITY: If reading the whole board would lead to an
                    overflow down the line, pretend we only read the
                    			  MAX_BOARD_LEN first. */
                    if (actuallyRead > MAX_BOARD_LEN)  {
                        actuallyRead = Min(actuallyRead, MAX_BOARD_LEN);
                        worldIsDamaged = true;
                    }

                    /* SANITY: reallocate and update board len if
                    there's a mismatch between how much we were told
                    				  we could read, and how much we actually read.
                    				  This also cuts down very large boards that we can't
                    				  represent in memory anyway (size > 20k). */
                    /* If you want to be extra stingy with memory, just
                    move this logic up to BlockRead and only read up to
                    				  MAX_BOARD_LEN, then seek the rest of the way. But
                    				  I can't be bothered. */
                    if (actuallyRead != World.BoardLen[boardId])  {
                        World.BoardData[boardId] = ReAllocMem(World.BoardData[boardId],
                                                              actuallyRead);
                        World.BoardLen[boardId] = actuallyRead;
                    }
                }
            }

            close(f);

            /* More sanity checks. If the current board number is
            negative or too high, set it to zero. */
            if ((World.Info.CurrentBoard < 0) ||
                    (World.Info.CurrentBoard > Min(MAX_BOARD, World.BoardCount)))
                World.Info.CurrentBoard = 0;

            BoardOpen(World.Info.CurrentBoard, worldIsDamaged);
            LoadedGameFileName = filename;
            WorldLoad_result = true;

            HighScoresLoad();

            SidebarClearLine(5);
        }
    }
    return WorldLoad_result;
}

void WorldSave(TString50 filename, TString50 extension) {
    untyped_file f;
    integer i;
    integer unk1;
    pointer ptr;
    integer version;


    BoardClose(true);
    VideoWriteText(63, 5, 0x1f, "Saving...");

    assign(f, filename + extension);
    OpenForWrite(f, 1);

    if (! DisplayIOError())  {
        ptr = IoTmpBuf;
        FillChar(*IoTmpBuf, 512, 0);
        version = -1;
        Move(version, ptr, sizeof(version));
        AdvancePointer(ptr, sizeof(version));

        Move(World.BoardCount, ptr, sizeof(World.BoardCount));
        AdvancePointer(ptr, sizeof(World.BoardCount));

        Move(World.Info, ptr, sizeof(World.Info));
        AdvancePointer(ptr, sizeof(World.Info));

        BlockWrite(f, *IoTmpBuf, 512);
        if (DisplayIOError())  goto LOnError;

        for( i = 0; i <= World.BoardCount; i ++) {
            BlockWrite(f, World.BoardLen[i], 2);
            if (DisplayIOError())  goto LOnError;

            BlockWrite(f, World.BoardData[i], World.BoardLen[i]);
            if (DisplayIOError())  goto LOnError;
        }

        close(f);
    }

    BoardOpen(World.Info.CurrentBoard, false);
    SidebarClearLine(5);
    return;

LOnError:
    close(f);
    Erase(f);
    BoardOpen(World.Info.CurrentBoard, false);
    SidebarClearLine(5);
}

void GameWorldSave(TString50 prompt, TString50& filename,
                   TString50 extension) {
    TString50 newFilename;

    newFilename = filename;
    SidebarPromptString(prompt, extension, newFilename, PROMPT_ALPHANUM);
    if ((InputKeyPressed != KEY_ESCAPE) && (length(newFilename) != 0))  {
        filename = newFilename;
        if (extension == ".ZZT")
            World.Info.Name = filename;
        WorldSave(filename, extension);
    }
}

boolean GameWorldLoad(TString50 extension) {
    TTextWindowState textWindow;
    SearchRec fileSearchRec;
    string entryName;
    integer i;

    boolean GameWorldLoad_result;
    TextWindowInitState(textWindow);
    if (extension == ".ZZT")
        textWindow.Title = "ZZT Worlds";
    else
        textWindow.Title = "Saved Games";
    GameWorldLoad_result = false;
    textWindow.Selectable = true;

    FindFirst(string('*') + extension, AnyFile, fileSearchRec);
    while (DosError == 0)  {
        entryName = copy(fileSearchRec.Name, 1, length(fileSearchRec.name) - 4);

        for( i = 1; i <= WorldFileDescCount; i ++)
            if (entryName == WorldFileDescKeys[i])
                entryName = WorldFileDescValues[i];

        TextWindowAppend(textWindow, entryName);
        FindNext(fileSearchRec);
    }
    TextWindowSort(textWindow); /* Sort the file names. */
    TextWindowAppend(textWindow, "Exit");

    TextWindowDrawOpen(textWindow);
    TextWindowSelect(textWindow, false, false);
    TextWindowDrawClose(textWindow);

    if ((textWindow.LinePos < textWindow.LineCount) && ! TextWindowRejected)  {
        entryName = *textWindow.Lines[textWindow.LinePos];
        if (pos(" ", entryName) != 0)
            entryName = copy(entryName, 1, pos(" ", entryName) - 1);

        GameWorldLoad_result = WorldLoad(entryName, extension);
        TransitionDrawToFill('\333', 0x44);
    }

    TextWindowFree(textWindow);
    return GameWorldLoad_result;
}

void CopyStatDataToTextWindow(integer statId, TTextWindowState& state) {
    string dataStr;
    pointer dataPtr;
    char dataChr;
    integer i;

    {
        TStat& with = Board.Stats[statId];
        TextWindowInitState(state);
        dataStr = "";
        dataPtr = with.Data;

        /* IMP: Fix off-by-one: Don't start counting
        from 0 when copying data. */
        for( i = 1; i <= with.DataLen; i ++) {
            Move(dataPtr, dataChr, 1);
            if (dataChr == KEY_ENTER)  {
                TextWindowAppend(state, dataStr);
                dataStr = "";
            } else {
                dataStr = dataStr + dataChr;
            }
            AdvancePointer(dataPtr, 1);
        }
    }
}

void AddStat(integer tx, integer ty, byte element, integer color,
             integer tcycle, TStat template_) {
    /* First of all: check if we have space. If not, no can do! */
    if ((template_.Data == nil) &&
            (World.BoardLen[World.Info.CurrentBoard] + sizeof(TStat) > MAX_BOARD_LEN))
        return;
    if ((template_.Data != nil) &&
            (World.BoardLen[World.Info.CurrentBoard] + sizeof(TStat) +
             template_.DataLen > MAX_BOARD_LEN))
        return;

    /* Can't put anything on top of the player. */
    if ((tx == Board.Stats[0].X) && (ty == Board.Stats[0].Y))  return;

    if (Board.StatCount < MAX_STAT)  {
        Board.StatCount = Board.StatCount + 1;
        Board.Stats[Board.StatCount] = template_;
        World.BoardLen[World.Info.CurrentBoard] =
            World.BoardLen[World.Info.CurrentBoard] + sizeof(TStat);
        {
            TStat& with = Board.Stats[Board.StatCount];
            with.X = tx;
            with.Y = ty;
            with.Cycle = tcycle;
            with.Under = Board.Tiles[tx][ty];
            with.DataPos = 0;
        }

        if ((template_.Data != nil) && (template_.DataLen > 0))  {
            GetMem(Board.Stats[Board.StatCount].Data, template_.DataLen);
            Move(*template_.Data, *Board.Stats[Board.StatCount].Data,
                 template_.DataLen);
            World.BoardLen[World.Info.CurrentBoard] =
                World.BoardLen[World.Info.CurrentBoard] + template_.DataLen;
        }

        if (ElementDefs[Board.Tiles[tx][ty].Element].PlaceableOnTop)
            Board.Tiles[tx][ty].Color = (color & 0xf) + (Board.Tiles[tx][ty].Color &
                                        0x70);
        else
            Board.Tiles[tx][ty].Color = color;
        Board.Tiles[tx][ty].Element = element;

        if (CoordInsideViewport(tx, ty))
            BoardDrawTile(tx, ty);
    }
}

void RemoveStat(integer statId) {
    integer i;


    {
        TStat& with = Board.Stats[statId];
        if (with.DataLen != 0)  {
            for( i = 1; i <= Board.StatCount; i ++) {
                if ((Board.Stats[i].Data == with.Data) && (i != statId))
                    goto LStatDataInUse;
            }
            FreeMem(with.Data, with.DataLen);
        }

LStatDataInUse:
        if (statId < CurrentStatTicked)
            CurrentStatTicked = CurrentStatTicked - 1;

        Board.Tiles[with.X][with.Y] = with.Under;
        if (with.Y > 0)
            BoardDrawTile(with.X, with.Y);

        for( i = 1; i <= Board.StatCount; i ++) {
            if (Board.Stats[i].Follower >= statId)  {
                if (Board.Stats[i].Follower == statId)
                    Board.Stats[i].Follower = -1;
                else
                    Board.Stats[i].Follower = Board.Stats[i].Follower - 1;
            }

            if (Board.Stats[i].Leader >= statId)  {
                if (Board.Stats[i].Leader == statId)
                    Board.Stats[i].Leader = -1;
                else
                    Board.Stats[i].Leader = Board.Stats[i].Leader - 1;
            }
        }

        for( i = (statId + 1); i <= Board.StatCount; i ++)
            Board.Stats[i - 1] = Board.Stats[i];
        Board.StatCount = Board.StatCount - 1;
    }
}

integer GetStatIdAt(integer x, integer y) {
    integer i;

    integer GetStatIdAt_result;
    i = -1;
    do {
        i = i + 1;
    } while (!(((Board.Stats[i].X == x) && (Board.Stats[i].Y == y))
               || (i > Board.StatCount)));

    if (i > Board.StatCount)
        GetStatIdAt_result = -1;
    else
        GetStatIdAt_result = i;
    return GetStatIdAt_result;
}

boolean BoardPrepareTileForPlacement(integer x, integer y) {
    integer statId;
    boolean result;

    boolean BoardPrepareTileForPlacement_result;
    statId = GetStatIdAt(x, y);
    if (statId > 0)  {
        RemoveStat(statId);
        result = true;
    } else if (statId < 0)  {
        if (! ElementDefs[Board.Tiles[x][y].Element].PlaceableOnTop)
            Board.Tiles[x][y].Element = E_EMPTY;
        result = true;
    } else {       /* statId = 0 (player) cannot be modified */
        result = false;
    }
    BoardDrawTile(x, y);
    BoardPrepareTileForPlacement_result = result;
    return BoardPrepareTileForPlacement_result;
}

void MoveStat(integer statId, integer newX, integer newY) {
    TTile iUnder;
    integer ix, iy;
    integer oldX, oldY;
    integer oldBgColor;

    {
        TStat& with = Board.Stats[statId];
        oldBgColor = Board.Tiles[newX][newY].Color & 0xf0;

        iUnder = Board.Stats[statId].Under;
        Board.Stats[statId].Under = Board.Tiles[newX][newY];

        if (Board.Tiles[with.X][with.Y].Element == E_PLAYER)
            Board.Tiles[newX][newY].Color = Board.Tiles[with.X][with.Y].Color;
        else if (Board.Tiles[newX][newY].Element == E_EMPTY)
            Board.Tiles[newX][newY].Color = Board.Tiles[with.X][with.Y].Color & 0xf;
        else
            Board.Tiles[newX][newY].Color = (Board.Tiles[with.X][with.Y].Color & 0xf)
                                            + (Board.Tiles[newX][newY].Color & 0x70);

        Board.Tiles[newX][newY].Element = Board.Tiles[with.X][with.Y].Element;
        Board.Tiles[with.X][with.Y] = iUnder;

        oldX = with.X;
        oldY = with.Y;
        with.X = newX;
        with.Y = newY;

        BoardDrawTile(with.X, with.Y);
        BoardDrawTile(oldX, oldY);

        if ((statId == 0) && Board.Info.IsDark && (World.Info.TorchTicks > 0))  {
            if ((sqr(oldX-with.X) + sqr(oldY-with.Y)) == 1)  {
                for( ix = (with.X - TORCH_DX - 3); ix <= (with.X + TORCH_DX + 3); ix ++)
                    if ((ix >= 1) && (ix <= BOARD_WIDTH))
                        for( iy = (with.Y - TORCH_DY - 3); iy <= (with.Y + TORCH_DY + 3); iy ++)
                            if ((iy >= 1) && (iy <= BOARD_HEIGHT))
                                if ((((sqr(ix-oldX))+(sqr(iy-oldY)*2)) < TORCH_DIST_SQR) ^
                                        (((sqr(ix-newX))+(sqr(iy-newY)*2)) < TORCH_DIST_SQR))
                                    BoardDrawTile(ix, iy);
            } else {
                DrawPlayerSurroundings(oldX, oldY, 0);
                DrawPlayerSurroundings(with.X, with.Y, 0);
            }
        }

    }
}

void PopupPromptString(string question, TString50& buffer) {
    integer x, y;

    VideoWriteText(3, 18, 0x4f, TextWindowStrTop);
    VideoWriteText(3, 19, 0x4f, TextWindowStrText);
    VideoWriteText(3, 20, 0x4f, TextWindowStrSep);
    VideoWriteText(3, 21, 0x4f, TextWindowStrText);
    VideoWriteText(3, 22, 0x4f, TextWindowStrText);
    VideoWriteText(3, 23, 0x4f, TextWindowStrBottom);
    VideoWriteText(4 + (TextWindowWidth - length(question)) / 2, 19, 0x4f,
                   question);
    buffer = "";
    PromptString(10, 22, 0x4f, 0x4e, TextWindowWidth - 16, PROMPT_ANY, buffer);
    for( y = 18; y <= 23; y ++)
        for( x = 3; x <= (TextWindowWidth + 3); x ++)
            BoardDrawTile(x + 1, y + 1);
}

integer Signum(integer val) {
    integer Signum_result;
    if (val > 0)
        Signum_result = 1;
    else if (val < 0)
        Signum_result = -1;
    else
        Signum_result = 0;
    return Signum_result;
}

integer Difference(integer a, integer b) {
    integer Difference_result;
    if ((a - b) >= 0)
        Difference_result = a - b;
    else
        Difference_result = b - a;
    return Difference_result;
}

void GameUpdateSidebar() {
    varying_string<8> numStr;
    integer i;

    if (GameStateElement == E_PLAYER)  {
        if (Board.Info.TimeLimitSec > 0)  {
            VideoWriteText(64, 6, 0x1e, "   Time:");
            str(Board.Info.TimeLimitSec - World.Info.BoardTimeSec, numStr);
            VideoWriteText(72, 6, 0x1e, numStr + ' ');
        } else {
            SidebarClearLine(6);
        }

        if (World.Info.Health < 0)
            World.Info.Health = 0;

        str(World.Info.Health, numStr);
        VideoWriteText(72, 7, 0x1e, numStr + ' ');
        str(World.Info.Ammo, numStr);
        VideoWriteText(72, 8, 0x1e, numStr + "  ");
        str(World.Info.Torches, numStr);
        VideoWriteText(72, 9, 0x1e, numStr + ' ');
        str(World.Info.Gems, numStr);
        VideoWriteText(72, 10, 0x1e, numStr + ' ');
        str(World.Info.Score, numStr);
        VideoWriteText(72, 11, 0x1e, numStr + ' ');

        if (World.Info.TorchTicks == 0)
            VideoWriteText(75, 9, 0x16, "    ");
        else {
            for( i = 2; i <= 5; i ++) {
                if (i <= ((World.Info.TorchTicks * 5) / TORCH_DURATION))
                    VideoWriteText(73 + i, 9, 0x16, "\261");
                else
                    VideoWriteText(73 + i, 9, 0x16, "\260");
            }
        }

        for( i = 1; i <= 7; i ++) {
            if (World.Info.Keys[i])
                VideoWriteText(71 + i, 12, 0x18 + i, ElementDefs[E_KEY].Character);
            else
                VideoWriteText(71 + i, 12, 0x1f, " ");
        }

        if (SoundEnabled)
            VideoWriteText(65, 15, 0x1f, " Be quiet");
        else
            VideoWriteText(65, 15, 0x1f, " Be noisy");

        if (DebugEnabled)  {
            /* TODO: Replace with some interesting stat
            on Linux.*/
            numStr = "lots";
            VideoWriteText(69, 4, 0x1e, string('m') + numStr + ' ');
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
        Board.Stats[Board.StatCount].P2 = Min(255, time / (TickTimeDuration + 1));
        Board.Info.Message = message;
    }
}

void DamageStat(integer attackerStatId) {
    integer oldX, oldY;

    {
        TStat& with = Board.Stats[attackerStatId];
        if (attackerStatId == 0)  {
            if (World.Info.Health > 0)  {
                World.Info.Health = World.Info.Health - 10;

                GameUpdateSidebar();
                DisplayMessage(100, "Ouch!");

                Board.Tiles[with.X][with.Y].Color = 0x70 + (ElementDefs[4].Color % 0x10);

                if (World.Info.Health > 0)  {
                    World.Info.BoardTimeSec = 0;
                    if (Board.Info.ReenterWhenZapped)  {
                        SoundQueue(4, "\40\1\43\1\47\1\60\1\20\1");

                        /* Move player to start */
                        Board.Tiles[with.X][with.Y].Element = E_EMPTY;
                        BoardDrawTile(with.X, with.Y);
                        oldX = with.X;
                        oldY = with.Y;
                        with.X = Board.Info.StartPlayerX;
                        with.Y = Board.Info.StartPlayerY;
                        DrawPlayerSurroundings(oldX, oldY, 0);
                        DrawPlayerSurroundings(with.X, with.Y, 0);

                        GamePaused = true;
                    }
                    SoundQueue(4, "\20\1\40\1\23\1\43\1");
                } else {
                    SoundQueue(5,
                               "\40\3\43\3\47\3\60\3\47\3\52\3\62\3\67\3\65\3\70\3\100\3\105\3\20\n");
                }
            }
        } else {
            switch (Board.Tiles[with.X][with.Y].Element) {
            case E_BULLET: SoundQueue(3, "\40\1"); break;
            case E_OBJECT: {; } break;
            default:
                SoundQueue(3, "\100\1\20\1\120\1\60\1");
            }
            RemoveStat(attackerStatId);
        }
    }
}

void BoardDamageTile(integer x, integer y) {
    integer statId;

    statId = GetStatIdAt(x, y);
    if (statId != -1)  {
        DamageStat(statId);
    } else {
        Board.Tiles[x][y].Element = E_EMPTY;
        BoardDrawTile(x, y);
    }
}

void BoardAttack(integer attackerStatId, integer x, integer y) {
    if ((attackerStatId == 0) && (World.Info.EnergizerTicks > 0))  {
        World.Info.Score = ElementDefs[Board.Tiles[x][y].Element].ScoreValue +
                           World.Info.Score;
        GameUpdateSidebar();
    } else {
        DamageStat(attackerStatId);
    }

    if ((attackerStatId > 0) && (attackerStatId <= CurrentStatTicked))
        CurrentStatTicked = CurrentStatTicked - 1;

    if ((Board.Tiles[x][y].Element == E_PLAYER)
            && (World.Info.EnergizerTicks > 0))  {
        World.Info.Score =
            ElementDefs[Board.Tiles[Board.Stats[attackerStatId].X][Board.Stats[attackerStatId].Y].Element]
            .ScoreValue + World.Info.Score;
        GameUpdateSidebar();
    } else {
        BoardDamageTile(x, y);
        SoundQueue(2, "\20\1");
    }
}

boolean BoardShoot(byte element, integer tx, integer ty, integer deltaX,
                   integer deltaY, integer source) {
    boolean BoardShoot_result;
    if (ElementDefs[Board.Tiles[tx + deltaX][ty + deltaY].Element].Walkable
            || (Board.Tiles[tx + deltaX][ty + deltaY].Element == E_WATER)) {
        AddStat(tx + deltaX, ty + deltaY, element, ElementDefs[element].Color, 1,
                StatTemplateDefault);
        {
            TStat& with = Board.Stats[Board.StatCount];
            with.P1 = source;
            with.StepX = deltaX;
            with.StepY = deltaY;
            with.P2 = 100;
        }
        BoardShoot_result = true;
    } else if ((Board.Tiles[tx + deltaX][ty + deltaY].Element == E_BREAKABLE)
               || (
                   ElementDefs[Board.Tiles[tx + deltaX][ty + deltaY].Element].Destructible
                   && ((Board.Tiles[tx + deltaX][ty + deltaY].Element == E_PLAYER) ==
                       (boolean)(source))
                   && (World.Info.EnergizerTicks <= 0)
               )) {
        BoardDamageTile(tx + deltaX, ty + deltaY);
        SoundQueue(2, "\20\1");
        BoardShoot_result = true;
    } else {
        BoardShoot_result = false;
    }
    return BoardShoot_result;
}

void CalcDirectionRnd(integer& deltaX, integer& deltaY) {
    deltaX = Random(3) - 1;

    if (deltaX == 0)
        deltaY = Random(2) * 2 - 1;
    else
        deltaY = 0;
}

void CalcDirectionSeek(integer x, integer y, integer& deltaX,
                       integer& deltaY) {
    deltaX = 0;
    deltaY = 0;

    if ((Random(2) < 1) || (Board.Stats[0].Y == y))
        deltaX = Signum(Board.Stats[0].X - x);

    if (deltaX == 0)
        deltaY = Signum(Board.Stats[0].Y - y);

    if (World.Info.EnergizerTicks > 0)  {
        deltaX = -deltaX;
        deltaY = -deltaY;
    }
}

void TransitionDrawBoardChange() {
    TransitionDrawToFill('\333', 0x5);
    TransitionDrawToBoard();
}

void BoardEnter() {
    Board.Info.StartPlayerX = Board.Stats[0].X;
    Board.Info.StartPlayerY = Board.Stats[0].Y;

    if (Board.Info.IsDark && MessageHintTorchNotShown)  {
        DisplayMessage(200, "Room is dark - you need to light a torch!");
        MessageHintTorchNotShown = false;
    }

    World.Info.BoardTimeSec = 0;
    GameUpdateSidebar();
}

void BoardPassageTeleport(integer x, integer y) {
    integer oldBoard;
    byte col;
    integer ix, iy;
    integer newX, newY;

    col = Board.Tiles[x][y].Color;

    oldBoard = World.Info.CurrentBoard;
    BoardChange(Board.Stats[GetStatIdAt(x, y)].P3);

    newX = 0;
    for( ix = 1; ix <= BOARD_WIDTH; ix ++)
        for( iy = 1; iy <= BOARD_HEIGHT; iy ++)
            if ((Board.Tiles[ix][iy].Element == E_PASSAGE)
                    && (Board.Tiles[ix][iy].Color == col))  {
                newX = ix;
                newY = iy;
            }

    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = E_EMPTY;
    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color = 0;
    if (newX != 0)  {
        Board.Stats[0].X = newX;
        Board.Stats[0].Y = newY;
    }

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
    for( i = 1; i <= length(input); i ++)
        input[i] = upcase(input[i]);

    toggle = true;
    if ((input[1] == '+') || (input[1] == '-'))  {
        if (input[1] == '-')
            toggle = false;
        input = copy(input, 2, length(input) - 1);

        if (toggle == true)
            WorldSetFlag(input);
        else
            WorldClearFlag(input);
    }

    DebugEnabled = WorldGetFlagPosition("DEBUG") >= 0;

    if (input == "HEALTH")
        World.Info.Health = World.Info.Health + 50;
    else if (input == "AMMO")
        World.Info.Ammo = World.Info.Ammo + 5;
    else if (input == "KEYS")
        for( i = 1; i <= 7; i ++) World.Info.Keys[i] = true;
    else if (input == "TORCHES")
        World.Info.Torches = World.Info.Torches + 3;
    else if (input == "TIME")
        World.Info.BoardTimeSec = World.Info.BoardTimeSec - 30;
    else if (input == "GEMS")
        World.Info.Gems = World.Info.Gems + 5;
    else if (input == "DARK")  {
        Board.Info.IsDark = toggle;
        TransitionDrawToBoard();
    } else if (input == "ZAP")  {
        for( i = 0; i <= 3; i ++) {
            BoardDamageTile(Board.Stats[0].X + NeighborDeltaX[i],
                            Board.Stats[0].Y + NeighborDeltaY[i]);
            Board.Tiles[Board.Stats[0].X + NeighborDeltaX[i]][Board.Stats[0].Y +
                    NeighborDeltaY[i]].Element = E_EMPTY;
            BoardDrawTile(Board.Stats[0].X + NeighborDeltaX[i],
                          Board.Stats[0].Y + NeighborDeltaY[i]);
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

void GamePlayLoop(boolean boardChanged);

static void GameDrawSidebar() {
    SidebarClear();
    SidebarClearLine(0);
    SidebarClearLine(1);
    SidebarClearLine(2);
    VideoWriteText(61, 0, 0x1f, "    - - - - -      ");
    VideoWriteText(62, 1, 0x70, "      ZZT      ");
    VideoWriteText(61, 2, 0x1f, "    - - - - -      ");
    if (GameStateElement == E_PLAYER)  {
        VideoWriteText(64, 7, 0x1e, " Health:");
        VideoWriteText(64, 8, 0x1e, "   Ammo:");
        VideoWriteText(64, 9, 0x1e, "Torches:");
        VideoWriteText(64, 10, 0x1e, "   Gems:");
        VideoWriteText(64, 11, 0x1e, "  Score:");
        VideoWriteText(64, 12, 0x1e, "   Keys:");
        VideoWriteText(62, 7, 0x1f, ElementDefs[E_PLAYER].Character);
        VideoWriteText(62, 8, 0x1b, ElementDefs[E_AMMO].Character);
        VideoWriteText(62, 9, 0x16, ElementDefs[E_TORCH].Character);
        VideoWriteText(62, 10, 0x1b, ElementDefs[E_GEM].Character);
        VideoWriteText(62, 12, 0x1f, ElementDefs[E_KEY].Character);
        VideoWriteText(62, 14, 0x70, " T ");
        VideoWriteText(65, 14, 0x1f, " Torch");
        VideoWriteText(62, 15, 0x30, " B ");
        VideoWriteText(62, 16, 0x70, " H ");
        VideoWriteText(65, 16, 0x1f, " Help");
        VideoWriteText(67, 18, 0x30, " \30\31\32\33 ");
        VideoWriteText(72, 18, 0x1f, " Move");
        VideoWriteText(61, 19, 0x70, " Shift \30\31\32\33 ");
        VideoWriteText(72, 19, 0x1f, " Shoot");
        VideoWriteText(62, 21, 0x70, " S ");
        VideoWriteText(65, 21, 0x1f, " Save game");
        VideoWriteText(62, 22, 0x30, " P ");
        VideoWriteText(65, 22, 0x1f, " Pause");
        VideoWriteText(62, 23, 0x70, " Q ");
        VideoWriteText(65, 23, 0x1f, " Quit");
    } else if (GameStateElement == E_MONITOR)  {
        SidebarPromptSlider(false, 66, 21, "Game speed:;FS", TickSpeed);
        VideoWriteText(62, 21, 0x70, " S ");
        VideoWriteText(62, 7, 0x30, " W ");
        VideoWriteText(65, 7, 0x1e, " World:");

        if (length(World.Info.Name) != 0)
            VideoWriteText(69, 8, 0x1f, World.Info.Name);
        else
            VideoWriteText(69, 8, 0x1f, "Untitled");

        VideoWriteText(62, 11, 0x70, " P ");
        VideoWriteText(65, 11, 0x1f, " Play");
        VideoWriteText(62, 12, 0x30, " R ");
        VideoWriteText(65, 12, 0x1e, " Restore game");
        VideoWriteText(62, 13, 0x70, " Q ");
        VideoWriteText(65, 13, 0x1e, " Quit");
        VideoWriteText(62, 16, 0x30, " A ");
        VideoWriteText(65, 16, 0x1f, " About ZZT!");
        VideoWriteText(62, 17, 0x70, " H ");
        VideoWriteText(65, 17, 0x1e, " High Scores");

        if (EditorEnabled)  {
            VideoWriteText(62, 18, 0x30, " E ");
            VideoWriteText(65, 18, 0x1e, " Board Editor");
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
            VideoWriteText(69, 8, 0x1f, StartupWorldFileName);
            if (! WorldLoad(StartupWorldFileName, ".ZZT"))  WorldCreate();
        }
        ReturnBoardId = World.Info.CurrentBoard;
        BoardChange(0);
        JustStarted = false;
    }

    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = GameStateElement;
    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color =
        ElementDefs[GameStateElement].Color;

    if (GameStateElement == E_MONITOR)  {
        DisplayMessage(0, "");
        VideoWriteText(62, 5, 0x1b, "Pick a command:");
    }

    if (boardChanged)
        TransitionDrawBoardChange();

    TickTimeDuration = TickSpeed * 2;
    GamePlayExitRequested = false;
    exitLoop = false;

    CurrentTick = Random(100);
    CurrentStatTicked = Board.StatCount + 1;

    pauseBlink = true;

    do {
        if (GamePaused)  {
            if (SoundHasTimeElapsed(TickTimeCounter, 25))
                pauseBlink = ! pauseBlink;

            if (pauseBlink)  {
                VideoWriteText(Board.Stats[0].X - 1, Board.Stats[0].Y - 1,
                               ElementDefs[E_PLAYER].Color, ElementDefs[E_PLAYER].Character);
            } else {
                if (Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element == E_PLAYER)
                    VideoWriteText(Board.Stats[0].X - 1, Board.Stats[0].Y - 1, 0xf, " ");
                else
                    BoardDrawTile(Board.Stats[0].X, Board.Stats[0].Y);
            }

            VideoWriteText(64, 5, 0x1f, "Pausing...");
            InputUpdate();

            if (InputKeyPressed == KEY_ESCAPE)
                GamePromptEndPlay();

            if ((InputDeltaX != 0) || (InputDeltaY != 0))  {
                ElementDefs[Board.Tiles[Board.Stats[0].X + InputDeltaX][Board.Stats[0].Y +
                                                         InputDeltaY].Element].TouchProc(
                                Board.Stats[0].X + InputDeltaX, Board.Stats[0].Y + InputDeltaY, 0,
                                InputDeltaX, InputDeltaY);
            }

            if (((InputDeltaX != 0) || (InputDeltaY != 0))
                    && ElementDefs[Board.Tiles[Board.Stats[0].X + InputDeltaX][Board.Stats[0].Y
                                                                + InputDeltaY].Element].Walkable) {
                /* Move player */
                if (Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element == E_PLAYER)
                    MoveStat(0, Board.Stats[0].X + InputDeltaX,
                             Board.Stats[0].Y + InputDeltaY);
                else {
                    BoardDrawTile(Board.Stats[0].X, Board.Stats[0].Y);
                    Board.Stats[0].X = Board.Stats[0].X + InputDeltaX;
                    Board.Stats[0].Y = Board.Stats[0].Y + InputDeltaY;
                    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = E_PLAYER;
                    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color =
                        ElementDefs[E_PLAYER].Color;
                    BoardDrawTile(Board.Stats[0].X, Board.Stats[0].Y);
                    DrawPlayerSurroundings(Board.Stats[0].X, Board.Stats[0].Y, 0);
                    DrawPlayerSurroundings(Board.Stats[0].X - InputDeltaX,
                                           Board.Stats[0].Y - InputDeltaY, 0);
                }

                /* Unpause */
                GamePaused = false;
                SidebarClearLine(5);
                CurrentTick = Random(100);
                CurrentStatTicked = Board.StatCount + 1;
                World.Info.IsSave = true;
            }

        } else {       /* not GamePaused */
            if (CurrentStatTicked <= Board.StatCount)  {
                {
                    TStat& with = Board.Stats[CurrentStatTicked];
                    if ((with.Cycle != 0)
                            && ((CurrentTick % with.Cycle) == (CurrentStatTicked % with.Cycle)))
                        ElementDefs[Board.Tiles[with.X][with.Y].Element].TickProc(
                            CurrentStatTicked);

                    CurrentStatTicked = CurrentStatTicked + 1;
                }
            }
        }

        if ((CurrentStatTicked > Board.StatCount) && ! GamePlayExitRequested)  {
            /* all stats ticked */
            if (SoundHasTimeElapsed(TickTimeCounter, TickTimeDuration))  {
                /* next cycle */
                CurrentTick = CurrentTick + 1;
                if (CurrentTick > 420)
                    CurrentTick = 1;
                CurrentStatTicked = 0;

                InputUpdate();
            }
        }
    } while (!((exitLoop || GamePlayExitRequested) && GamePlayExitRequested));

    SoundClearQueue();

    if (GameStateElement == E_PLAYER)  {
        if (World.Info.Health <= 0)  {
            HighScoresAdd(World.Info.Score);
        }
    } else if (GameStateElement == E_MONITOR)  {
        SidebarClearLine(5);
    }

    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = E_PLAYER;
    Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color =
        ElementDefs[E_PLAYER].Color;

    SoundBlockQueueing = false;
}

void GameTitleLoop() {
    boolean boardChanged;
    boolean startPlay;

    GameTitleExitRequested = false;
    JustStarted = true;
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

            switch (upcase(InputKeyPressed)) {
            case 'W': {
                if (GameWorldLoad(".ZZT"))  {
                    ReturnBoardId = World.Info.CurrentBoard;
                    boardChanged = true;
                }
            }
            break;
            case 'P': {
                if (World.Info.IsSave && ! DebugEnabled)  {
                    startPlay = WorldLoad(World.Info.Name, ".ZZT");
                    ReturnBoardId = World.Info.CurrentBoard;
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
                    ReturnBoardId = World.Info.CurrentBoard;
                    boardChanged = true;
                }
                break;
            case 'S': {
                SidebarPromptSlider(true, 66, 21, "Game speed:;FS", TickSpeed);
                InputKeyPressed = '\0';
            }
            break;
            case 'R': {
                if (GameWorldLoad(".SAV"))  {
                    ReturnBoardId = World.Info.CurrentBoard;
                    BoardChange(ReturnBoardId);
                    startPlay = true;
                }
            }
            break;
            case 'H': {
                HighScoresLoad();
                HighScoresDisplay(1);
            }
            break;
            case '|': {
                GameDebugPrompt();
            }
            break;
            case KEY_ESCAPE: case 'Q': {
                GameTitleExitRequested = SidebarPromptYesNo("Quit ZZT? ", true);
            }
            break;
            }

            if (startPlay)  {
                GameStateElement = E_PLAYER;
                GamePaused = true;
                GamePlayLoop(true);
                boardChanged = true;
            }
        } while (!(boardChanged || GameTitleExitRequested));
    } while (!GameTitleExitRequested);
}

void GamePrintRegisterMessage() {
    string s;
    untyped_file f;
    integer i;
    integer ix, iy;
    integer color;
    boolean isReading;
    pointer strPtr;

    SetCBreak(false);
    s = string("END") + chr(49 + Random(4)) + ".MSG";
    iy = 0;
    color = 0xf;

    for( i = 1; i <= ResourceDataHeader.EntryCount; i ++) {
        if (ResourceDataHeader.Name[i] == s)  {
            assign(f, ResourceDataFileName);
            OpenForRead(f, 1);
            seek(f, ResourceDataHeader.FileOffset[i]);

            isReading = true;
            while ((ioResult == 0) && isReading)  {
                BlockRead(f, s, 1);
                strPtr = &s;
                AdvancePointer(strPtr, 1);
                if (length(s) == 0)  {
                    color -= 1;
                } else {
                    BlockRead(f, strPtr, length(s));
                    if (s != '@')
                        VideoWriteText(0, iy, color, s);
                    else
                        isReading = false;
                }
                iy += 1;
            }

            close(f);
            VideoWriteText(28, 24, 0x1f, "Press any key to exit...");
            TextColor(LightGray);

            do {; } while (!keypressed());
            InputKeyPressed = readkey();

            VideoWriteText(28, 24, 0, "                        ");
            GotoXY(1, 23);
        }
    }
}

class unit_Game_initialize {
public: unit_Game_initialize();
};
static unit_Game_initialize Game_constructor;

unit_Game_initialize::unit_Game_initialize() {
    ;
}
