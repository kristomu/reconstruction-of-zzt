
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

// This would be a good target for refactoring into objects.

/*$F+*/
/*$I-*/
#define __Elements_implementation__


#include "elements.h"
#include "ptoc.h"
#include "gamevars.h"
#include "board.h"

#include "world.h"
#include "video.h"
#include "sounds.h"
#include "txtwind.h"
#include "editor.h"
#include "oop.h"
#include "game.h"
#include "gamevars.h"

#include "hardware.h"

const std::string TransporterNSChars = "^~^-v_v-";
const std::string TransporterEWChars = "(<(\263)>)\263";
const std::string StarAnimChars = "\263/\304\\";

void ElementDefaultTick(integer statId) {
	;
}

void ElementDefaultTouch(integer x, integer y, integer sourceStatId,
                         integer& deltaX, integer& deltaY) {
	;
}

void ElementDefaultDraw(integer x, integer y, byte& ch) {
	ch = ord('?');
}

void ElementMessageTimerTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		switch (with.X) {
		case 0: {
			video.VideoWriteText((60 - Board.Info.Message.size()) / 2, 24,
			               9 + (with.P2 % 7), " " + Board.Info.Message + " " );
			with.P2 = with.P2 - 1;
			if (with.P2 <= 0)  {
				RemoveStat(statId);
				CurrentStatTicked = CurrentStatTicked - 1;
				BoardDrawBorder();
				Board.Info.Message = "";
			}
		}
		break;
		}
	}
}

void ElementDamagingTouch(integer x, integer y, integer sourceStatId,
                          integer& deltaX, integer& deltaY) {
	BoardAttack(sourceStatId, x, y);
}

void ElementLionTick(integer statId) {
	integer deltaX, deltaY;

	{
		TStat& with = Board.Stats[statId];
		if (with.P1 < Random(10))
			CalcDirectionRnd(deltaX, deltaY);
		else
			CalcDirectionSeek(with.X, with.Y, deltaX, deltaY);

		if (ElementDefs[Board.Tiles[with.X + deltaX][with.Y +
		                                   deltaY].Element].Walkable)  {
			MoveStat(statId, with.X + deltaX, with.Y + deltaY);
		} else if (Board.Tiles[with.X + deltaX][with.Y + deltaY].Element ==
		           E_PLAYER)  {
			BoardAttack(statId, with.X + deltaX, with.Y + deltaY);
		}
	}
}

void ElementTigerTick(integer statId) {
	boolean shot;
	byte element;

	{
		TStat& with = Board.Stats[statId];
		element = E_BULLET;
		if (with.P2 >= 0x80)
			element = E_STAR;

		if ((Random(10) * 3) <= (with.P2 % 0x80))  {
			if (Difference(with.X, Board.Stats[0].X) <= 2)  {
				shot = BoardShoot(element, with.X, with.Y, 0,
				                  Signum(Board.Stats[0].Y - with.Y), SHOT_SOURCE_ENEMY);
			} else {
				shot = false;
			}

			if (! shot)  {
				if (Difference(with.Y, Board.Stats[0].Y) <= 2)  {
					shot = BoardShoot(element, with.X, with.Y,
					                  Signum(Board.Stats[0].X - with.X), 0, SHOT_SOURCE_ENEMY);
				}
			}
		}

		ElementLionTick(statId);
	}
}

void ElementRuffianTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		if ((with.StepX == 0) && (with.StepY == 0))  {
			if ((with.P2 + 8) <= Random(17))  {
				if (with.P1 >= Random(9))
					CalcDirectionSeek(with.X, with.Y, with.StepX, with.StepY);
				else
					CalcDirectionRnd(with.StepX, with.StepY);
			}
		} else {
			if (((with.Y == Board.Stats[0].Y) || (with.X == Board.Stats[0].X))
			        && (Random(9) <= with.P1))  {
				CalcDirectionSeek(with.X, with.Y, with.StepX, with.StepY);
			}

			{
				TTile& with1 = Board.Tiles[with.X + with.StepX][with.Y + with.StepY];
				if (with1.Element == E_PLAYER)  {
					BoardAttack(statId, with.X + with.StepX, with.Y + with.StepY);
				} else if (ElementDefs[with1.Element].Walkable)  {
					MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
					if ((with.P2 + 8) <= Random(17))  {
						with.StepX = 0;
						with.StepY = 0;
					}
				} else {
					with.StepX = 0;
					with.StepY = 0;
				}
			}

		}
	}
}

void ElementBearTick(integer statId) {
	integer deltaX, deltaY;


	{
		TStat& with = Board.Stats[statId];
		if (with.X != Board.Stats[0].X)
			if (Difference(with.Y, Board.Stats[0].Y) <= (8 - with.P1))  {
				deltaX = Signum(Board.Stats[0].X - with.X);
				deltaY = 0;
				goto LMovement;
			}

		if (Difference(with.X, Board.Stats[0].X) <= (8 - with.P1))  {
			deltaY = Signum(Board.Stats[0].Y - with.Y);
			deltaX = 0;
		} else {
			deltaX = 0;
			deltaY = 0;
		}

LMovement: {
			TTile& with1 = Board.Tiles[with.X + deltaX][with.Y + deltaY];
			if (ElementDefs[with1.Element].Walkable)  {
				MoveStat(statId, with.X + deltaX, with.Y + deltaY);
			} else if ((with1.Element == E_PLAYER)
			           || (with1.Element == E_BREAKABLE))  {
				BoardAttack(statId, with.X + deltaX, with.Y + deltaY);
			}
		}

	}
}

void ElementCentipedeHeadTick(integer statId) {
	integer ix, iy;
	integer tx, ty;
	integer tmp;

	{
		TStat& with = Board.Stats[statId];
		if ((with.X == Board.Stats[0].X) && (Random(10) < with.P1))  {
			with.StepY = Signum(Board.Stats[0].Y - with.Y);
			with.StepX = 0;
		} else if ((with.Y == Board.Stats[0].Y) && (Random(10) < with.P1))  {
			with.StepX = Signum(Board.Stats[0].X - with.X);
			with.StepY = 0;
		} else if (((Random(10) * 4) < with.P2) || ((with.StepX == 0)
		           && (with.StepY == 0)))  {
			CalcDirectionRnd(with.StepX, with.StepY);
		}

		if (! ElementDefs[Board.Tiles[with.X + with.StepX][with.Y +
		                                     with.StepY].Element].Walkable
		        && (Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Element !=
		            E_PLAYER)) {
			ix = with.StepX;
			iy = with.StepY;
			tmp = ((Random(2) * 2) - 1) * with.StepY;
			with.StepY = ((Random(2) * 2) - 1) * with.StepX;
			with.StepX = tmp;
			if (! ElementDefs[Board.Tiles[with.X + with.StepX][with.Y +
			                                     with.StepY].Element].Walkable
			        && (Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Element !=
			            E_PLAYER)) {
				with.StepX = -with.StepX;
				with.StepY = -with.StepY;
				if (! ElementDefs[Board.Tiles[with.X + with.StepX][with.Y +
				                                     with.StepY].Element].Walkable
				        && (Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Element !=
				            E_PLAYER)) {
					if (ElementDefs[Board.Tiles[with.X - ix][with.Y - iy].Element].Walkable
					        || (Board.Tiles[with.X - ix][with.Y - iy].Element == E_PLAYER)) {
						with.StepX = -ix;
						with.StepY = -iy;
					} else {
						with.StepX = 0;
						with.StepY = 0;
					}
				}
			}
		}

		if ((with.StepX == 0) && (with.StepY == 0))  {
			Board.Tiles[with.X][with.Y].Element = E_CENTIPEDE_SEGMENT;
			with.Leader = -1;
			while (Board.Stats[statId].Follower > 0)  {
				tmp = Board.Stats[statId].Follower;
				Board.Stats[statId].Follower = Board.Stats[statId].Leader;
				Board.Stats[statId].Leader = tmp;
				statId = tmp;
			}
			Board.Stats[statId].Follower = Board.Stats[statId].Leader;
			Board.Tiles[Board.Stats[statId].X][Board.Stats[statId].Y].Element =
			    E_CENTIPEDE_HEAD;
		} else if (Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Element ==
		           E_PLAYER)  {
			if (with.Follower != -1)  {
				Board.Tiles[Board.Stats[with.Follower].X][Board.Stats[with.Follower].Y].Element
				    = E_CENTIPEDE_HEAD;
				Board.Stats[with.Follower].StepX = with.StepX;
				Board.Stats[with.Follower].StepY = with.StepY;
				BoardDrawTile(Board.Stats[with.Follower].X, Board.Stats[with.Follower].Y);
			}
			BoardAttack(statId, with.X + with.StepX, with.Y + with.StepY);
		} else {
			MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
			tx = with.X - with.StepX;
			ty = with.Y - with.StepY;
			ix = with.StepX;
			iy = with.StepY;

			do {
				{
					TStat& with1 = Board.Stats[statId];
					tx = with1.X - with1.StepX;
					ty = with1.Y - with1.StepY;
					ix = with1.StepX;
					iy = with1.StepY;
					if (with1.Follower < 0)  {
						if ((Board.Tiles[tx - ix][ty - iy].Element == E_CENTIPEDE_SEGMENT)
						        && (Board.Stats[GetStatIdAt(tx - ix, ty - iy)].Leader < 0)) {
							with1.Follower = GetStatIdAt(tx - ix, ty - iy);
						} else if ((Board.Tiles[tx - iy][ty - ix].Element == E_CENTIPEDE_SEGMENT)
						           && (Board.Stats[GetStatIdAt(tx - iy, ty - ix)].Leader < 0)) {
							with1.Follower = GetStatIdAt(tx - iy, ty - ix);
						} else if ((Board.Tiles[tx + iy][ty + ix].Element == E_CENTIPEDE_SEGMENT)
						           && (Board.Stats[GetStatIdAt(tx + iy, ty + ix)].Leader < 0)) {
							with1.Follower = GetStatIdAt(tx + iy, ty + ix);
						}
					}

					if (with1.Follower > 0)  {
						Board.Stats[with1.Follower].Leader = statId;
						Board.Stats[with1.Follower].P1 = with1.P1;
						Board.Stats[with1.Follower].P2 = with1.P2;
						Board.Stats[with1.Follower].StepX = tx - Board.Stats[with1.Follower].X;
						Board.Stats[with1.Follower].StepY = ty - Board.Stats[with1.Follower].Y;
						MoveStat(with1.Follower, tx, ty);
					}
					statId = with1.Follower;
				}
			} while (!(statId == -1));
		}
	}
}

void ElementCentipedeSegmentTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		if (with.Leader < 0)  {
			if (with.Leader < -1)
				Board.Tiles[with.X][with.Y].Element = E_CENTIPEDE_HEAD;
			else
				with.Leader = with.Leader - 1;
		}
	}
}

void ElementBulletTick(integer statId) {
	integer ix, iy;
	integer iStat;
	byte iElem;
	boolean firstTry;


	{
		TStat& with = Board.Stats[statId];
		firstTry = true;

LTryMove:
		ix = with.X + with.StepX;
		iy = with.Y + with.StepY;
		iElem = Board.Tiles[ix][iy].Element;

		if (ElementDefs[iElem].Walkable || (iElem == E_WATER))  {
			MoveStat(statId, ix, iy);
			return;
		}

		if ((iElem == E_RICOCHET) && firstTry)  {
			with.StepX = -with.StepX;
			with.StepY = -with.StepY;
			SoundQueue(1, "\371\1");
			firstTry = false;
			goto LTryMove;
			return;
		}

		if ((iElem == E_BREAKABLE)
		        || (ElementDefs[iElem].Destructible && ((iElem == E_PLAYER)
		                || (with.P1 == 0)))) {
			if (ElementDefs[iElem].ScoreValue != 0)  {
				World.Info.Score = World.Info.Score + ElementDefs[iElem].ScoreValue;
				GameUpdateSidebar();
			}
			BoardAttack(statId, ix, iy);
			return;
		}

		if ((Board.Tiles[with.X + with.StepY][with.Y + with.StepX].Element ==
		        E_RICOCHET) && firstTry)  {
			ix = with.StepX;
			with.StepX = -with.StepY;
			with.StepY = -ix;
			SoundQueue(1, "\371\1");
			firstTry = false;
			goto LTryMove;
			return;
		}

		if ((Board.Tiles[with.X - with.StepY][with.Y - with.StepX].Element ==
		        E_RICOCHET) && firstTry)  {
			ix = with.StepX;
			with.StepX = with.StepY;
			with.StepY = ix;
			SoundQueue(1, "\371\1");
			firstTry = false;
			goto LTryMove;
			return;
		}

		RemoveStat(statId);
		CurrentStatTicked = CurrentStatTicked - 1;
		if ((iElem == E_OBJECT) || (iElem == E_SCROLL))  {
			iStat = GetStatIdAt(ix, iy);
			if (OopSend(-iStat, "SHOT", false))  {; }
		}
	}
}

void ElementSpinningGunDraw(integer x, integer y, byte& ch) {
	switch (CurrentTick % 8) {
	case 0: case 1: ch = 24; break;
	case 2: case 3: ch = 26; break;
	case 4: case 5: ch = 25; break;
	default: ch = 27;
	}
}

void ElementLineDraw(integer x, integer y, byte& ch) {
	integer i, v, shift;

	v = 1;
	shift = 1;
	for( i = 0; i <= 3; i ++) {
		switch (Board.Tiles[x + NeighborDeltaX[i]][y +
		        NeighborDeltaY[i]].Element) {
		case E_LINE: case E_BOARD_EDGE: v = v + shift; break;
		}
		shift = shift << 1;
	}
	ch = LineChars[v-1];
}

void ElementSpinningGunTick(integer statId) {
	boolean shot;
	integer deltaX, deltaY;
	byte element;

	{
		TStat& with = Board.Stats[statId];
		BoardDrawTile(with.X, with.Y);

		element = E_BULLET;
		if (with.P2 >= 0x80)
			element = E_STAR;

		if (Random(9) < (with.P2 % 0x80))  {
			if (Random(9) <= with.P1)  {
				if (Difference(with.X, Board.Stats[0].X) <= 2)  {
					shot = BoardShoot(element, with.X, with.Y, 0,
					                  Signum(Board.Stats[0].Y - with.Y), SHOT_SOURCE_ENEMY);
				} else {
					shot = false;
				}

				if (! shot)  {
					if (Difference(with.Y, Board.Stats[0].Y) <= 2)  {
						shot = BoardShoot(element, with.X, with.Y,
						                  Signum(Board.Stats[0].X - with.X), 0, SHOT_SOURCE_ENEMY);
					}
				}
			} else {
				CalcDirectionRnd(deltaX, deltaY);
				shot = BoardShoot(element, with.X, with.Y, deltaX, deltaY,
				                  SHOT_SOURCE_ENEMY);
			}
		}
	}
}

void ElementConveyorTick(integer x, integer y, integer direction) {
	integer i;
	integer iStat;
	integer srcx, srcy;
	integer destx, desty;
	boolean canMove;
	array<0,7,TTile> tiles;
	array<0,7,integer> statsIndices;      /*IMP: Fix stat clobbering bug*/
	integer iMin, iMax;
	TTile tmpTile;

	if (direction == 1)  {
		iMin = 0;
		iMax = 8;
	} else {
		iMin = 7;
		iMax = -1;
	}

	canMove = true;
	i = iMin;
	do {
		tiles[i] = Board.Tiles[x + DiagonalDeltaX[i]][y + DiagonalDeltaY[i]];
		statsIndices[i] = GetStatIdAt(x + DiagonalDeltaX[i],
		                              y + DiagonalDeltaY[i]);
		{
			TTile& with = tiles[i];
			if (with.Element == E_EMPTY)
				canMove = true;
			else if (! ElementDefs[with.Element].Pushable)
				canMove = false;
		}
		i = i + direction;
	} while (!(i == iMax));

	i = iMin;
	do {
		{
			TTile& with = tiles[i];
			if (canMove)  {
				if (ElementDefs[with.Element].Pushable)  {
					srcx = x + DiagonalDeltaX[i];
					srcy = y + DiagonalDeltaY[i];

					destx = x + DiagonalDeltaX[(i - direction + 8) % 8];
					desty = y + DiagonalDeltaY[(i - direction + 8) % 8];

					if (ElementDefs[with.Element].Cycle > -1)  {
						tmpTile = Board.Tiles[srcx][srcy];
						iStat = statsIndices[i];
						Board.Tiles[srcx][srcy] = tiles[i];
						Board.Tiles[destx][desty].Element = E_EMPTY;
						MoveStat(iStat, destx, desty);
						Board.Tiles[srcx][srcy] = tmpTile;
					} else {
						Board.Tiles[destx][desty] = tiles[i];
					}
					if (! ElementDefs[tiles[(i + direction + 8) % 8].Element].Pushable)  {
						Board.Tiles[srcx][srcy].Element = E_EMPTY;
					}
				} else {
					canMove = false;
				}
			} else if (with.Element == E_EMPTY)
				canMove = true;
			else if (! ElementDefs[with.Element].Pushable)
				canMove = false;
		}
		i = i + direction;
	} while (!(i == iMax));

	/* Draw everything to be sure that every char is updated. Doing
	BoardDraw inside the loop above can lead to tiles at some
		  relative coordinates not getting drawn. */
	for( i = 0; i <= DiagonalDeltaX.size()-1; i ++)
		BoardDrawTile(x + DiagonalDeltaX[i], y + DiagonalDeltaY[i]);
}

void ElementConveyorCWDraw(integer x, integer y, byte& ch) {
	switch ((CurrentTick / ElementDefs[E_CONVEYOR_CW].Cycle) % 4) {
	case 0: ch = 179; break;
	case 1: ch = 47; break;
	case 2: ch = 196; break;
	default: ch = 92;
	}
}

void ElementConveyorCWTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		BoardDrawTile(with.X, with.Y);
		ElementConveyorTick(with.X, with.Y, 1);
	}
}

void ElementConveyorCCWDraw(integer x, integer y, byte& ch) {
	switch ((CurrentTick / ElementDefs[E_CONVEYOR_CCW].Cycle) % 4) {
	case 3: ch = 179; break;
	case 2: ch = 47; break;
	case 1: ch = 196; break;
	default: ch = 92;
	}
}

void ElementConveyorCCWTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		BoardDrawTile(with.X, with.Y);
		ElementConveyorTick(with.X, with.Y, -1);
	}
}

void ElementBombDraw(integer x, integer y, byte& ch) {
	{
		TStat& with = Board.Stats[GetStatIdAt(x, y)];
		if (with.P1 <= 1)
			ch = 11;
		else
			ch = 48 + with.P1;
	}
}

void ElementBombTick(integer statId) {
	integer oldX, oldY;

	{
		TStat& with = Board.Stats[statId];
		if (with.P1 > 0)  {
			with.P1 = with.P1 - 1;
			BoardDrawTile(with.X, with.Y);

			if (with.P1 == 1)  {
				SoundQueue(1, "\140\1\120\1\100\1\60\1\40\1\20\1");
				DrawPlayerSurroundings(with.X, with.Y, 1);
			} else if (with.P1 == 0)  {
				oldX = with.X;
				oldY = with.Y;
				RemoveStat(statId);
				DrawPlayerSurroundings(oldX, oldY, 2);
			} else {
				if ((with.P1 % 2) == 0)
					SoundQueue(1, "\370\1");
				else
					SoundQueue(1, "\365\1");
			}
		}
	}
}

void ElementBombTouch(integer x, integer y, integer sourceStatId,
                      integer& deltaX, integer& deltaY) {
	{
		TStat& with = Board.Stats[GetStatIdAt(x, y)];
		if (with.P1 == 0)  {
			with.P1 = 9;
			BoardDrawTile(with.X, with.Y);
			DisplayMessage(200, "Bomb activated!");
			SoundQueue(4, "\60\1\65\1\100\1\105\1\120\1");
		} else {
			ElementPushablePush(with.X, with.Y, deltaX, deltaY);
		}
	}
}

void ElementTransporterMove(integer x, integer y, integer deltaX,
                            integer deltaY) {
	integer ix, iy;
	integer newX, newY;
	integer iStat;
	boolean finishSearch;
	boolean isValidDest;

	{
		TStat& with = Board.Stats[GetStatIdAt(x + deltaX, y + deltaY)];
		if ((deltaX == with.StepX) && (deltaY == with.StepY))  {
			ix = with.X;
			iy = with.Y;
			newX = -1;
			finishSearch = false;
			isValidDest = true;
			do {
				ix = ix + deltaX;
				iy = iy + deltaY;
				{
					TTile& with1 = Board.Tiles[ix][iy];
					if (with1.Element == E_BOARD_EDGE)
						finishSearch = true;
					else if (isValidDest)  {
						isValidDest = false;

						if (! ElementDefs[with1.Element].Walkable)
							ElementPushablePush(ix, iy, deltaX, deltaY);

						if (ElementDefs[with1.Element].Walkable)  {
							finishSearch = true;
							newX = ix;
							newY = iy;
						} else {
							newX = -1;
						}
					}
					if (with1.Element == E_TRANSPORTER)  {
						iStat = GetStatIdAt(ix, iy);
						if ((Board.Stats[iStat].StepX == -deltaX)
						        && (Board.Stats[iStat].StepY == -deltaY))
							isValidDest = true;
					}
				}
			} while (!finishSearch);
			if (newX != -1)  {
				ElementMove(with.X - deltaX, with.Y - deltaY, newX, newY);
				SoundQueue(3, "\60\1\102\1\64\1\106\1\70\1\112\1\100\1\122\1");
			}
		}
	}
}

void ElementTransporterTouch(integer x, integer y, integer sourceStatId,
                             integer& deltaX, integer& deltaY) {
	ElementTransporterMove(x - deltaX, y - deltaY, deltaX, deltaY);
	deltaX = 0;
	deltaY = 0;
}

void ElementTransporterTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		BoardDrawTile(with.X, with.Y);
	}
}

void ElementTransporterDraw(integer x, integer y, byte& ch) {
	{
		TStat& with = Board.Stats[GetStatIdAt(x, y)];
		if (with.StepX == 0)
			ch = ord(TransporterNSChars[with.StepY * 2 + 3 + (CurrentTick / with.Cycle)
			                                       % 4-1]);
		else
			ch = ord(TransporterEWChars[with.StepX * 2 + 3 + (CurrentTick / with.Cycle)
			                                       % 4-1]);
	}
}

void ElementStarDraw(integer x, integer y, byte& ch) {
	ch = ord(StarAnimChars[(CurrentTick % 4) + 1-1]);
	Board.Tiles[x][y].Color = Board.Tiles[x][y].Color + 1;
	if (Board.Tiles[x][y].Color > 15)
		Board.Tiles[x][y].Color = 9;
}

void ElementStarTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		with.P2 = with.P2 - 1;
		if (with.P2 <= 0)  {
			RemoveStat(statId);
		} else if ((with.P2 % 2) == 0)  {
			CalcDirectionSeek(with.X, with.Y, with.StepX, with.StepY);
			{
				TTile& with1 = Board.Tiles[with.X + with.StepX][with.Y + with.StepY];
				if ((with1.Element == E_PLAYER) || (with1.Element == E_BREAKABLE))  {
					BoardAttack(statId, with.X + with.StepX, with.Y + with.StepY);
				} else {
					if (! ElementDefs[with1.Element].Walkable)
						ElementPushablePush(with.X + with.StepX, with.Y + with.StepY, with.StepX,
						                    with.StepY);

					if (ElementDefs[with1.Element].Walkable || (with1.Element == E_WATER))
						MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
				}
			}
		} else {
			BoardDrawTile(with.X, with.Y);
		}
	}
}

void ElementEnergizerTouch(integer x, integer y, integer sourceStatId,
                           integer& deltaX, integer& deltaY) {
	SoundQueue(9, string("\40\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3")
	           + "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
	           + "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
	           + "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
	           + "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
	           + "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
	           + "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3");

	Board.Tiles[x][y].Element = E_EMPTY;
	BoardDrawTile(x, y);

	World.Info.EnergizerTicks = 75;
	GameUpdateSidebar();

	if (MessageEnergizerNotShown)  {
		DisplayMessage(200, "Energizer - You are invincible");
		MessageEnergizerNotShown = false;
	}

	if (OopSend(0, "ALL:ENERGIZE", false))  {; }
}

void ElementSlimeTick(integer statId) {
	integer dir, color, changedTiles;
	integer startX, startY;

	{
		TStat& with = Board.Stats[statId];
		if (with.P1 < with.P2)
			with.P1 = with.P1 + 1;
		else {
			color = Board.Tiles[with.X][with.Y].Color;
			with.P1 = 0;
			startX = with.X;
			startY = with.Y;
			changedTiles = 0;

			for( dir = 0; dir <= 3; dir ++) {
				if (ElementDefs[Board.Tiles[startX + NeighborDeltaX[dir]][startY +
				                                   NeighborDeltaY[dir]].Element].Walkable)  {
					if (changedTiles == 0)  {
						MoveStat(statId, startX + NeighborDeltaX[dir],
						         startY + NeighborDeltaY[dir]);
						Board.Tiles[startX][startY].Color = color;
						Board.Tiles[startX][startY].Element = E_BREAKABLE;
						BoardDrawTile(startX, startY);
					} else {
						AddStat(startX + NeighborDeltaX[dir], startY + NeighborDeltaY[dir],
						        E_SLIME, color,
						        ElementDefs[E_SLIME].Cycle, StatTemplateDefault);
						Board.Stats[Board.StatCount].P2 = with.P2;
					}

					changedTiles = changedTiles + 1;
				}
			}

			if (changedTiles == 0)  {
				RemoveStat(statId);
				Board.Tiles[startX][startY].Element = E_BREAKABLE;
				Board.Tiles[startX][startY].Color = color;
				BoardDrawTile(startX, startY);
			}
		}
	}
}

void ElementSlimeTouch(integer x, integer y, integer sourceStatId,
                       integer& deltaX, integer& deltaY) {
	integer color;

	color = Board.Tiles[x][y].Color;
	DamageStat(GetStatIdAt(x, y));
	Board.Tiles[x][y].Element = E_BREAKABLE;
	Board.Tiles[x][y].Color = color;
	BoardDrawTile(x, y);
	SoundQueue(2, "\40\1\43\1");
}

void ElementSharkTick(integer statId) {
	integer deltaX, deltaY;

	{
		TStat& with = Board.Stats[statId];
		if (with.P1 < Random(10))
			CalcDirectionRnd(deltaX, deltaY);
		else
			CalcDirectionSeek(with.X, with.Y, deltaX, deltaY);

		if (Board.Tiles[with.X + deltaX][with.Y + deltaY].Element == E_WATER)
			MoveStat(statId, with.X + deltaX, with.Y + deltaY);
		else if (Board.Tiles[with.X + deltaX][with.Y + deltaY].Element == E_PLAYER)
			BoardAttack(statId, with.X + deltaX, with.Y + deltaY);
	}
}

void ElementBlinkWallDraw(integer x, integer y, byte& ch) {
	ch = 206;
}

void ElementBlinkWallTick(integer statId) {
	integer ix, iy;
	boolean hitBoundary;
	integer playerStatId;
	integer el;

	{
		TStat& with = Board.Stats[statId];
		if (with.P3 == 0)
			with.P3 = with.P1 + 1;
		if (with.P3 == 1)  {
			ix = with.X + with.StepX;
			iy = with.Y + with.StepY;

			if (with.StepX != 0)
				el = E_BLINK_RAY_EW;
			else
				el = E_BLINK_RAY_NS;

			while ((Board.Tiles[ix][iy].Element == el)
			        && (Board.Tiles[ix][iy].Color == Board.Tiles[with.X][with.Y].Color)) {
				Board.Tiles[ix][iy].Element = E_EMPTY;
				BoardDrawTile(ix, iy);
				ix = ix + with.StepX;
				iy = iy + with.StepY;
				with.P3 = (with.P2) * 2 + 1;
			}

			if (((with.X + with.StepX) == ix) && ((with.Y + with.StepY) == iy))  {
				hitBoundary = false;
				do {
					if ((Board.Tiles[ix][iy].Element != E_EMPTY)
					        && (ElementDefs[Board.Tiles[ix][iy].Element].Destructible))
						BoardDamageTile(ix, iy);

					if (Board.Tiles[ix][iy].Element == E_PLAYER)  {
						playerStatId = GetStatIdAt(ix, iy);
						if (with.StepX != 0)  {
							if (Board.Tiles[ix][iy - 1].Element == E_EMPTY)
								MoveStat(playerStatId, ix, iy - 1);
							else if (Board.Tiles[ix][iy + 1].Element == E_EMPTY)
								MoveStat(playerStatId, ix, iy + 1);
						} else {
							if (Board.Tiles[ix + 1][iy].Element == E_EMPTY)
								MoveStat(playerStatId, ix + 1, iy);
							else if (Board.Tiles[ix - 1][iy].Element == E_EMPTY)
								MoveStat(playerStatId, ix + 1, iy);
						}

						if (Board.Tiles[ix][iy].Element == E_PLAYER)  {
							while (World.Info.Health > 0)
								DamageStat(playerStatId);
							hitBoundary = true;
						}
					}

					if (Board.Tiles[ix][iy].Element == E_EMPTY)  {
						Board.Tiles[ix][iy].Element = el;
						Board.Tiles[ix][iy].Color = Board.Tiles[with.X][with.Y].Color;
						BoardDrawTile(ix, iy);
					} else {
						hitBoundary = true;
					}

					ix = ix + with.StepX;
					iy = iy + with.StepY;
				} while (!hitBoundary);

				with.P3 = (with.P2 * 2) + 1;
			}
		} else {
			with.P3 = with.P3 - 1;
		}
	}
}

void ElementMove(integer oldX, integer oldY, integer newX, integer newY) {
	integer statId;

	statId = GetStatIdAt(oldX, oldY);

	if (statId >= 0)  {
		MoveStat(statId, newX, newY);
	} else {
		Board.Tiles[newX][newY] = Board.Tiles[oldX][oldY];
		BoardDrawTile(newX, newY);
		Board.Tiles[oldX][oldY].Element = E_EMPTY;
		BoardDrawTile(oldX, oldY);
	}
}

void ElementPushablePush(integer x, integer y, integer deltaX,
                         integer deltaY) {
	integer unk1;

	/* IMP: Fix infinite regression when trying to push something
	onto itself.*/
	if ((deltaX == 0) && (deltaY == 0))  return;

	{
		TTile& with = Board.Tiles[x][y];
		if (((with.Element == E_SLIDER_NS) && (deltaX == 0))
		        || ((with.Element == E_SLIDER_EW) && (deltaY == 0))
		        || ElementDefs[with.Element].Pushable) {
			if (Board.Tiles[x + deltaX][y + deltaY].Element == E_TRANSPORTER)
				ElementTransporterMove(x, y, deltaX, deltaY);
			else if (Board.Tiles[x + deltaX][y + deltaY].Element != E_EMPTY)
				ElementPushablePush(x + deltaX, y + deltaY, deltaX, deltaY);

			if (! ElementDefs[Board.Tiles[x + deltaX][y + deltaY].Element].Walkable
			        && ElementDefs[Board.Tiles[x + deltaX][y + deltaY].Element].Destructible
			        && (Board.Tiles[x + deltaX][y + deltaY].Element != E_PLAYER)) {
				BoardDamageTile(x + deltaX, y + deltaY);
			}

			if (ElementDefs[Board.Tiles[x + deltaX][y + deltaY].Element].Walkable)
				ElementMove(x, y, x + deltaX, y + deltaY);
		}
	}
}

void ElementDuplicatorDraw(integer x, integer y, byte& ch) {
	{
		TStat& with = Board.Stats[GetStatIdAt(x, y)];
		switch (with.P1) {
		case 1: ch = 250; break;
		case 2: ch = 249; break;
		case 3: ch = 248; break;
		case 4: ch = 111; break;
		case 5: ch = 79; break;
		default: ch = 250;
		}
	}
}

void ElementObjectTick(integer statId) {
	boolean retVal;

	{
		TStat& with = Board.Stats[statId];
		if (with.DataPos >= 0)
			OopExecute(statId, with.DataPos, "Interaction");

		if ((with.StepX != 0) || (with.StepY != 0))  {
			if (ElementDefs[Board.Tiles[with.X + with.StepX][with.Y +
			                                   with.StepY].Element].Walkable)
				MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
			else
				retVal = OopSend(-statId, "THUD", false);
		}
	}
}

void ElementObjectDraw(integer x, integer y, byte& ch) {
	ch = Board.Stats[GetStatIdAt(x, y)].P1;
}

void ElementObjectTouch(integer x, integer y, integer sourceStatId,
                        integer& deltaX, integer& deltaY) {
	integer statId;
	boolean retVal;

	statId = GetStatIdAt(x, y);
	retVal = OopSend(-statId, "TOUCH", false);
}

void ElementDuplicatorTick(integer statId) {
	integer sourceStatId;

	{
		TStat& with = Board.Stats[statId];
		if (with.P1 <= 4)  {
			with.P1 = with.P1 + 1;
			BoardDrawTile(with.X, with.Y);
		} else {
			with.P1 = 0;
			if (Board.Tiles[with.X - with.StepX][with.Y - with.StepY].Element ==
			        E_PLAYER)  {
				ElementDefs[Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Element]
				.TouchProc(with.X + with.StepX, with.Y + with.StepY, 0, InputDeltaX,
				           InputDeltaY);
			} else {
				if (Board.Tiles[with.X - with.StepX][with.Y - with.StepY].Element !=
				        E_EMPTY)
					ElementPushablePush(with.X - with.StepX, with.Y - with.StepY, -with.StepX,
					                    -with.StepY);

				if (Board.Tiles[with.X - with.StepX][with.Y - with.StepY].Element ==
				        E_EMPTY)  {
					sourceStatId = GetStatIdAt(with.X + with.StepX, with.Y + with.StepY);
					if (sourceStatId > 0)  {
						if (Board.StatCount < 174)  {
							AddStat(with.X - with.StepX, with.Y - with.StepY,
							        Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Element,
							        Board.Tiles[with.X + with.StepX][with.Y + with.StepY].Color,
							        Board.Stats[sourceStatId].Cycle, Board.Stats[sourceStatId]);
							BoardDrawTile(with.X - with.StepX, with.Y - with.StepY);
						}
					} else if (sourceStatId != 0)  {
						Board.Tiles[with.X - with.StepX][with.Y - with.StepY]
						    = Board.Tiles[with.X + with.StepX][with.Y + with.StepY];
						BoardDrawTile(with.X - with.StepX, with.Y - with.StepY);
					}

					SoundQueue(3, "\60\2\62\2\64\2\65\2\67\2");
				} else {
					SoundQueue(3, "\30\1\26\1");
				}
			}

			with.P1 = 0;
			BoardDrawTile(with.X, with.Y);
		}

		with.Cycle = (9 - with.P2) * 3;
	}
}

void ElementScrollTick(integer statId) {
	{
		TStat& with = Board.Stats[statId];
		Board.Tiles[with.X][with.Y].Color = Board.Tiles[with.X][with.Y].Color + 1;
		if (Board.Tiles[with.X][with.Y].Color > 0xf)
			Board.Tiles[with.X][with.Y].Color = 0x9;

		BoardDrawTile(with.X, with.Y);
	}
}

void ElementScrollTouch(integer x, integer y, integer sourceStatId,
                        integer& deltaX, integer& deltaY) {
	TTextWindowState textWindow;
	integer statId;
	integer unk1;

	statId = GetStatIdAt(x, y);

	{
		TStat& with = Board.Stats[statId];
		textWindow.Selectable = false;
		textWindow.LinePos = 1;

		SoundQueue(2, SoundParse("c-c+d-d+e-e+f-f+g-g"));

		with.DataPos = 0;
		OopExecute(statId, with.DataPos, "Scroll");
	}

	RemoveStat(GetStatIdAt(x, y));
}

void ElementKeyTouch(integer x, integer y, integer sourceStatId,
                     integer& deltaX, integer& deltaY) {
	integer key;

	key = Board.Tiles[x][y].Color % 8;

	if (World.Info.HasKey(key))  {
		DisplayMessage(200, string("You already have a ")+World.Info.KeyName(key).c_str()+" key!");
		SoundQueue(2, "\60\2\40\2");
	} else {
		World.Info.GiveKey(key);
		Board.Tiles[x][y].Element = E_EMPTY;
		GameUpdateSidebar();
		DisplayMessage(200, string("You now have the ")+World.Info.KeyName(key).c_str()+" key.");
		SoundQueue(2,
		           "\100\1\104\1\107\1\100\1\104\1\107\1\100\1\104\1\107\1\120\2");
	}
}

void ElementAmmoTouch(integer x, integer y, integer sourceStatId,
                      integer& deltaX, integer& deltaY) {
	World.Info.Ammo = World.Info.Ammo + 5;

	Board.Tiles[x][y].Element = E_EMPTY;
	GameUpdateSidebar();
	SoundQueue(2, "\60\1\61\1\62\1");

	if (MessageAmmoNotShown)  {
		MessageAmmoNotShown = false;
		DisplayMessage(200, "Ammunition - 5 shots per container.");
	}
}

void ElementGemTouch(integer x, integer y, integer sourceStatId,
                     integer& deltaX, integer& deltaY) {
	World.Info.Gems = World.Info.Gems + 1;
	World.Info.Health = World.Info.Health + 1;
	World.Info.Score = World.Info.Score + 10;

	Board.Tiles[x][y].Element = E_EMPTY;
	GameUpdateSidebar();
	SoundQueue(2, "\100\1\67\1\64\1\60\1");

	if (MessageGemNotShown)  {
		MessageGemNotShown = false;
		DisplayMessage(200, "Gems give you Health!");
	}
}

void ElementPassageTouch(integer x, integer y, integer sourceStatId,
                         integer& deltaX, integer& deltaY) {
	BoardPassageTeleport(x, y);
	deltaX = 0;
	deltaY = 0;
}

void ElementDoorTouch(integer x, integer y, integer sourceStatId,
                      integer& deltaX, integer& deltaY) {
	integer key;

	key = (Board.Tiles[x][y].Color / 16) % 8;

	if (World.Info.HasKey(key))  {
		Board.Tiles[x][y].Element = E_EMPTY;
		BoardDrawTile(x, y);

		World.Info.TakeKey(key);
		GameUpdateSidebar();

		DisplayMessage(200, string("The ")+World.Info.KeyName(key).c_str()+" door is now open.");
		SoundQueue(3, "\60\1\67\1\73\1\60\1\67\1\73\1\100\4");
	} else {
		DisplayMessage(200, string("The ")+World.Info.KeyName(key).c_str()+" door is locked!");
		SoundQueue(3, "\27\1\20\1");
	}
}

void ElementPushableTouch(integer x, integer y, integer sourceStatId,
                          integer& deltaX, integer& deltaY) {
	ElementPushablePush(x, y, deltaX, deltaY);
	SoundQueue(2, "\25\1");
}

void ElementPusherDraw(integer x, integer y, byte& ch) {
	{
		TStat& with = Board.Stats[GetStatIdAt(x, y)];
		if (with.StepX == 1)
			ch = 16;
		else if (with.StepX == -1)
			ch = 17;
		else if (with.StepY == -1)
			ch = 30;
		else
			ch = 31;
	}
}

void ElementPusherTick(integer statId) {
	integer i, startX, startY;

	{
		TStat& with = Board.Stats[statId];
		startX = with.X;
		startY = with.Y;

		if (! ElementDefs[Board.Tiles[with.X + with.StepX][with.Y +
		                                     with.StepY].Element].Walkable)  {
			ElementPushablePush(with.X + with.StepX, with.Y + with.StepY, with.StepX,
			                    with.StepY);
		}
	}

	statId = GetStatIdAt(startX, startY);
	{
		TStat& with = Board.Stats[statId];
		if (ElementDefs[Board.Tiles[with.X + with.StepX][with.Y +
		                                   with.StepY].Element].Walkable)  {
			MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
			SoundQueue(2, "\25\1");

			if (Board.Tiles[with.X - (with.StepX * 2)][with.Y - (with.StepY *
			        2)].Element == E_PUSHER)  {
				i = GetStatIdAt(with.X - (with.StepX * 2), with.Y - (with.StepY * 2));
				if ((Board.Stats[i].StepX == with.StepX)
				        && (Board.Stats[i].StepY == with.StepY))
					ElementDefs[E_PUSHER].TickProc(i);
			}
		}
	}
}

void ElementTorchTouch(integer x, integer y, integer sourceStatId,
                       integer& deltaX, integer& deltaY) {
	World.Info.Torches = World.Info.Torches + 1;
	Board.Tiles[x][y].Element = E_EMPTY;

	BoardDrawTile(x, y);
	GameUpdateSidebar();

	if (MessageTorchNotShown)  {
		DisplayMessage(200, "Torch - used for lighting in the underground.");
	}
	MessageTorchNotShown = false;

	SoundQueue(3, "\60\1\71\1\64\2");
}

void ElementInvisibleTouch(integer x, integer y, integer sourceStatId,
                           integer& deltaX, integer& deltaY) {
	{
		TTile& with = Board.Tiles[x][y];
		with.Element = E_NORMAL;
		BoardDrawTile(x, y);

		SoundQueue(3, "\22\1\20\1");
		DisplayMessage(100, "You are blocked by an invisible wall.");
	}
}

void ElementForestTouch(integer x, integer y, integer sourceStatId,
                        integer& deltaX, integer& deltaY) {
	Board.Tiles[x][y].Element = E_EMPTY;
	BoardDrawTile(x, y);

	SoundQueue(3, "\71\1");

	if (MessageForestNotShown)  {
		DisplayMessage(200, "A path is cleared through the forest.");
	}
	MessageForestNotShown = false;
}

void ElementFakeTouch(integer x, integer y, integer sourceStatId,
                      integer& deltaX, integer& deltaY) {
	if (MessageFakeNotShown)  {
		DisplayMessage(150, "A fake wall - secret passage!");
	}
	MessageFakeNotShown = false;
}

void ElementBoardEdgeTouch(integer x, integer y, integer sourceStatId,
                           integer& deltaX, integer& deltaY) {
	integer neighborId;
	integer boardId;
	integer entryX, entryY;

	entryX = Board.Stats[0].X;
	entryY = Board.Stats[0].Y;
	if (deltaY == -1)  {
		neighborId = 0;
		entryY = BOARD_HEIGHT;
	} else if (deltaY == 1)  {
		neighborId = 1;
		entryY = 1;
	} else if (deltaX == -1)  {
		neighborId = 2;
		entryX = BOARD_WIDTH;
	} else {
		neighborId = 3;
		entryX = 1;
	}

	if (Board.Info.NeighborBoards[neighborId] != 0)  {
		boardId = World.Info.CurrentBoard;
		BoardChange(Board.Info.NeighborBoards[neighborId]);
		if (Board.Tiles[entryX][entryY].Element != E_PLAYER)  {
			ElementDefs[Board.Tiles[entryX][entryY].Element].TouchProc(
			    entryX, entryY, sourceStatId, InputDeltaX, InputDeltaY);
		}

		if (ElementDefs[Board.Tiles[entryX][entryY].Element].Walkable
		        || (Board.Tiles[entryX][entryY].Element == E_PLAYER)) {
			if (Board.Tiles[entryX][entryY].Element != E_PLAYER)
				MoveStat(0, entryX, entryY);

			TransitionDrawBoardChange();
			deltaX = 0;
			deltaY = 0;
			BoardEnter();
		} else {
			BoardChange(boardId);
		}
	}
}

void ElementWaterTouch(integer x, integer y, integer sourceStatId,
                       integer& deltaX, integer& deltaY) {
	SoundQueue(3, "\100\1\120\1");
	DisplayMessage(100, "Your way is blocked by water.");
}

void DrawPlayerSurroundings(integer x, integer y, integer bombPhase) {
	integer ix, iy;
	integer istat;
	boolean result;

	for( ix = ((x - TORCH_DX) - 1); ix <= ((x + TORCH_DX) + 1); ix ++)
		if ((ix >= 1) && (ix <= BOARD_WIDTH))
			for( iy = ((y - TORCH_DY) - 1); iy <= ((y + TORCH_DY) + 1); iy ++)
				if ((iy >= 1) && (iy <= BOARD_HEIGHT)) {
					TTile& with = Board.Tiles[ix][iy];
					if ((bombPhase > 0) && ((sqr(ix-x) + sqr(iy-y)*2) < TORCH_DIST_SQR))  {
						if (bombPhase == 1)  {
							if (length(ElementDefs[with.Element].ParamTextName) != 0)  {
								istat = GetStatIdAt(ix, iy);
								if (istat > 0)
									result = OopSend(-istat, "BOMBED", false);
							}

							if (ElementDefs[with.Element].Destructible || (with.Element == E_STAR))
								BoardDamageTile(ix, iy);

							if ((with.Element == E_EMPTY) || (with.Element == E_BREAKABLE))  {
								with.Element = E_BREAKABLE;
								with.Color = 0x9 + Random(7);
								BoardDrawTile(ix, iy);
							}
						} else {
							if (with.Element == E_BREAKABLE)
								with.Element = E_EMPTY;
						}
					}
					BoardDrawTile(ix, iy);
				}
}

void GamePromptEndPlay() {
	if (World.Info.Health <= 0)  {
		GamePlayExitRequested = true;
		BoardDrawBorder();
	} else {
		GamePlayExitRequested = SidebarPromptYesNo("End this game? ", true);
		if (InputKeyPressed == '\33')
			GamePlayExitRequested = false;
	}
	InputKeyPressed = '\0';
}

void ElementPlayerTick(integer statId) {
	integer unk1, unk2, unk3;
	integer i;
	integer bulletCount;

	{
		TStat& with = Board.Stats[statId];
		if (World.Info.EnergizerTicks > 0)  {
			if (ElementDefs[E_PLAYER].Character == '\2')
				ElementDefs[E_PLAYER].Character = '\1';
			else
				ElementDefs[E_PLAYER].Character = '\2';

			if ((CurrentTick % 2) != 0)
				Board.Tiles[with.X][with.Y].Color = 0xf;
			else
				Board.Tiles[with.X][with.Y].Color = (((CurrentTick % 7) + 1) * 16) + 0xf;

			BoardDrawTile(with.X, with.Y);
		} else if ((Board.Tiles[with.X][with.Y].Color != 0x1f)
		           || (ElementDefs[E_PLAYER].Character != '\2'))  {
			Board.Tiles[with.X][with.Y].Color = 0x1f;
			ElementDefs[E_PLAYER].Character = '\2';
			BoardDrawTile(with.X, with.Y);
		}

		if (World.Info.Health <= 0)  {
			InputDeltaX = 0;
			InputDeltaY = 0;
			InputShiftPressed = false;

			if (GetStatIdAt(0,0) == -1)
				DisplayMessage(32000, " Game over  -  Press ESCAPE");

			TickTimeDuration = 0;
			SoundBlockQueueing = true;
		}
		if (InputShiftPressed || (InputKeyPressed == ' '))  {
			if (InputShiftPressed && ((InputDeltaX != 0) || (InputDeltaY != 0)))  {
				PlayerDirX = InputDeltaX;
				PlayerDirY = InputDeltaY;
			}

			if ((PlayerDirX != 0) || (PlayerDirY != 0))  {
				if (Board.Info.MaxShots == 0)  {
					if (MessageNoShootingNotShown)
						DisplayMessage(200, "Can\47t shoot in this place!");
					MessageNoShootingNotShown = false;
				} else if (World.Info.Ammo == 0)  {
					if (MessageOutOfAmmoNotShown)
						DisplayMessage(200, "You don\47t have any ammo!");
					MessageOutOfAmmoNotShown = false;
				} else {
					bulletCount = 0;
					for( i = 0; i <= Board.StatCount; i ++)
						if ((Board.Tiles[Board.Stats[i].X][Board.Stats[i].Y].Element == E_BULLET)
						        && (Board.Stats[i].P1 == 0))
							bulletCount = bulletCount + 1;

					if (bulletCount < Board.Info.MaxShots)  {
						if (BoardShoot(E_BULLET, with.X, with.Y, PlayerDirX, PlayerDirY,
						               SHOT_SOURCE_PLAYER))  {
							World.Info.Ammo = World.Info.Ammo - 1;
							GameUpdateSidebar();

							SoundQueue(2, "\100\1\60\1\40\1");

							InputDeltaX = 0;
							InputDeltaY = 0;
						}
					}
				}
			}
		} else if ((InputDeltaX != 0) || (InputDeltaY != 0))  {
			PlayerDirX = InputDeltaX;
			PlayerDirY = InputDeltaY;

			ElementDefs[Board.Tiles[with.X + InputDeltaX][with.Y +
			                               InputDeltaY].Element].TouchProc(
			                with.X + InputDeltaX, with.Y + InputDeltaY, 0, InputDeltaX, InputDeltaY);
			if ((InputDeltaX != 0) || (InputDeltaY != 0))  {
				if (SoundEnabled && ! SoundIsPlaying)
					Sound(110);
				if (ElementDefs[Board.Tiles[with.X + InputDeltaX][with.Y +
				                                   InputDeltaY].Element].Walkable)  {
					if (SoundEnabled && ! SoundIsPlaying)
						NoSound;

					MoveStat(0, with.X + InputDeltaX, with.Y + InputDeltaY);
				} else if (SoundEnabled && ! SoundIsPlaying)  {
					NoSound;
				}
			}
		}

		switch (keyUpCase(InputKeyPressed)) {
		case 'T': {
			if (World.Info.TorchTicks <= 0)  {
				if (World.Info.Torches > 0)  {
					if (Board.Info.IsDark)  {
						World.Info.Torches = World.Info.Torches - 1;
						World.Info.TorchTicks = TORCH_DURATION;

						DrawPlayerSurroundings(with.X, with.Y, 0);
						GameUpdateSidebar();
					} else {
						if (MessageRoomNotDarkNotShown)  {
							DisplayMessage(200, "Don\47t need torch - room is not dark!");
							MessageRoomNotDarkNotShown = false;
						}
					}
				} else {
					if (MessageOutOfTorchesNotShown)  {
						DisplayMessage(200, "You don\47t have any torches!");
						MessageOutOfTorchesNotShown = false;
					}
				}
			}
		}
		break;
		case '\33': case 'Q': {
			GamePromptEndPlay();
		}
		break;
		case 'S': {
			GameWorldSave("Save game:", SavedGameFileName, ".SAV");
		}
		break;
		case 'P': {
			if (World.Info.Health > 0)
				GamePaused = true;
		}
		break;
		case 'B': {
			SoundEnabled = ! SoundEnabled;
			SoundClearQueue();
			GameUpdateSidebar();
			InputKeyPressed = ' ';
		}
		break;
		case 'H': {
			TextWindowDisplayFile("GAME.HLP", "Playing ZZT");
		}
		break;
		case 'F': {
			TextWindowDisplayFile("ORDER.HLP", "Order form");
		}
		break;
		case '?': {
			GameDebugPrompt();
			InputKeyPressed = '\0';
		}
		break;
		}

		if (World.Info.TorchTicks > 0)  {
			World.Info.TorchTicks = World.Info.TorchTicks - 1;
			if (World.Info.TorchTicks <= 0)  {
				DrawPlayerSurroundings(with.X, with.Y, 0);
				SoundQueue(3, "\60\1\40\1\20\1");
			}

			if ((World.Info.TorchTicks % 40) == 0)
				GameUpdateSidebar();
		}

		if (World.Info.EnergizerTicks > 0)  {
			World.Info.EnergizerTicks = World.Info.EnergizerTicks - 1;

			if (World.Info.EnergizerTicks == 10)
				SoundQueue(9, "\40\3\32\3\27\3\26\3\25\3\23\3\20\3");
			else if (World.Info.EnergizerTicks <= 0)  {
				Board.Tiles[with.X][with.Y].Color = ElementDefs[E_PLAYER].Color;
				BoardDrawTile(with.X, with.Y);
			}
		}

		if ((Board.Info.TimeLimitSec > 0) && (World.Info.Health > 0))
			if (SoundHasTimeElapsed(World.Info.BoardTimeHsec, 100))  {
				World.Info.BoardTimeSec = World.Info.BoardTimeSec + 1;

				if ((Board.Info.TimeLimitSec - 10) == World.Info.BoardTimeSec)  {
					DisplayMessage(200, "Running out of time!");
					SoundQueue(3, "\100\6\105\6\100\6\65\6\100\6\105\6\100\n");
				} else if (World.Info.BoardTimeSec > Board.Info.TimeLimitSec)  {
					DamageStat(0);
				}

				GameUpdateSidebar();
			}
	}
}

void ElementMonitorTick(integer statId) {

	if (set::of(E_KEY_ESCAPE, 'A', 'E', 'H', 'N', 'P', 'Q', 'R', 'S', 'W', '|',
	            eos).has(keyUpCase(InputKeyPressed))) {
		GamePlayExitRequested = true;
	}
}

void ResetMessageNotShownFlags() {
	MessageAmmoNotShown = true;
	MessageOutOfAmmoNotShown = true;
	MessageNoShootingNotShown = true;
	MessageTorchNotShown = true;
	MessageOutOfTorchesNotShown = true;
	MessageRoomNotDarkNotShown = true;
	MessageHintTorchNotShown = true;
	MessageForestNotShown = true;
	MessageFakeNotShown = true;
	MessageGemNotShown = true;
	MessageEnergizerNotShown = true;
}

void InitElementDefs() {
	integer i;

	for( i = 0; i <= MAX_ELEMENT; i ++) {
		TElementDef& with = ElementDefs[i];
		with.Character = ' ';
		with.Color = COLOR_CHOICE_ON_BLACK;
		with.Destructible = false;
		with.Pushable = false;
		with.VisibleInDark = false;
		with.PlaceableOnTop = false;
		with.Walkable = false;
		with.HasDrawProc = false;
		with.Cycle = -1;
		with.TickProc = &ElementDefaultTick;
		with.DrawProc = &ElementDefaultDraw;
		with.TouchProc = &ElementDefaultTouch;
		with.EditorCategory = 0;
		with.EditorShortcut = '\0';
		with.Name = "";
		with.CategoryName = "";
		with.Param1Name = "";
		with.Param2Name = "";
		with.ParamBulletTypeName = "";
		with.ParamBoardName = "";
		with.ParamDirName = "";
		with.ParamTextName = "";
		with.ScoreValue = 0;
	}

	ElementDefs[0].Character = ' ';
	ElementDefs[0].Color = 0x70;
	ElementDefs[0].Pushable = true;
	ElementDefs[0].Walkable = true;
	ElementDefs[0].Name = "Empty";

	ElementDefs[3].Character = ' ';
	ElementDefs[3].Color = 0x7;
	ElementDefs[3].Cycle = 1;
	ElementDefs[3].TickProc = &ElementMonitorTick;
	ElementDefs[3].Name = "Monitor";

	ElementDefs[19].Character = '\260';
	ElementDefs[19].Color = 0xf9;
	ElementDefs[19].PlaceableOnTop = true;
	ElementDefs[19].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[19].TouchProc = &ElementWaterTouch;
	ElementDefs[19].EditorShortcut = 'W';
	ElementDefs[19].Name = "Water";
	ElementDefs[19].CategoryName = "Terrains:";

	ElementDefs[20].Character = '\260';
	ElementDefs[20].Color = 0x20;
	ElementDefs[20].Walkable = false;
	ElementDefs[20].TouchProc = &ElementForestTouch;
	ElementDefs[20].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[20].EditorShortcut = 'F';
	ElementDefs[20].Name = "Forest";

	ElementDefs[4].Character = '\2';
	ElementDefs[4].Color = 0x1f;
	ElementDefs[4].Destructible = true;
	ElementDefs[4].Pushable = true;
	ElementDefs[4].VisibleInDark = true;
	ElementDefs[4].Cycle = 1;
	ElementDefs[4].TickProc = &ElementPlayerTick;
	ElementDefs[4].EditorCategory = CATEGORY_ITEM;
	ElementDefs[4].EditorShortcut = 'Z';
	ElementDefs[4].Name = "Player";
	ElementDefs[4].CategoryName = "Items:";

	ElementDefs[41].Character = '\352';
	ElementDefs[41].Color = 0xc;
	ElementDefs[41].Destructible = true;
	ElementDefs[41].Pushable = true;
	ElementDefs[41].Cycle = 2;
	ElementDefs[41].TickProc = &ElementLionTick;
	ElementDefs[41].TouchProc = &ElementDamagingTouch;
	ElementDefs[41].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[41].EditorShortcut = 'L';
	ElementDefs[41].Name = "Lion";
	ElementDefs[41].CategoryName = "Beasts:";
	ElementDefs[41].Param1Name = "Intelligence?";
	ElementDefs[41].ScoreValue = 1;

	ElementDefs[42].Character = '\343';
	ElementDefs[42].Color = 0xb;
	ElementDefs[42].Destructible = true;
	ElementDefs[42].Pushable = true;
	ElementDefs[42].Cycle = 2;
	ElementDefs[42].TickProc = &ElementTigerTick;
	ElementDefs[42].TouchProc = &ElementDamagingTouch;
	ElementDefs[42].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[42].EditorShortcut = 'T';
	ElementDefs[42].Name = "Tiger";
	ElementDefs[42].Param1Name = "Intelligence?";
	ElementDefs[42].Param2Name = "Firing rate?";
	ElementDefs[42].ParamBulletTypeName = "Firing type?";
	ElementDefs[42].ScoreValue = 2;

	ElementDefs[44].Character = '\351';
	ElementDefs[44].Destructible = true;
	ElementDefs[44].Cycle = 2;
	ElementDefs[44].TickProc = &ElementCentipedeHeadTick;
	ElementDefs[44].TouchProc = &ElementDamagingTouch;
	ElementDefs[44].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[44].EditorShortcut = 'H';
	ElementDefs[44].Name = "Head";
	ElementDefs[44].CategoryName = "Centipedes";
	ElementDefs[44].Param1Name = "Intelligence?";
	ElementDefs[44].Param2Name = "Deviance?";
	ElementDefs[44].ScoreValue = 1;

	ElementDefs[45].Character = 'O';
	ElementDefs[45].Destructible = true;
	ElementDefs[45].Cycle = 2;
	ElementDefs[45].TickProc = &ElementCentipedeSegmentTick;
	ElementDefs[45].TouchProc = &ElementDamagingTouch;
	ElementDefs[45].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[45].EditorShortcut = 'S';
	ElementDefs[45].Name = "Segment";
	ElementDefs[45].ScoreValue = 3;

	ElementDefs[18].Character = '\370';
	ElementDefs[18].Color = 0xf;
	ElementDefs[18].Destructible = true;
	ElementDefs[18].Cycle = 1;
	ElementDefs[18].TickProc = &ElementBulletTick;
	ElementDefs[18].TouchProc = &ElementDamagingTouch;
	ElementDefs[18].Name = "Bullet";

	ElementDefs[15].Character = 'S';
	ElementDefs[15].Color = 0xf;
	ElementDefs[15].Destructible = false;
	ElementDefs[15].Cycle = 1;
	ElementDefs[15].TickProc = &ElementStarTick;
	ElementDefs[15].TouchProc = &ElementDamagingTouch;
	ElementDefs[15].HasDrawProc = true;
	ElementDefs[15].DrawProc = &ElementStarDraw;
	ElementDefs[15].Name = "Star";

	ElementDefs[8].Character = '\14';
	ElementDefs[8].Pushable = true;
	ElementDefs[8].TouchProc = &ElementKeyTouch;
	ElementDefs[8].EditorCategory = CATEGORY_ITEM;
	ElementDefs[8].EditorShortcut = 'K';
	ElementDefs[8].Name = "Key";

	ElementDefs[5].Character = '\204';
	ElementDefs[5].Color = 0x3;
	ElementDefs[5].Pushable = true;
	ElementDefs[5].TouchProc = &ElementAmmoTouch;
	ElementDefs[5].EditorCategory = CATEGORY_ITEM;
	ElementDefs[5].EditorShortcut = 'A';
	ElementDefs[5].Name = "Ammo";

	ElementDefs[7].Character = '\4';
	ElementDefs[7].Pushable = true;
	ElementDefs[7].TouchProc = &ElementGemTouch;
	ElementDefs[7].Destructible = true;
	ElementDefs[7].EditorCategory = CATEGORY_ITEM;
	ElementDefs[7].EditorShortcut = 'G';
	ElementDefs[7].Name = "Gem";

	ElementDefs[11].Character = '\360';
	ElementDefs[11].Color = COLOR_WHITE_ON_CHOICE;
	ElementDefs[11].Cycle = 0;
	ElementDefs[11].VisibleInDark = true;
	ElementDefs[11].TouchProc = &ElementPassageTouch;
	ElementDefs[11].EditorCategory = CATEGORY_ITEM;
	ElementDefs[11].EditorShortcut = 'P';
	ElementDefs[11].Name = "Passage";
	ElementDefs[11].ParamBoardName = "Room thru passage?";

	ElementDefs[9].Character = '\12';
	ElementDefs[9].Color = COLOR_WHITE_ON_CHOICE;
	ElementDefs[9].TouchProc = &ElementDoorTouch;
	ElementDefs[9].EditorCategory = CATEGORY_ITEM;
	ElementDefs[9].EditorShortcut = 'D';
	ElementDefs[9].Name = "Door";

	ElementDefs[10].Character = '\350';
	ElementDefs[10].Color = 0xf;
	ElementDefs[10].TouchProc = &ElementScrollTouch;
	ElementDefs[10].TickProc = &ElementScrollTick;
	ElementDefs[10].Pushable = true;
	ElementDefs[10].Cycle = 1;
	ElementDefs[10].EditorCategory = CATEGORY_ITEM;
	ElementDefs[10].EditorShortcut = 'S';
	ElementDefs[10].Name = "Scroll";
	ElementDefs[10].ParamTextName = "Edit text of scroll";

	ElementDefs[12].Character = '\372';
	ElementDefs[12].Color = 0xf;
	ElementDefs[12].Cycle = 2;
	ElementDefs[12].TickProc = &ElementDuplicatorTick;
	ElementDefs[12].HasDrawProc = true;
	ElementDefs[12].DrawProc = &ElementDuplicatorDraw;
	ElementDefs[12].EditorCategory = CATEGORY_ITEM;
	ElementDefs[12].EditorShortcut = 'U';
	ElementDefs[12].Name = "Duplicator";
	ElementDefs[12].ParamDirName = "Source direction?";
	ElementDefs[12].Param2Name = "Duplication rate?;SF";

	ElementDefs[6].Character = '\235';
	ElementDefs[6].Color = 0x6;
	ElementDefs[6].VisibleInDark = true;
	ElementDefs[6].TouchProc = &ElementTorchTouch;
	ElementDefs[6].EditorCategory = CATEGORY_ITEM;
	ElementDefs[6].EditorShortcut = 'T';
	ElementDefs[6].Name = "Torch";

	ElementDefs[39].Character = '\30';
	ElementDefs[39].Cycle = 2;
	ElementDefs[39].TickProc = &ElementSpinningGunTick;
	ElementDefs[39].HasDrawProc = true;
	ElementDefs[39].DrawProc = &ElementSpinningGunDraw;
	ElementDefs[39].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[39].EditorShortcut = 'G';
	ElementDefs[39].Name = "Spinning gun";
	ElementDefs[39].Param1Name = "Intelligence?";
	ElementDefs[39].Param2Name = "Firing rate?";
	ElementDefs[39].ParamBulletTypeName = "Firing type?";

	ElementDefs[35].Character = '\5';
	ElementDefs[35].Color = 0xd;
	ElementDefs[35].Destructible = true;
	ElementDefs[35].Pushable = true;
	ElementDefs[35].Cycle = 1;
	ElementDefs[35].TickProc = &ElementRuffianTick;
	ElementDefs[35].TouchProc = &ElementDamagingTouch;
	ElementDefs[35].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[35].EditorShortcut = 'R';
	ElementDefs[35].Name = "Ruffian";
	ElementDefs[35].Param1Name = "Intelligence?";
	ElementDefs[35].Param2Name = "Resting time?";
	ElementDefs[35].ScoreValue = 2;

	ElementDefs[34].Character = '\231';
	ElementDefs[34].Color = 0x6;
	ElementDefs[34].Destructible = true;
	ElementDefs[34].Pushable = true;
	ElementDefs[34].Cycle = 3;
	ElementDefs[34].TickProc = &ElementBearTick;
	ElementDefs[34].TouchProc = &ElementDamagingTouch;
	ElementDefs[34].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[34].EditorShortcut = 'B';
	ElementDefs[34].Name = "Bear";
	ElementDefs[34].CategoryName = "Creatures:";
	ElementDefs[34].Param1Name = "Sensitivity?";
	ElementDefs[34].ScoreValue = 1;

	ElementDefs[37].Character = '*';
	ElementDefs[37].Color = COLOR_CHOICE_ON_BLACK;
	ElementDefs[37].Destructible = false;
	ElementDefs[37].Cycle = 3;
	ElementDefs[37].TickProc = &ElementSlimeTick;
	ElementDefs[37].TouchProc = &ElementSlimeTouch;
	ElementDefs[37].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[37].EditorShortcut = 'V';
	ElementDefs[37].Name = "Slime";
	ElementDefs[37].Param2Name = "Movement speed?;FS";

	ElementDefs[38].Character = '^';
	ElementDefs[38].Color = 0x7;
	ElementDefs[38].Destructible = false;
	ElementDefs[38].Cycle = 3;
	ElementDefs[38].TickProc = &ElementSharkTick;
	ElementDefs[38].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[38].EditorShortcut = 'Y';
	ElementDefs[38].Name = "Shark";
	ElementDefs[38].Param1Name = "Intelligence?";

	ElementDefs[16].Character = '/';
	ElementDefs[16].Cycle = 3;
	ElementDefs[16].HasDrawProc = true;
	ElementDefs[16].TickProc = &ElementConveyorCWTick;
	ElementDefs[16].DrawProc = &ElementConveyorCWDraw;
	ElementDefs[16].EditorCategory = CATEGORY_ITEM;
	ElementDefs[16].EditorShortcut = '1';
	ElementDefs[16].Name = "Clockwise";
	ElementDefs[16].CategoryName = "Conveyors:";

	ElementDefs[17].Character = '\\';
	ElementDefs[17].Cycle = 2;
	ElementDefs[17].HasDrawProc = true;
	ElementDefs[17].DrawProc = &ElementConveyorCCWDraw;
	ElementDefs[17].TickProc = &ElementConveyorCCWTick;
	ElementDefs[17].EditorCategory = CATEGORY_ITEM;
	ElementDefs[17].EditorShortcut = '2';
	ElementDefs[17].Name = "Counter";

	ElementDefs[21].Character = '\333';
	ElementDefs[21].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[21].CategoryName = "Walls:";
	ElementDefs[21].EditorShortcut = 'S';
	ElementDefs[21].Name = "Solid";

	ElementDefs[22].Character = '\262';
	ElementDefs[22].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[22].EditorShortcut = 'N';
	ElementDefs[22].Name = "Normal";

	ElementDefs[31].Character = '\316';
	ElementDefs[31].HasDrawProc = true;
	ElementDefs[31].DrawProc = &ElementLineDraw;
	ElementDefs[31].Name = "Line";

	ElementDefs[43].Character = '\272';

	ElementDefs[33].Character = '\315';

	ElementDefs[32].Character = '*';
	ElementDefs[32].Color = 0xa;
	ElementDefs[32].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[32].EditorShortcut = 'R';
	ElementDefs[32].Name = "Ricochet";

	ElementDefs[23].Character = '\261';
	ElementDefs[23].Destructible = false;
	ElementDefs[23].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[23].EditorShortcut = 'B';
	ElementDefs[23].Name = "Breakable";

	ElementDefs[24].Character = '\376';
	ElementDefs[24].Pushable = true;
	ElementDefs[24].TouchProc = &ElementPushableTouch;
	ElementDefs[24].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[24].EditorShortcut = 'O';
	ElementDefs[24].Name = "Boulder";

	ElementDefs[25].Character = '\22';
	ElementDefs[25].TouchProc = &ElementPushableTouch;
	ElementDefs[25].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[25].EditorShortcut = '1';
	ElementDefs[25].Name = "Slider (NS)";

	ElementDefs[26].Character = '\35';
	ElementDefs[26].TouchProc = &ElementPushableTouch;
	ElementDefs[26].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[26].EditorShortcut = '2';
	ElementDefs[26].Name = "Slider (EW)";

	ElementDefs[30].Character = '\305';
	ElementDefs[30].TouchProc = &ElementTransporterTouch;
	ElementDefs[30].HasDrawProc = true;
	ElementDefs[30].DrawProc = &ElementTransporterDraw;
	ElementDefs[30].Cycle = 2;
	ElementDefs[30].TickProc = &ElementTransporterTick;
	ElementDefs[30].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[30].EditorShortcut = 'T';
	ElementDefs[30].Name = "Transporter";
	ElementDefs[30].ParamDirName = "Direction?";

	ElementDefs[40].Character = '\20';
	ElementDefs[40].Color = COLOR_CHOICE_ON_BLACK;
	ElementDefs[40].HasDrawProc = true;
	ElementDefs[40].DrawProc = &ElementPusherDraw;
	ElementDefs[40].Cycle = 4;
	ElementDefs[40].TickProc = &ElementPusherTick;
	ElementDefs[40].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[40].EditorShortcut = 'P';
	ElementDefs[40].Name = "Pusher";
	ElementDefs[40].ParamDirName = "Push direction?";

	ElementDefs[13].Character = '\13';
	ElementDefs[13].HasDrawProc = true;
	ElementDefs[13].DrawProc = &ElementBombDraw;
	ElementDefs[13].Pushable = true;
	ElementDefs[13].Cycle = 6;
	ElementDefs[13].TickProc = &ElementBombTick;
	ElementDefs[13].TouchProc = &ElementBombTouch;
	ElementDefs[13].EditorCategory = CATEGORY_ITEM;
	ElementDefs[13].EditorShortcut = 'B';
	ElementDefs[13].Name = "Bomb";

	ElementDefs[14].Character = '\177';
	ElementDefs[14].Color = 0x5;
	ElementDefs[14].TouchProc = &ElementEnergizerTouch;
	ElementDefs[14].EditorCategory = CATEGORY_ITEM;
	ElementDefs[14].EditorShortcut = 'E';
	ElementDefs[14].Name = "Energizer";

	ElementDefs[29].Character = '\316';
	ElementDefs[29].Cycle = 1;
	ElementDefs[29].TickProc = &ElementBlinkWallTick;
	ElementDefs[29].HasDrawProc = true;
	ElementDefs[29].DrawProc = &ElementBlinkWallDraw;
	ElementDefs[29].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[29].EditorShortcut = 'L';
	ElementDefs[29].Name = "Blink wall";
	ElementDefs[29].Param1Name = "Starting time";
	ElementDefs[29].Param2Name = "Period";
	ElementDefs[29].ParamDirName = "Wall direction";

	ElementDefs[27].Character = '\262';
	ElementDefs[27].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[27].PlaceableOnTop = true;
	ElementDefs[27].Walkable = true;
	ElementDefs[27].TouchProc = &ElementFakeTouch;
	ElementDefs[27].EditorShortcut = 'A';
	ElementDefs[27].Name = "Fake";

	ElementDefs[28].Character = ' ';
	ElementDefs[28].EditorCategory = CATEGORY_TERRAIN;
	ElementDefs[28].TouchProc = &ElementInvisibleTouch;
	ElementDefs[28].EditorShortcut = 'I';
	ElementDefs[28].Name = "Invisible";

	ElementDefs[36].Character = '\2';
	ElementDefs[36].EditorCategory = CATEGORY_CREATURE;
	ElementDefs[36].Cycle = 3;
	ElementDefs[36].HasDrawProc = true;
	ElementDefs[36].DrawProc = &ElementObjectDraw;
	ElementDefs[36].TickProc = &ElementObjectTick;
	ElementDefs[36].TouchProc = &ElementObjectTouch;
	ElementDefs[36].EditorShortcut = 'O';
	ElementDefs[36].Name = "Object";
	ElementDefs[36].Param1Name = "Character?";
	ElementDefs[36].ParamTextName = "Edit Program";

	ElementDefs[2].TickProc = &ElementMessageTimerTick;

	ElementDefs[1].TouchProc = &ElementBoardEdgeTouch;

	EditorPatternCount = 5;
	EditorPatterns[1] = E_SOLID;
	EditorPatterns[2] = E_NORMAL;
	EditorPatterns[3] = E_BREAKABLE;
	EditorPatterns[4] = E_EMPTY;
	EditorPatterns[5] = E_LINE;
}

void InitElementsEditor() {
	InitElementDefs();
	ElementDefs[28].Character = '\260';
	ElementDefs[28].Color = COLOR_CHOICE_ON_BLACK;
	ForceDarknessOff = true;
}

void InitElementsGame() {
	InitElementDefs();
	ForceDarknessOff = false;
}

void InitEditorStatSettings() {
	integer i;

	PlayerDirX = 0;
	PlayerDirY = 0;

	for( i = 0; i <= MAX_ELEMENT; i ++) {
		TEditorStatSetting& with = World.EditorStatSettings[i];
		with.P1 = 4;
		with.P2 = 4;
		with.P3 = 0;
		with.StepX = 0;
		with.StepY = -1;
	}

	World.EditorStatSettings[E_OBJECT].P1 = 1;
	World.EditorStatSettings[E_BEAR].P1 = 8;
}

class unit_Elements_initialize {
public: unit_Elements_initialize();
};
static unit_Elements_initialize Elements_constructor;

unit_Elements_initialize::unit_Elements_initialize() {
	;
}
