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
#define __Oop_implementation__


#include "oop.h"

#include "sounds.h"
#include "txtwind.h"
#include "game.h"
#include "elements.h"


void OopError(integer statId, string message) {
    {
        TStat& with = Board.Stats[statId];
        DisplayMessage(200, string("ERR: ") + message);
        SoundQueue(5, "\120\n");
        with.DataPos = -1;
    }
}

void OopReadChar(integer statId, integer& position) {
    {
        TStat& with = Board.Stats[statId];
        if ((position >= 0) && (position < with.DataLen))  {
            OopChar = *(with.Data + position);
            /*Move(* {{with.Data+position}}, OopChar, 1);*/
            position += 1;
        } else {
            OopChar = '\0';
        }
    }
}

void OopReadWord(integer statId, integer& position) {
    OopWord = "";
    do {
        OopReadChar(statId, position);
    } while (!(OopChar != ' '));
    OopChar = upcase(OopChar);
    if ((OopChar < '0') || (OopChar > '9'))  {
        while (((OopChar >= 'A') && (OopChar <= 'Z')) || (OopChar == ':')
                || ((OopChar >= '0') && (OopChar <= '9')) || (OopChar == '_')) {
            OopWord = OopWord + OopChar;
            OopReadChar(statId, position);
            OopChar = upcase(OopChar);
        }
    }
    if (position > 0)
        position -= 1;
}

void OopReadValue(integer statId, integer& position) {
    varying_string<20> s;
    integer code;

    s = "";
    do {
        OopReadChar(statId, position);
    } while (!(OopChar != ' '));

    OopChar = upcase(OopChar);
    while ((OopChar >= '0') && (OopChar <= '9'))  {
        s = s + OopChar;
        OopReadChar(statId, position);
        OopChar = upcase(OopChar);
    }

    if (position > 0)
        position = position - 1;

    if (length(s) != 0)
        val(s, OopValue, code);
    else
        OopValue = -1;
}

void OopSkipLine(integer statId, integer& position) {
    do {
        OopReadChar(statId, position);
    } while (!((OopChar == '\0') || (OopChar == '\15')));
}

boolean OopParseDirection(integer statId, integer& position, integer& dx,
                          integer& dy) {
    boolean OopParseDirection_result;
    {
        TStat& with = Board.Stats[statId];
        OopParseDirection_result = true;

        if ((OopWord == 'N') || (OopWord == "NORTH"))  {
            dx = 0;
            dy = -1;
        } else if ((OopWord == 'S') || (OopWord == "SOUTH"))  {
            dx = 0;
            dy = 1;
        } else if ((OopWord == 'E') || (OopWord == "EAST"))  {
            dx = 1;
            dy = 0;
        } else if ((OopWord == 'W') || (OopWord == "WEST"))  {
            dx = -1;
            dy = 0;
        } else if ((OopWord == 'I') || (OopWord == "IDLE"))  {
            dx = 0;
            dy = 0;
        } else if (OopWord == "SEEK")  {
            CalcDirectionSeek(with.X, with.Y, dx, dy);
        } else if (OopWord == "FLOW")  {
            dx = with.StepX;
            dy = with.StepY;
        } else if (OopWord == "RND")  {
            CalcDirectionRnd(dx, dy);
        } else if (OopWord == "RNDNS")  {
            dx = 0;
            dy = Random(2) * 2 - 1;
        } else if (OopWord == "RNDNE")  {
            dx = Random(2);
            if (dx == 0)  dy = -1;
            else dy = 0;
        } else if (OopWord == "CW")  {
            OopReadWord(statId, position);
            OopParseDirection_result = OopParseDirection(statId, position, dy, dx);
            dx = -dx;
        } else if (OopWord == "CCW")  {
            OopReadWord(statId, position);
            OopParseDirection_result = OopParseDirection(statId, position, dy, dx);
            dy = -dy;
        } else if (OopWord == "RNDP")  {
            OopReadWord(statId, position);
            OopParseDirection_result = OopParseDirection(statId, position, dy, dx);
            if (Random(2) == 0)
                dx = -dx;
            else
                dy = -dy;
        } else if (OopWord == "OPP")  {
            OopReadWord(statId, position);
            OopParseDirection_result = OopParseDirection(statId, position, dx, dy);
            dx = -dx;
            dy = -dy;
        } else {
            dx = 0;
            dy = 0;
            OopParseDirection_result = false;
        }
    }
    return OopParseDirection_result;
}

void OopReadDirection(integer statId, integer& position, integer& dx,
                      integer& dy) {
    OopReadWord(statId, position);
    if (! OopParseDirection(statId, position, dx, dy))
        OopError(statId, "Bad direction");
}

integer OopFindString(integer statId, string s) {
    integer pos, wordPos, cmpPos;


    integer OopFindString_result;
    {
        TStat& with = Board.Stats[statId];
        pos = 0;
        while (pos <= with.DataLen)  {
            wordPos = 1;
            cmpPos = pos;
            do {
                OopReadChar(statId, cmpPos);
                if (upcase(s[wordPos]) != upcase(OopChar))
                    goto LNoMatch;
                wordPos = wordPos + 1;
            } while (!(wordPos > length(s)));

            /* string matches */
            OopReadChar(statId, cmpPos);
            OopChar = upcase(OopChar);
            if (((OopChar >= 'A') && (OopChar <= 'Z')) || (OopChar == '_'))  {
                ;
                /* word continues, match invalid */
            } else {
                /* word complete, match valid */
                OopFindString_result = pos;
                return OopFindString_result;
            }

LNoMatch:
            pos = pos + 1;
        }
        OopFindString_result = -1;
    }
    return OopFindString_result;
}

boolean OopIterateStat(integer statId, integer& iStat, string lookup) {
    integer pos;
    boolean found;

    boolean OopIterateStat_result;
    iStat = iStat + 1;
    found = false;

    if (lookup == "ALL")  {
        if (iStat <= Board.StatCount)
            found = true;
    } else if (lookup == "OTHERS")  {
        if (iStat <= Board.StatCount)  {
            if (iStat != statId)
                found = true;
            else {
                iStat = iStat + 1;
                found = (iStat <= Board.StatCount);
            }
        }
    } else if (lookup == "SELF")  {
        if ((statId > 0) && (iStat <= statId))  {
            iStat = statId;
            found = true;
        }
    } else {
        while ((iStat <= Board.StatCount) && ! found)  {
            if (Board.Stats[iStat].Data != nil)  {
                pos = 0;
                OopReadChar(iStat, pos);
                if (OopChar == '@')  {
                    OopReadWord(iStat, pos);
                    if (OopWord == lookup)
                        found = true;
                }
            }

            if (! found)
                iStat = iStat + 1;
        }
    }

    OopIterateStat_result = found;
    return OopIterateStat_result;
}

boolean OopFindLabel(integer statId, string sendLabel, integer& iStat,
                     integer& iDataPos, string labelPrefix) {
    integer targetSplitPos;
    integer unk1;
    varying_string<20> targetLookup;
    varying_string<20> objectMessage;
    boolean foundStat;


    boolean OopFindLabel_result;
    foundStat = false;
    targetSplitPos = pos(":", sendLabel);
    if (targetSplitPos <= 0)  {
        /* if there is no target, we only check statId */
        if (iStat < statId)  {
            objectMessage = sendLabel;
            iStat = statId;
            targetSplitPos = 0;
            foundStat = true;
        }
    } else {
        targetLookup = copy(sendLabel, 1, targetSplitPos - 1);
        objectMessage = copy(sendLabel, targetSplitPos + 1,
                             length(sendLabel) - targetSplitPos);
LFindNextStat:
        foundStat = OopIterateStat(statId, iStat, targetLookup);
    }

    if (foundStat)  {
        if (objectMessage == "RESTART")  {
            iDataPos = 0;
        } else {
            iDataPos = OopFindString(iStat, labelPrefix + objectMessage);
            /* if lookup target exists, there may be more stats */
            if ((iDataPos < 0) && (targetSplitPos > 0))
                goto LFindNextStat;
        }
        foundStat = iDataPos >= 0;
    }

    OopFindLabel_result = foundStat;
    return OopFindLabel_result;
}

integer WorldGetFlagPosition(TString50 name) {
    integer i;

    integer WorldGetFlagPosition_result;
    WorldGetFlagPosition_result = -1;
    for( i = 1; i <= 10; i ++) {
        if (World.Info.Flags[i] == name)
            WorldGetFlagPosition_result = i;
    }
    return WorldGetFlagPosition_result;
}

void WorldSetFlag(TString50 name) {
    integer i;

    if (WorldGetFlagPosition(name) < 0)  {
        i = 1;
        while ((i < MAX_FLAG) && (length(World.Info.Flags[i]) != 0))
            i = i + 1;
        World.Info.Flags[i] = name;
    }
}

void WorldClearFlag(TString50 name) {
    integer i;

    if (WorldGetFlagPosition(name) >= 0)
        World.Info.Flags[WorldGetFlagPosition(name)] = "";
}

TString50 OopStringToWord(TString50 input) {
    varying_string<50> output;
    integer i;

    TString50 OopStringToWord_result;
    output = "";
    for( i = 1; i <= length(input); i ++) {
        if (((input[i] >= 'A') && (input[i] <= 'Z'))
                || ((input[i] >= '0') && (input[i] <= '9')))
            output = output + input[i];
        else if ((input[i] >= 'a') && (input[i] <= 'z'))
            output = output + chr(ord(input[i]) - 0x20);
    }
    OopStringToWord_result = output;
    return OopStringToWord_result;
}

boolean OopParseTile(integer& statId, integer& position, TTile& tile) {
    integer i;


    boolean OopParseTile_result;
    OopParseTile_result = false;
    tile.Color = 0;

    OopReadWord(statId, position);
    for( i = 1; i <= 7; i ++) {
        if (OopWord == OopStringToWord(ColorNames[i]))  {
            tile.Color = i + 0x8;
            OopReadWord(statId, position);
            goto LColorFound;
        }
    }
LColorFound:

    for( i = 0; i <= MAX_ELEMENT; i ++) {
        if (OopWord == OopStringToWord(ElementDefs[i].Name))  {
            OopParseTile_result = true;
            tile.Element = i;
            return OopParseTile_result;
        }
    }
    return OopParseTile_result;
}

byte GetColorForTileMatch(TTile& tile) {
    byte GetColorForTileMatch_result;
    if (ElementDefs[tile.Element].Color < COLOR_SPECIAL_MIN)
        GetColorForTileMatch_result = ElementDefs[tile.Element].Color & 0x7;
    else if (ElementDefs[tile.Element].Color == COLOR_WHITE_ON_CHOICE)
        GetColorForTileMatch_result = (((cardinal)tile.Color >> 4) & 0xf) + 8;
    else
        GetColorForTileMatch_result = (tile.Color & 0xf);
    return GetColorForTileMatch_result;
}

boolean FindTileOnBoard(integer& x, integer& y, TTile tile) {
    boolean FindTileOnBoard_result;
    FindTileOnBoard_result = false;
    while (true)  {
        x = x + 1;
        if (x > BOARD_WIDTH)  {
            x = 1;
            y = y + 1;
            if (y > BOARD_HEIGHT)
                return FindTileOnBoard_result;
        }

        if (Board.Tiles[x][y].Element == tile.Element)
            if ((tile.Color == 0)
                    || (GetColorForTileMatch(Board.Tiles[x][y]) == tile.Color))  {
                FindTileOnBoard_result = true;
                return FindTileOnBoard_result;
            }
    }
    return FindTileOnBoard_result;
}

void OopPlaceTile(integer x, integer y, TTile& tile) {
    byte color;

    if (Board.Tiles[x][y].Element != 4)  {
        color = tile.Color;
        if (ElementDefs[tile.Element].Color < COLOR_SPECIAL_MIN)
            color = ElementDefs[tile.Element].Color;
        else {
            if (color == 0)
                color = Board.Tiles[x][y].Color;

            if (color == 0)
                color = 0xf;

            if (ElementDefs[tile.Element].Color == COLOR_WHITE_ON_CHOICE)  {
                /*IMP: Fix range check error. Might produce slightly different
                colors if the original color was dark with a black background,
                			 but I'm considering that an acceptable deviation from exact
                			 bug-compatibility. Note that applying this twice
                			 (e.g. #put n blue door; #put n door always produces
                			 white as the color, just like in the original ZZT.*/
                color = color % 8;
                color = (color * 0x10) + 0xf;
            }
        }

        if (Board.Tiles[x][y].Element == tile.Element)
            Board.Tiles[x][y].Color = color;
        else {
            BoardDamageTile(x, y);
            if (ElementDefs[tile.Element].Cycle >= 0)  {
                AddStat(x, y, tile.Element, color, ElementDefs[tile.Element].Cycle,
                        StatTemplateDefault);
            } else {
                Board.Tiles[x][y].Element = tile.Element;
                Board.Tiles[x][y].Color = color;
            }
        }

        BoardDrawTile(x, y);
    }
}

boolean OopCheckCondition(integer statId, integer& position) {
    integer deltaX, deltaY;
    TTile tile;
    integer ix, iy;

    boolean OopCheckCondition_result;
    {
        TStat& with = Board.Stats[statId];
        if (OopWord == "NOT")  {
            OopReadWord(statId, position);
            OopCheckCondition_result = ! OopCheckCondition(statId, position);
        } else if (OopWord == "ALLIGNED")  {
            OopCheckCondition_result = (with.X == Board.Stats[0].X)
                                       || (with.Y == Board.Stats[0].Y);
        } else if (OopWord == "CONTACT")  {
            OopCheckCondition_result = (sqr(with.X - Board.Stats[0].X) + sqr(
                                            with.Y - Board.Stats[0].Y)) == 1;
        } else if (OopWord == "BLOCKED")  {
            OopReadDirection(statId, position, deltaX, deltaY);
            OopCheckCondition_result = ! ElementDefs[Board.Tiles[with.X +
                                                          deltaX][with.Y + deltaY].Element].Walkable;
        } else if (OopWord == "ENERGIZED")  {
            OopCheckCondition_result = World.Info.EnergizerTicks > 0;
        } else if (OopWord == "ANY")  {
            if (! OopParseTile(statId, position, tile))
                OopError(statId, "Bad object kind");

            ix = 0;
            iy = 1;
            OopCheckCondition_result = FindTileOnBoard(ix, iy, tile);
        } else {
            OopCheckCondition_result = WorldGetFlagPosition(OopWord) >= 0;
        }
    }
    return OopCheckCondition_result;
}

string OopReadLineToEnd(integer statId, integer& position) {
    string s;

    string OopReadLineToEnd_result;
    s = "";
    OopReadChar(statId, position);
    while ((OopChar != '\0') && (OopChar != '\15'))  {
        s = s + OopChar;
        OopReadChar(statId, position);
    }
    OopReadLineToEnd_result = s;
    return OopReadLineToEnd_result;
}

boolean OopSend(integer statId, string sendLabel, boolean ignoreLock) {
    integer iDataPos, iStat;
    boolean ignoreSelfLock;

    boolean OopSend_result;
    if (statId < 0)  {
        /* if statId is negative, label send will always succeed on self */
        /* this is used for in-game events (f.e. TOUCH, SHOT) */
        statId = -statId;
        ignoreSelfLock = true;
    } else {
        ignoreSelfLock = false;
    }

    OopSend_result = false;
    iStat = 0;

    while (OopFindLabel(statId, sendLabel, iStat, iDataPos, "\r:"))  {
        if (((Board.Stats[iStat].P2 == 0) || (ignoreLock)) || ((statId == iStat)
                && ! ignoreSelfLock))  {
            if (iStat == statId)
                OopSend_result = true;

            Board.Stats[iStat].DataPos = iDataPos;
        }
    }
    return OopSend_result;
}

void OopExecute(integer statId, integer& position, TString50 name) {
    TTextWindowState textWindow;
    string textLine;
    integer deltaX, deltaY;
    integer ix, iy;
    boolean stopRunning;
    boolean replaceStat;
    boolean endOfProgram;
    TTile replaceTile;
    integer namePosition;
    integer lastPosition;
    boolean repeatInsNextTick;
    boolean lineFinished;
    byte* labelPtr;
    integer labelDataPos;
    integer labelStatId;
    integer* counterPtr;
    boolean counterSubtract;
    integer bindStatId;
    integer insCount;
    TTile argTile;
    TTile argTile2;




    {
        TStat& with = Board.Stats[statId];
LStartParsing:
        TextWindowInitState(textWindow);
        textWindow.Selectable = false;
        stopRunning = false;
        repeatInsNextTick = false;
        replaceStat = false;
        endOfProgram = false;
        insCount = 0;
        do {
LReadInstruction:
            lineFinished = true;
            lastPosition = position;
            OopReadChar(statId, position);

            /* skip labels */
            while (OopChar == ':')  {
                do {
                    OopReadChar(statId, position);
                } while (!((OopChar == '\0') || (OopChar == '\15')));
                OopReadChar(statId, position);
            }

            if (OopChar == '\47') { /* apostrophe */
                OopSkipLine(statId, position);
            } else if (OopChar == '@')  {
                OopSkipLine(statId, position);
            } else if ((OopChar == '/') || (OopChar == '?'))  {
                if (OopChar == '/')
                    repeatInsNextTick = true;

                OopReadWord(statId, position);
                if (OopParseDirection(statId, position, deltaX, deltaY))  {
                    if ((deltaX != 0) || (deltaY != 0))  {
                        if (! ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                             deltaY].Element].Walkable)
                            ElementPushablePush(with.X + deltaX, with.Y + deltaY, deltaX, deltaY);

                        if (ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                           deltaY].Element].Walkable)  {
                            MoveStat(statId, with.X + deltaX, with.Y + deltaY);
                            repeatInsNextTick = false;
                        }
                    } else {
                        repeatInsNextTick = false;
                    }

                    OopReadChar(statId, position);
                    if (OopChar != '\15')
                        position -= 1;

                    stopRunning = true;
                } else {
                    OopError(statId, "Bad direction");
                }
            } else if (OopChar == '#')  {
LReadCommand:
                OopReadWord(statId, position);
                if (OopWord == "THEN")
                    OopReadWord(statId, position);
                if (length(OopWord) == 0)
                    goto LReadInstruction;
                insCount += 1;
                if (length(OopWord) != 0)  {
                    if (OopWord == "GO")  {
                        OopReadDirection(statId, position, deltaX, deltaY);

                        if (! ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                             deltaY].Element].Walkable)
                            ElementPushablePush(with.X + deltaX, with.Y + deltaY, deltaX, deltaY);

                        if (ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                           deltaY].Element].Walkable)  {
                            MoveStat(statId, with.X + deltaX, with.Y + deltaY);
                        } else {
                            repeatInsNextTick = true;
                        }

                        stopRunning = true;
                    } else if (OopWord == "TRY")  {
                        OopReadDirection(statId, position, deltaX, deltaY);

                        if (! ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                             deltaY].Element].Walkable)
                            ElementPushablePush(with.X + deltaX, with.Y + deltaY, deltaX, deltaY);

                        if (ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                           deltaY].Element].Walkable)  {
                            MoveStat(statId, with.X + deltaX, with.Y + deltaY);
                            stopRunning = true;
                        } else {
                            goto LReadCommand;
                        }
                    } else if (OopWord == "WALK")  {
                        OopReadDirection(statId, position, deltaX, deltaY);
                        with.StepX = deltaX;
                        with.StepY = deltaY;
                    } else if (OopWord == "SET")  {
                        OopReadWord(statId, position);
                        WorldSetFlag(OopWord);
                    } else if (OopWord == "CLEAR")  {
                        OopReadWord(statId, position);
                        WorldClearFlag(OopWord);
                    } else if (OopWord == "IF")  {
                        OopReadWord(statId, position);
                        if (OopCheckCondition(statId, position))
                            goto LReadCommand;
                    } else if (OopWord == "SHOOT")  {
                        OopReadDirection(statId, position, deltaX, deltaY);
                        if (BoardShoot(E_BULLET, with.X, with.Y, deltaX, deltaY,
                                       SHOT_SOURCE_ENEMY))
                            SoundQueue(2, "\60\1\46\1");
                        stopRunning = true;
                    } else if (OopWord == "THROWSTAR")  {
                        OopReadDirection(statId, position, deltaX, deltaY);
                        if (BoardShoot(E_STAR, with.X, with.Y, deltaX, deltaY, SHOT_SOURCE_ENEMY))
                        {; }
                        stopRunning = true;
                    } else if ((OopWord == "GIVE") || (OopWord == "TAKE"))  {
                        if (OopWord == "TAKE")
                            counterSubtract = true;
                        else
                            counterSubtract = false;

                        OopReadWord(statId, position);
                        if (OopWord == "HEALTH")
                            counterPtr = &World.Info.Health;
                        else if (OopWord == "AMMO")
                            counterPtr = &World.Info.Ammo;
                        else if (OopWord == "GEMS")
                            counterPtr = &World.Info.Gems;
                        else if (OopWord == "TORCHES")
                            counterPtr = &World.Info.Torches;
                        else if (OopWord == "SCORE")
                            counterPtr = &World.Info.Score;
                        else if (OopWord == "TIME")
                            counterPtr = &World.Info.BoardTimeSec;
                        else
                            counterPtr = nil;

                        if (counterPtr != nil)  {
                            OopReadValue(statId, position);
                            if (OopValue > 0)  {
                                if (counterSubtract)
                                    OopValue = -OopValue;

                                if ((*counterPtr + OopValue) >= 0)  {
                                    *counterPtr = *counterPtr + OopValue;
                                } else {
                                    goto LReadCommand;
                                }
                            }
                        }

                        GameUpdateSidebar();
                    } else if (OopWord == "END")  {
                        position = -1;
                        OopChar = '\0';
                    } else if (OopWord == "ENDGAME")  {
                        World.Info.Health = 0;
                    } else if (OopWord == "IDLE")  {
                        stopRunning = true;
                    } else if (OopWord == "RESTART")  {
                        position = 0;
                        lineFinished = false;
                    } else if (OopWord == "ZAP")  {
                        OopReadWord(statId, position);

                        labelStatId = 0;
                        while (OopFindLabel(statId, OopWord, labelStatId, labelDataPos, "\r:"))  {
                            labelPtr = Board.Stats[labelStatId].Data;
                            labelPtr += labelDataPos + 1;

                            *labelPtr = '\47';
                        }
                    } else if (OopWord == "RESTORE")  {
                        OopReadWord(statId, position);

                        labelStatId = 0;
                        while (OopFindLabel(statId, OopWord, labelStatId, labelDataPos, "\r\47"))
                            do {
                                labelPtr = Board.Stats[labelStatId].Data;
                                labelPtr += labelDataPos + 1;

                                *labelPtr = ':';

                                labelDataPos = OopFindString(labelStatId,
                                                             string("\r\47") + OopWord + '\15');
                            } while (!(labelDataPos <= 0));
                    } else if (OopWord == "LOCK")  {
                        with.P2 = 1;
                    } else if (OopWord == "UNLOCK")  {
                        with.P2 = 0;
                    } else if (OopWord == "SEND")  {
                        OopReadWord(statId, position);
                        if (OopSend(statId, OopWord, false))
                            lineFinished = false;
                    } else if (OopWord == "BECOME")  {
                        if (OopParseTile(statId, position, argTile))  {
                            replaceStat = true;
                            replaceTile.Element = argTile.Element;
                            replaceTile.Color = argTile.Color;
                        } else {
                            OopError(statId, "Bad #BECOME");
                        }
                    } else if (OopWord == "PUT")  {
                        OopReadDirection(statId, position, deltaX, deltaY);
                        if ((deltaX == 0) && (deltaY == 0))
                            OopError(statId, "Bad #PUT");
                        else if (! OopParseTile(statId, position, argTile))
                            OopError(statId, "Bad #PUT");
                        else if (((with.X + deltaX) > 0)
                                 && ((with.X + deltaX) <= BOARD_WIDTH)
                                 && ((with.Y + deltaY) > 0)
                                 && ((with.Y + deltaY) < BOARD_HEIGHT)) {
                            if (! ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
                                                                 deltaY].Element].Walkable)
                                ElementPushablePush(with.X + deltaX, with.Y + deltaY, deltaX, deltaY);

                            OopPlaceTile(with.X + deltaX, with.Y + deltaY, argTile);
                        }
                    } else if (OopWord == "CHANGE")  {
                        if (! OopParseTile(statId, position, argTile))
                            OopError(statId, "Bad #CHANGE");
                        if (! OopParseTile(statId, position, argTile2))
                            OopError(statId, "Bad #CHANGE");

                        ix = 0;
                        iy = 1;
                        if ((argTile2.Color == 0)
                                && (ElementDefs[argTile2.Element].Color < COLOR_SPECIAL_MIN))

                            argTile2.Color = ElementDefs[argTile2.Element].Color;

                        while (FindTileOnBoard(ix, iy, argTile))
                            OopPlaceTile(ix, iy, argTile2);
                    } else if (OopWord == "PLAY")  {
                        textLine = SoundParse(OopReadLineToEnd(statId, position));
                        if (length(textLine) != 0)
                            SoundQueue(-1, textLine);
                        lineFinished = false;
                    } else if (OopWord == "CYCLE")  {
                        OopReadValue(statId, position);
                        if (OopValue > 0)
                            with.Cycle = OopValue;
                    } else if (OopWord == "CHAR")  {
                        OopReadValue(statId, position);
                        if ((OopValue > 0) && (OopValue <= 255))  {
                            with.P1 = OopValue;
                            BoardDrawTile(with.X, with.Y);
                        }
                    } else if (OopWord == "DIE")  {
                        replaceStat = true;
                        replaceTile.Element = E_EMPTY;
                        replaceTile.Color = 0xf;
                    } else if (OopWord == "BIND")  {
                        OopReadWord(statId, position);
                        bindStatId = 0;
                        if (OopIterateStat(statId, bindStatId, OopWord))  {
                            FreeMem(with.Data, with.DataLen);
                            with.Data = Board.Stats[bindStatId].Data;
                            with.DataLen = Board.Stats[bindStatId].DataLen;
                            position = 0;
                        }
                    } else {
                        textLine = OopWord;
                        if (OopSend(statId, OopWord, false))  {
                            lineFinished = false;
                        } else {
                            if (pos(":", textLine) <= 0)  {
                                OopError(statId, string("Bad command ") + textLine);
                            }
                        }
                    }
                }

                if (lineFinished)
                    OopSkipLine(statId, position);
            } else if (OopChar == '\15')  {
                if (textWindow.LineCount > 0)
                    TextWindowAppend(textWindow, "");
            } else if (OopChar == '\0')  {
                endOfProgram = true;
            } else {
                /* The order of execution appears to be undefined for statements like
                  x := y + f(z) where y is a global variable and f alters it. Turbo Pascal
                  just happens to do it left-to-right, but FreePascal does not. Thus this
                  somewhat inelegant fix. (Beware global variables, folks.) */
                textLine = OopChar;
                textLine = textLine + OopReadLineToEnd(statId, position);
                TextWindowAppend(textWindow, textLine);
            }
        } while (!(endOfProgram || stopRunning || repeatInsNextTick || replaceStat
                   || (insCount > 32)));

        if (repeatInsNextTick)
            position = lastPosition;

        if (OopChar == '\0')
            position = -1;

        if (textWindow.LineCount > 1)  {
            namePosition = 0;
            OopReadChar(statId, namePosition);
            if (OopChar == '@')  {
                name = OopReadLineToEnd(statId, namePosition);
            }

            if (length(name) == 0)
                name = "Interaction";

            textWindow.Title = name;
            TextWindowDrawOpen(textWindow);
            TextWindowSelect(textWindow, true, false);
            TextWindowDrawClose(textWindow);
            TextWindowFree(textWindow);

            if (length(textWindow.Hyperlink) != 0)
                if (OopSend(statId, textWindow.Hyperlink, false))
                    goto LStartParsing;
        } else if (textWindow.LineCount == 1)  {
            DisplayMessage(200, *textWindow.Lines[1]);
            TextWindowFree(textWindow);
        }

        if (replaceStat)  {
            /*IMP: Fix runtime error with anything that destroys a
            scroll "ahead of time". */
            if (Board.Tiles[with.X][with.Y].Element == E_SCROLL)  return;

            ix = with.X;
            iy = with.Y;
            DamageStat(statId);
            OopPlaceTile(ix, iy, replaceTile);
        }
    }
}

class unit_Oop_initialize {
public: unit_Oop_initialize();
};
static unit_Oop_initialize Oop_constructor;

unit_Oop_initialize::unit_Oop_initialize() {
    ;
}
