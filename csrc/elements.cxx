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
#include "world.h"
#include "video.h"
#include "sounds.h"
#include "input.h"
#include "txtwind.h"
#include "editor.h"
#include "oop.h"
#include "game.h"
#include "minmax.h"

#include "hardware.h"

const std::string TransporterNSChars = "^~^-v_v-";
const std::string TransporterEWChars = "(<(\263)>)\263";
const std::string StarAnimChars = "\263/\304\\";

/* For keeping track of what boards we've visited when going
	gallivanting across board edges. */
std::array<bool, MAX_BOARD+1> BoardEdgeSeen;

int sign(int x) {
	if (x > 0) {
		return 1;
	}
	if (x < 0) {
		return -1;
	}
	return 0;
}

boolean ValidStatIdx(integer x) {
	boolean ValidStatIdx_result;
	ValidStatIdx_result = (x >= 0) && (x < game_world->currentBoard.StatCount);
	return ValidStatIdx_result;
}

void SetElement(integer x, integer y, byte element) {
	/* Not if it's the player. */
	if ((game_world->currentBoard.Stats[0].X == x)
		&& (game_world->currentBoard.Stats[0].Y == y)) {
		return;
	}
	game_world->currentBoard.Tiles[x][y].Element = element;
}

void ColorCycle(integer x, integer y) {
	game_world->currentBoard.Tiles[x][y].Color =
		(game_world->currentBoard.Tiles[x][y].Color + 1) % 255;
	if (game_world->currentBoard.Tiles[x][y].Color > 15) {
		game_world->currentBoard.Tiles[x][y].Color = 9;
	}
}

void ElementDefaultTick(integer statId) {
	;
}

void ElementDefaultTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	;
}

void ElementDefaultDraw(integer x, integer y, byte & ch) {
	ch = ord('?');
}

void ElementMessageTimerTick(integer statId) {
	{
		TStat & with = game_world->currentBoard.Stats[statId];
		switch (with.X) {
			case 0: {
				video.write((60 - game_world->currentBoard.Info.Message.size()) / 2, 24,
					9 + (with.P2 % 7), " "+ game_world->currentBoard.Info.Message + " ");
				if (with.P2 > 0) {
					with.P2 = with.P2 - 1;
				}
				if (with.P2 <= 0)  {
					RemoveStat(statId);
					CurrentStatTicked = CurrentStatTicked - 1;
					BoardDrawBorder();
					game_world->currentBoard.Info.Message = "";
				}
			}
			break;
		}
	}
}

void ElementDamagingTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	BoardAttack(sourceStatId, x, y);
}

void ElementLionTick(integer statId) {
	integer deltaX, deltaY;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P1 < rnd.randint(10)) {
			CalcDirectionRnd(deltaX, deltaY);
		} else {
			CalcDirectionSeek(with.X, with.Y, deltaX, deltaY);
		}

		if (! ValidCoord(with.X + deltaX, with.Y + deltaY)) {
			return;
		}

		if (elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
														  deltaX][with.Y +
														  deltaY].Element].Walkable)  {
			MoveStat(statId, with.X + deltaX, with.Y + deltaY);
		} else if (game_world->currentBoard.Tiles[with.X + deltaX][with.Y +
				deltaY].Element ==
			E_PLAYER)  {
			BoardAttack(statId, with.X + deltaX, with.Y + deltaY);
		}
	}
}

void ElementTigerTick(integer statId) {
	boolean shot;
	byte element;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		element = E_BULLET;
		if (with.P2 >= 0x80) {
			element = E_STAR;
		}

		if ((rnd.randint(10) * 3) <= (with.P2 % 0x80))  {
			if (Difference(with.X, game_world->currentBoard.Stats[0].X) <= 2)  {
				shot = BoardShoot(element, with.X, with.Y, 0,
						Signum(game_world->currentBoard.Stats[0].Y - with.Y), SHOT_SOURCE_ENEMY);
			} else {
				shot = false;
			}

			if (! shot)  {
				if (Difference(with.Y, game_world->currentBoard.Stats[0].Y) <= 2)  {
					shot = BoardShoot(element, with.X, with.Y,
							Signum(game_world->currentBoard.Stats[0].X - with.X), 0,
							SHOT_SOURCE_ENEMY);
				}
			}
		}

		ElementLionTick(statId);
	}
}

void ElementRuffianTick(integer statId) {
	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if ((with.StepX == 0) && (with.StepY == 0))  {
			if ((with.P2 + 8) <= rnd.randint(17))  {
				if (with.P1 >= rnd.randint(9)) {
					CalcDirectionSeek(with.X, with.Y, with.StepX, with.StepY);
				} else {
					CalcDirectionRnd(with.StepX, with.StepY);
				}
			}
		} else {
			if (((with.Y == game_world->currentBoard.Stats[0].Y)
					|| (with.X == game_world->currentBoard.Stats[0].X))
				&& (rnd.randint(9) <= with.P1))  {
				CalcDirectionSeek(with.X, with.Y, with.StepX, with.StepY);
			}

			if (! ValidCoord(with.X+with.StepX, with.Y+with.StepY)) {
				return;
			}

			{
				TTile & with1 = game_world->currentBoard.Tiles[with.X + with.StepX][with.Y
						+
						with.StepY];
				if (with1.Element == E_PLAYER)  {
					BoardAttack(statId, with.X + with.StepX, with.Y + with.StepY);
				} else if (elem_info_ptr->defs[with1.Element].Walkable)  {
					MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
					if ((with.P2 + 8) <= rnd.randint(17))  {
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
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.X != game_world->currentBoard.Stats[0].X)
			if (Difference(with.Y, game_world->currentBoard.Stats[0].Y) <=
				(8 - with.P1))  {
				deltaX = Signum(game_world->currentBoard.Stats[0].X - with.X);
				deltaY = 0;
				goto LMovement;
			}

		if (Difference(with.X, game_world->currentBoard.Stats[0].X) <=
			(8 - with.P1))  {
			deltaY = Signum(game_world->currentBoard.Stats[0].Y - with.Y);
			deltaX = 0;
		} else {
			deltaX = 0;
			deltaY = 0;
		}

LMovement:
		if (! ValidCoord(with.X+deltaX, with.Y+deltaY)) {
			return;
		}

		{
			TTile & with1 = game_world->currentBoard.Tiles[with.X + deltaX][with.Y +
					deltaY];
			if (elem_info_ptr->defs[with1.Element].Walkable)  {
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
	array<0,MAX_STAT,boolean> seenFollower;


	for (tmp = 0; tmp <= MAX_STAT; tmp ++) {
		seenFollower[tmp] = false;
	}

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if ((with.X == game_world->currentBoard.Stats[0].X)
			&& (rnd.randint(10) < with.P1))  {
			with.StepY = Signum(game_world->currentBoard.Stats[0].Y - with.Y);
			with.StepX = 0;
		} else if ((with.Y == game_world->currentBoard.Stats[0].Y)
			&& (rnd.randint(10) < with.P1))  {
			with.StepX = Signum(game_world->currentBoard.Stats[0].X - with.X);
			with.StepY = 0;
		} else if (((rnd.randint(10) * 4) < with.P2) || ((with.StepX == 0)
				&& (with.StepY == 0)))  {
			CalcDirectionRnd(with.StepX, with.StepY);
		}

		if (ValidCoord(with.X+with.StepX, with.Y+with.StepY)
			&& (! elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
															  with.StepX][with.Y +
															  with.StepY].Element].Walkable
				&& (game_world->currentBoard.Tiles[with.X + with.StepX][with.Y +
						with.StepY].Element !=
					E_PLAYER))) {
			ix = with.StepX;
			iy = with.StepY;
			tmp = ((rnd.randint(2) * 2) - 1) * with.StepY;
			with.StepY = ((rnd.randint(2) * 2) - 1) * with.StepX;
			with.StepX = tmp;
			if (ValidCoord(with.X+with.StepX, with.Y+with.StepY)
				&& (! elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
																  with.StepX][with.Y +
																  with.StepY].Element].Walkable
					&& (game_world->currentBoard.Tiles[with.X + with.StepX][with.Y +
							with.StepY].Element !=
						E_PLAYER))) {
				with.StepX = -with.StepX;
				with.StepY = -with.StepY;
				if (ValidCoord(with.X+with.StepX, with.Y+with.StepY)
					&& (! elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
																	  with.StepX][with.Y +
																	  with.StepY].Element].Walkable
						&& (game_world->currentBoard.Tiles[with.X + with.StepX][with.Y +
								with.StepY].Element !=
							E_PLAYER))) {
					if (ValidCoord(with.X-ix, with.Y-iy)
						&& (elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X - ix][with.Y
																		  -
																		  iy].Element].Walkable
							|| (game_world->currentBoard.Tiles[with.X - ix][with.Y - iy].Element ==
								E_PLAYER))) {
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
			SetElement(with.X, with.Y, E_CENTIPEDE_SEGMENT);
			with.Leader = -1;
			while (ValidStatIdx(statId)
				&& (game_world->currentBoard.Stats[statId].Follower > 0))  {
				tmp = game_world->currentBoard.Stats[statId].Follower;
				game_world->currentBoard.Stats[statId].Follower =
					game_world->currentBoard.Stats[statId].Leader;
				game_world->currentBoard.Stats[statId].Leader = tmp;
				statId = tmp;

				/* Avoid infinite follower loops. */
				if (seenFollower[tmp]) {
					statId = -1;
				} else {
					seenFollower[tmp] = true;
				}
			}
			if (ValidStatIdx(statId))  {
				game_world->currentBoard.Stats[statId].Follower =
					game_world->currentBoard.Stats[statId].Leader;
				SetElement(game_world->currentBoard.Stats[statId].X,
					game_world->currentBoard.Stats[statId].Y, E_CENTIPEDE_HEAD);
			}
		} else if (ValidCoord(with.X + with.StepX, with.Y + with.StepY)
			&& (game_world->currentBoard.Tiles[with.X + with.StepX][with.Y +
					with.StepY].Element ==
				E_PLAYER))  {
			if (ValidStatIdx(with.Follower)
				&& ValidCoord(game_world->currentBoard.Stats[with.Follower].X,
					game_world->currentBoard.Stats[with.Follower].Y))  {
				SetElement(game_world->currentBoard.Stats[with.Follower].X,
					game_world->currentBoard.Stats[with.Follower].Y,
					E_CENTIPEDE_HEAD);
				game_world->currentBoard.Stats[with.Follower].StepX = with.StepX;
				game_world->currentBoard.Stats[with.Follower].StepY = with.StepY;
				BoardDrawTile(game_world->currentBoard.Stats[with.Follower].X,
					game_world->currentBoard.Stats[with.Follower].Y);
			}
			BoardAttack(statId, with.X + with.StepX, with.Y + with.StepY);
		} else {
			if (ValidCoord(with.X+with.StepX, with.Y+with.StepY)) {
				MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
			}

			tx = with.X - with.StepX;
			ty = with.Y - with.StepY;
			ix = with.StepX;
			iy = with.StepY;

			do {
				{
					TStat & with1 = game_world->currentBoard.Stats[statId];
					tx = with1.X - with1.StepX;
					ty = with1.Y - with1.StepY;
					ix = with1.StepX;
					iy = with1.StepY;

					if (with1.Follower < 0)  {
						if (ValidCoord(tx - ix, ty - iy)
							&& (game_world->currentBoard.Tiles[tx - ix][ty - iy].Element ==
								E_CENTIPEDE_SEGMENT)
							&& (GetStatIdAt(tx - ix, ty - iy) >= 0)
							&& (game_world->currentBoard.Stats[GetStatIdAt(tx - ix,
													   ty - iy)].Leader < 0)) {
							with1.Follower = GetStatIdAt(tx - ix, ty - iy);
						} else if (ValidCoord(tx - iy, ty - ix)
							&& (game_world->currentBoard.Tiles[tx - iy][ty - ix].Element ==
								E_CENTIPEDE_SEGMENT)
							&& (GetStatIdAt(tx - iy, ty - ix) >= 0)
							&& (game_world->currentBoard.Stats[GetStatIdAt(tx - iy,
													   ty - ix)].Leader < 0)) {
							with1.Follower = GetStatIdAt(tx - iy, ty - ix);
						} else if (ValidCoord(tx + iy, ty + ix)
							&& (game_world->currentBoard.Tiles[tx + iy][ty + ix].Element ==
								E_CENTIPEDE_SEGMENT)
							&& (GetStatIdAt(tx + iy, ty + ix) >= 0)
							&& (game_world->currentBoard.Stats[GetStatIdAt(tx + iy,
													   ty + ix)].Leader < 0)) {
							with1.Follower = GetStatIdAt(tx + iy, ty + ix);
						}
					}

					if ((with1.Follower > 0) && ValidStatIdx(with1.Follower))  {
						game_world->currentBoard.Stats[with1.Follower].Leader = statId;
						game_world->currentBoard.Stats[with1.Follower].P1 = with1.P1;
						game_world->currentBoard.Stats[with1.Follower].P2 = with1.P2;
						game_world->currentBoard.Stats[with1.Follower].StepX = tx -
							game_world->currentBoard.Stats[with1.Follower].X;
						game_world->currentBoard.Stats[with1.Follower].StepY = ty -
							game_world->currentBoard.Stats[with1.Follower].Y;
						if (ValidCoord(tx, ty)) {
							MoveStat(with1.Follower, tx, ty);
						}
					}

					/* Avoid infinite follower loops. */
					if ((with1.Follower < 0) || seenFollower[with1.Follower]) {
						statId = -1;
					} else {
						statId = with1.Follower;
						seenFollower[with1.Follower] = true;
					}
				}
			} while (!(statId == -1));
		}
	}
}

void ElementCentipedeSegmentTick(integer statId) {
	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.Leader < 0)  {
			if (with.Leader < -1) {
				SetElement(with.X, with.Y, E_CENTIPEDE_HEAD);
			} else {
				with.Leader = with.Leader - 1;
			}
		}
	}
}

void ElementBulletTick(integer statId) {
	integer ix, iy;
	integer iStat;
	byte iElem;
	boolean firstTry;


	{
		TStat & with = game_world->currentBoard.Stats[statId];
		firstTry = true;

LTryMove:
		ix = with.X + with.StepX;
		iy = with.Y + with.StepY;
		if (! ValidCoord(ix, iy))  {
			with.StepX = 0;
			with.StepY = 0;
			ix = with.X;
			iy = with.Y;
		}

		iElem = game_world->currentBoard.Tiles[ix][iy].Element;

		if (elem_info_ptr->defs[iElem].Walkable || (iElem == E_WATER))  {
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
			|| (elem_info_ptr->defs[iElem].Destructible && ((iElem == E_PLAYER)
					|| (with.P1 == 0)))) {
			if (elem_info_ptr->defs[iElem].ScoreValue != 0)  {
				game_world->Info.Score = game_world->Info.Score +
					elem_info_ptr->defs[iElem].ScoreValue;
				GameUpdateSidebar();
			}
			BoardAttack(statId, ix, iy);
			return;
		}

		if (ValidCoord(with.X+with.StepY, with.Y+with.StepX)
			&& (game_world->currentBoard.Tiles[with.X + with.StepY][with.Y +
					with.StepX].Element ==
				E_RICOCHET) && firstTry)  {
			ix = with.StepX;
			with.StepX = -with.StepY;
			with.StepY = -ix;
			SoundQueue(1, "\371\1");
			firstTry = false;
			goto LTryMove;
			return;
		}

		if (ValidCoord(with.X-with.StepY, with.Y-with.StepX)
			&& (game_world->currentBoard.Tiles[with.X - with.StepY][with.Y -
					with.StepX].Element ==
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

void ElementSpinningGunDraw(integer x, integer y, byte & ch) {
	switch (CurrentTick % 8) {
		case 0: case 1: ch = 24; break;
		case 2: case 3: ch = 26; break;
		case 4: case 5: ch = 25; break;
		default: ch = 27;
	}
}

void ElementLineDraw(integer x, integer y, byte & ch) {
	integer i, v, shift;

	v = 1;
	shift = 1;
	for (int i = 0; i <= 3; i ++) {
		switch (game_world->currentBoard.Tiles[x + NeighborDeltaX[i]][y +
				NeighborDeltaY[i]].Element) {
			case E_LINE: case E_BOARD_EDGE: v = v + shift; break;
		}
		shift = shift << 1;
	}
	ch = ord(LineChars[v-1]);
}

void ElementSpinningGunTick(integer statId) {
	boolean shot;
	integer deltaX, deltaY;
	byte element;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		BoardDrawTile(with.X, with.Y);

		element = E_BULLET;
		if (with.P2 >= 0x80) {
			element = E_STAR;
		}

		if (rnd.randint(9) < (with.P2 % 0x80))  {
			if (rnd.randint(9) <= with.P1)  {
				if (Difference(with.X, game_world->currentBoard.Stats[0].X) <= 2)  {
					shot = BoardShoot(element, with.X, with.Y, 0,
							Signum(game_world->currentBoard.Stats[0].Y - with.Y), SHOT_SOURCE_ENEMY);
				} else {
					shot = false;
				}

				if (! shot)  {
					if (Difference(with.Y, game_world->currentBoard.Stats[0].Y) <= 2)  {
						shot = BoardShoot(element, with.X, with.Y,
								Signum(game_world->currentBoard.Stats[0].X - with.X), 0,
								SHOT_SOURCE_ENEMY);
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
		if (! ValidCoord(x + DiagonalDeltaX[i], y + DiagonalDeltaY[i])) {
			return;
		}

		tiles[i] = game_world->currentBoard.Tiles[x + DiagonalDeltaX[i]][y +
				DiagonalDeltaY[i]];
		statsIndices[i] = GetStatIdAt(x + DiagonalDeltaX[i],
				y + DiagonalDeltaY[i]);
		{
			TTile & with = tiles[i];
			if (with.Element == E_EMPTY) {
				canMove = true;
			} else if (! elem_info_ptr->defs[with.Element].Pushable) {
				canMove = false;
			}
			/* Everything outside the viewport is treated as unpushable
			to prevent anything from going to (0,0), which is the
					  exclusive domain of the message tile. Redoing the logic
					  (e.g. a MoveStat function that rejects moving anything
					   to (0, 0)) would be better, but I can't be bothered. */
			canMove = canMove && CoordInsideViewport(x + DiagonalDeltaX[i],
					y + DiagonalDeltaY[i]);
		}
		i = i + direction;
	} while (!(i == iMax));

	i = iMin;
	do {
		{
			TTile & with = tiles[i];
			if (canMove)  {
				if (elem_info_ptr->defs[with.Element].Pushable)  {
					srcx = x + DiagonalDeltaX[i];
					srcy = y + DiagonalDeltaY[i];

					destx = x + DiagonalDeltaX[(i - direction + 8) % 8];
					desty = y + DiagonalDeltaY[(i - direction + 8) % 8];

					if (elem_info_ptr->defs[with.Element].Cycle > -1)  {
						tmpTile = game_world->currentBoard.Tiles[srcx][srcy];
						iStat = statsIndices[i];
						game_world->currentBoard.Tiles[srcx][srcy] = tiles[i];
						game_world->currentBoard.Tiles[destx][desty].Element = E_EMPTY;
						/* If the object should have stats but doesn't...
						don't crash! */
						if (iStat != -1) {
							MoveStat(iStat, destx, desty);
						}
						game_world->currentBoard.Tiles[srcx][srcy] = tmpTile;
					} else {
						game_world->currentBoard.Tiles[destx][desty] = tiles[i];
					}
					if (! elem_info_ptr->defs[tiles[(i + direction + 8) %
										 8].Element].Pushable)  {
						game_world->currentBoard.Tiles[srcx][srcy].Element = E_EMPTY;
					}
				} else {
					canMove = false;
				}
			} else if (with.Element == E_EMPTY) {
				canMove = true;
			} else if (! elem_info_ptr->defs[with.Element].Pushable) {
				canMove = false;
			}
			canMove = canMove && CoordInsideViewport(x + DiagonalDeltaX[i],
					y + DiagonalDeltaY[i]);
		}
		i = i + direction;
	} while (!(i == iMax));

	/* Draw everything to be sure that every char is updated. Doing
	BoardDraw inside the loop above can lead to tiles at some
		  relative coordinates not getting drawn. */
	for (int i = 0; i < DiagonalDeltaX.size(); i ++) {
		BoardDrawTile(x + DiagonalDeltaX[i], y + DiagonalDeltaY[i]);
	}
}

void ElementConveyorCWDraw(integer x, integer y, byte & ch) {
	switch ((CurrentTick / elem_info_ptr->defs[E_CONVEYOR_CW].Cycle) % 4) {
		case 0: ch = 179; break;
		case 1: ch = 47; break;
		case 2: ch = 196; break;
		default: ch = 92;
	}
}

void ElementConveyorCWTick(integer statId) {
	{
		TStat & with = game_world->currentBoard.Stats[statId];
		BoardDrawTile(with.X, with.Y);
		ElementConveyorTick(with.X, with.Y, 1);
	}
}

void ElementConveyorCCWDraw(integer x, integer y, byte & ch) {
	switch ((CurrentTick / elem_info_ptr->defs[E_CONVEYOR_CCW].Cycle) % 4) {
		case 3: ch = 179; break;
		case 2: ch = 47; break;
		case 1: ch = 196; break;
		default: ch = 92;
	}
}

void ElementConveyorCCWTick(integer statId) {
	{
		TStat & with = game_world->currentBoard.Stats[statId];
		BoardDrawTile(with.X, with.Y);
		ElementConveyorTick(with.X, with.Y, -1);
	}
}

void ElementBombDraw(integer x, integer y, byte & ch) {
	if (GetStatIdAt(x, y) < 0)  {
		ch = 11;
		return;
	}
	{
		TStat & with = game_world->currentBoard.Stats[GetStatIdAt(x, y)];
		if (with.P1 <= 1) {
			ch = 11;
		} else {
			ch = (48 + with.P1) % 256;
		}
	}
}

void ElementBombTick(integer statId) {
	integer oldX, oldY;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P1 > 0)  {
			with.P1 = (with.P1 - 1);
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
				if ((with.P1 % 2) == 0) {
					SoundQueue(1, "\370\1");
				} else {
					SoundQueue(1, "\365\1");
				}
			}
		}
	}
}

void ElementBombTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	if (GetStatIdAt(x, y) < 0) {
		return;
	}

	{
		TStat & with = game_world->currentBoard.Stats[GetStatIdAt(x, y)];
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

	if (GetStatIdAt(x + deltaX, y + deltaY) < 0) {
		return;
	}
	if ((deltaX == 0) && (deltaY == 0)) {
		return;
	}

	{
		TStat & with = game_world->currentBoard.Stats[GetStatIdAt(x + deltaX,
								  y + deltaY)];
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
					TTile & with1 = game_world->currentBoard.Tiles[ix][iy];
					if (with1.Element == E_BOARD_EDGE) {
						finishSearch = true;
					} else if (isValidDest)  {
						isValidDest = false;

						if (! elem_info_ptr->defs[with1.Element].Walkable) {
							ElementPushablePush(ix, iy, deltaX, deltaY);
						}

						if (elem_info_ptr->defs[with1.Element].Walkable)  {
							finishSearch = true;
							newX = ix;
							newY = iy;
						} else {
							newX = -1;
						}
					}
					if (with1.Element == E_TRANSPORTER)  {
						iStat = GetStatIdAt(ix, iy);
						if ((iStat >= 0)
							&& (game_world->currentBoard.Stats[iStat].StepX == -deltaX)
							&& (game_world->currentBoard.Stats[iStat].StepY == -deltaY)) {
							isValidDest = true;
						}
					}
				}
			} while (!(finishSearch || (! ValidCoord(ix + deltaX, iy + deltaY))));
			if (newX != -1)  {
				ElementMove(with.X - deltaX, with.Y - deltaY, newX, newY);
				SoundQueue(3, "\60\1\102\1\64\1\106\1\70\1\112\1\100\1\122\1");
			}
		}
	}
}

void ElementTransporterTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	ElementTransporterMove(x - deltaX, y - deltaY, deltaX, deltaY);
	deltaX = 0;
	deltaY = 0;
}

void ElementTransporterTick(integer statId) {
	TStat & with = game_world->currentBoard.Stats[statId];
	BoardDrawTile(with.X, with.Y);
}

// TODO: Verify against DOS.
void ElementTransporterDraw(integer x, integer y, byte & ch) {
	if (GetStatIdAt(x, y) < 0)  {
		ch = ord(' ');  /* What DOS ZZT draws. */
		return;
	}

	TStat & with = game_world->currentBoard.Stats[GetStatIdAt(x, y)];
	if (with.Cycle <= 0) {
		with.Cycle = 1;
	}

	if (with.StepX == 0) {
		ch = TransporterNSChars[sign(with.StepY) * 2 + 2 + (CurrentTick /
									 with.Cycle) % 4];
	} else {
		ch = TransporterEWChars[sign(with.StepX) * 2 + 2 + (CurrentTick /
									 with.Cycle) % 4];
	}
}

void ElementStarDraw(integer x, integer y, byte & ch) {
	ch = ord(StarAnimChars[(CurrentTick % 4) + 1-1]);
	ColorCycle(x, y);
}

void ElementStarTick(integer statId) {
	integer newStatId;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P2 == 0) {
			with.P2 = 255;
		} else {
			with.P2 = with.P2 - 1;
		}

		if (with.P2 <= 0)  {
			RemoveStat(statId);
		} else if ((with.P2 % 2) == 0)  {
			CalcDirectionSeek(with.X, with.Y, with.StepX, with.StepY);
			if (! ValidCoord(with.X + with.StepX, with.Y + with.StepY)) {
				return;
			}
			{
				TTile & with1 = game_world->currentBoard.Tiles[with.X + with.StepX][with.Y
						+
						with.StepY];
				if ((with1.Element == E_PLAYER) || (with1.Element == E_BREAKABLE))  {
					BoardAttack(statId, with.X + with.StepX, with.Y + with.StepY);
				} else {
					if (! elem_info_ptr->defs[with1.Element].Walkable) {
						ElementPushablePush(with.X + with.StepX, with.Y + with.StepY, with.StepX,
							with.StepY);
					}

					if (elem_info_ptr->defs[with1.Element].Walkable
						|| (with1.Element == E_WATER)) {
						MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
					}
				}
			}
		} else {
			BoardDrawTile(with.X, with.Y);
		}
	}
}

void ElementEnergizerTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	SoundQueue(9, string("\40\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3")
		+ "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
		+ "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
		+ "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
		+ "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
		+ "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3"
		+ "\60\3\43\3\44\3\45\3\65\3\45\3\43\3\40\3");

	game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
	BoardDrawTile(x, y);

	game_world->Info.EnergizerTicks = 75;
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
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P1 < with.P2) {
			with.P1 = with.P1 + 1;
		} else {
			color = game_world->currentBoard.Tiles[with.X][with.Y].Color;
			with.P1 = 0;
			startX = with.X;
			startY = with.Y;
			changedTiles = 0;

			for (dir = 0; dir <= 3; dir ++) {
				// Don't examine an illegal tile. (HSLIME)
				if (!ValidCoord(startX + NeighborDeltaX[dir], startY +
						NeighborDeltaY[dir])) {
					continue;
				}

				if (elem_info_ptr->defs[game_world->currentBoard.Tiles[startX +
																  NeighborDeltaX[dir]][startY +
																  NeighborDeltaY[dir]].Element].Walkable)  {
					if (changedTiles == 0)  {
						MoveStat(statId, startX + NeighborDeltaX[dir],
							startY + NeighborDeltaY[dir]);
						game_world->currentBoard.Tiles[startX][startY].Color = color;
						game_world->currentBoard.Tiles[startX][startY].Element = E_BREAKABLE;
						BoardDrawTile(startX, startY);
					} else {
						AddStat(startX + NeighborDeltaX[dir], startY + NeighborDeltaY[dir],
							E_SLIME, color,
							elem_info_ptr->defs[E_SLIME].Cycle, StatTemplateDefault);
						game_world->currentBoard.Stats[game_world->currentBoard.StatCount].P2 =
							with.P2;
					}

					changedTiles = changedTiles + 1;
				}
			}

			if (changedTiles == 0)  {
				RemoveStat(statId);
				game_world->currentBoard.Tiles[startX][startY].Element = E_BREAKABLE;
				game_world->currentBoard.Tiles[startX][startY].Color = color;
				BoardDrawTile(startX, startY);
			}
		}
	}
}

void ElementSlimeTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	integer color;

	color = game_world->currentBoard.Tiles[x][y].Color;
	if (GetStatIdAt(x, y) >= 0) {
		DamageStat(GetStatIdAt(x, y));
	}
	game_world->currentBoard.Tiles[x][y].Element = E_BREAKABLE;
	game_world->currentBoard.Tiles[x][y].Color = color;
	BoardDrawTile(x, y);
	SoundQueue(2, "\40\1\43\1");
}

void ElementSharkTick(integer statId) {
	integer deltaX, deltaY;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P1 < rnd.randint(10)) {
			CalcDirectionRnd(deltaX, deltaY);
		} else {
			CalcDirectionSeek(with.X, with.Y, deltaX, deltaY);
		}

		if (! ValidCoord(with.X + deltaX, with.Y + deltaY)) {
			return;
		}

		if (game_world->currentBoard.Tiles[with.X + deltaX][with.Y +
				deltaY].Element ==
			E_WATER) {
			MoveStat(statId, with.X + deltaX, with.Y + deltaY);
		} else if (game_world->currentBoard.Tiles[with.X + deltaX][with.Y +
				deltaY].Element ==
			E_PLAYER) {
			BoardAttack(statId, with.X + deltaX, with.Y + deltaY);
		}
	}
}

void ElementBlinkWallDraw(integer x, integer y, byte & ch) {
	ch = 206;
}

void ElementBlinkWallTick(integer statId) {
	integer ix, iy;
	boolean hitBoundary;
	integer playerStatId;
	integer el;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P3 == 0) {
			with.P3 = (with.P1 + 1) % 256;
		}
		if (with.P3 == 1)  {
			ix = with.X + with.StepX;
			iy = with.Y + with.StepY;

			if (with.StepX != 0) {
				el = E_BLINK_RAY_EW;
			} else {
				el = E_BLINK_RAY_NS;
			}

			if (! ValidCoord(ix, iy)) {
				return;
			}

			while (ValidCoord(ix, iy)
				&& (game_world->currentBoard.Tiles[ix][iy].Element == el)
				&& (game_world->currentBoard.Tiles[ix][iy].Color ==
					game_world->currentBoard.Tiles[with.X][with.Y].Color)) {
				game_world->currentBoard.Tiles[ix][iy].Element = E_EMPTY;
				BoardDrawTile(ix, iy);
				ix = ix + with.StepX;
				iy = iy + with.StepY;
				with.P3 = ((with.P2) * 2 + 1) % 256;
			}

			if (((with.X + with.StepX) == ix) && ((with.Y + with.StepY) == iy))  {
				hitBoundary = false;
				do {
					if ((game_world->currentBoard.Tiles[ix][iy].Element != E_EMPTY)
						&& (elem_info_ptr->defs[game_world->currentBoard.Tiles[ix][iy].Element].Destructible)) {
						BoardDamageTile(ix, iy);
					}

					if (game_world->currentBoard.Tiles[ix][iy].Element == E_PLAYER)  {
						playerStatId = GetStatIdAt(ix, iy);
						if (playerStatId == -1) {
							BoardDamageTile(ix, iy);
						} else {
							if (with.StepX != 0)  {
								if (game_world->currentBoard.Tiles[ix][iy - 1].Element == E_EMPTY) {
									MoveStat(playerStatId, ix, iy - 1);
								} else if (game_world->currentBoard.Tiles[ix][iy + 1].Element == E_EMPTY) {
									MoveStat(playerStatId, ix, iy + 1);
								}
							} else {
								if (game_world->currentBoard.Tiles[ix + 1][iy].Element == E_EMPTY) {
									MoveStat(playerStatId, ix + 1, iy);
								} else if (game_world->currentBoard.Tiles[ix - 1][iy].Element == E_EMPTY) {
									MoveStat(playerStatId, ix + 1, iy);
								}
							}

							if (game_world->currentBoard.Tiles[ix][iy].Element == E_PLAYER
								&& playerStatId == 0)  {
								while (game_world->Info.Health > 0) {
									DamageStat(playerStatId);
								}
								hitBoundary = true;
							}
						}
					}

					if (game_world->currentBoard.Tiles[ix][iy].Element == E_EMPTY)  {
						game_world->currentBoard.Tiles[ix][iy].Element = el;
						game_world->currentBoard.Tiles[ix][iy].Color =
							game_world->currentBoard.Tiles[with.X][with.Y].Color;
						BoardDrawTile(ix, iy);
					} else {
						hitBoundary = true;
					}

					ix = ix + with.StepX;
					iy = iy + with.StepY;
				} while (!(hitBoundary || ! ValidCoord(ix, iy)));

				with.P3 = ((with.P2 * 2) + 1) % 256;
			}
		} else {
			if (with.P3 > 0) {
				with.P3 = with.P3 - 1;
			}
		}
	}
}

void ElementMove(integer oldX, integer oldY, integer newX, integer newY) {
	integer statId;

	statId = GetStatIdAt(oldX, oldY);

	if (statId >= 0)  {
		MoveStat(statId, newX, newY);
	} else {
		game_world->currentBoard.Tiles[newX][newY] =
			game_world->currentBoard.Tiles[oldX][oldY];
		BoardDrawTile(newX, newY);
		game_world->currentBoard.Tiles[oldX][oldY].Element = E_EMPTY;
		BoardDrawTile(oldX, oldY);
	}
}

void ElementPushablePush(integer x, integer y, integer deltaX,
	integer deltaY) {
	integer unk1;

	/* IMP: Fix infinite regression when trying to push something
	onto itself.*/
	if ((deltaX == 0) && (deltaY == 0)) {
		return;
	}

	{
		TTile & with = game_world->currentBoard.Tiles[x][y];
		if (((with.Element == E_SLIDER_NS) && (deltaX == 0))
			|| ((with.Element == E_SLIDER_EW) && (deltaY == 0))
			|| elem_info_ptr->defs[with.Element].Pushable) {
			if (! ValidCoord(x + deltaX, y + deltaY)) {
				return;
			}

			if (game_world->currentBoard.Tiles[x + deltaX][y + deltaY].Element ==
				E_TRANSPORTER) {
				ElementTransporterMove(x, y, deltaX, deltaY);
			} else if (game_world->currentBoard.Tiles[x + deltaX][y + deltaY].Element
				!=
				E_EMPTY) {
				ElementPushablePush(x + deltaX, y + deltaY, deltaX, deltaY);
			}

			if (! elem_info_ptr->defs[game_world->currentBoard.Tiles[x + deltaX][y +
														 deltaY].Element].Walkable
				&& elem_info_ptr->defs[game_world->currentBoard.Tiles[x + deltaX][y +
														 deltaY].Element].Destructible
				&& (game_world->currentBoard.Tiles[x + deltaX][y + deltaY].Element !=
					E_PLAYER)) {
				BoardDamageTile(x + deltaX, y + deltaY);
			}

			if (elem_info_ptr->defs[game_world->currentBoard.Tiles[x + deltaX][y +
														 deltaY].Element].Walkable) {
				ElementMove(x, y, x + deltaX, y + deltaY);
			}
		}
	}
}

void ElementDuplicatorDraw(integer x, integer y, byte & ch) {
	/*SANITY: If there are no stats, abort outright.
	It might be better to replace GetStatIdAt with a function
		 that returns an all-zeroes stat if there's nothing there.
		 Later?*/
	ch = 250;
	if (GetStatIdAt(x, y) == -1) {
		return;
	}

	{
		TStat & with = game_world->currentBoard.Stats[GetStatIdAt(x, y)];
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
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.DataPos >= 0) {
			OopExecute(statId, with.DataPos, "Interaction");
		}

		if ((with.StepX != 0) || (with.StepY != 0))  {
			if (ValidCoord(with.X + with.StepX, with.Y + with.StepY)
				&& elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
															  with.StepX][with.Y +
															  with.StepY].Element].Walkable) {
				MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
			} else {
				retVal = OopSend(-statId, "THUD", false);
			}
		}
	}
}

void ElementObjectDraw(integer x, integer y, byte & ch) {
	ch = 1;
	if (GetStatIdAt(x, y) == -1) {
		return;
	}
	ch = game_world->currentBoard.Stats[GetStatIdAt(x, y)].P1;
}

void ElementObjectTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	integer statId;
	boolean retVal;

	if (GetStatIdAt(x, y) == -1) {
		return;
	}
	statId = GetStatIdAt(x, y);
	retVal = OopSend(-statId, "TOUCH", false);
}

void ElementDuplicatorTick(integer statId) {
	integer sourceStatId;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (with.P1 <= 4)  {
			with.P1 = with.P1 + 1;
			BoardDrawTile(with.X, with.Y);
		} else {
			with.P1 = 0;
			if (ValidCoord(with.X - with.StepX, with.Y - with.StepY)
				&& ValidCoord(with.X + with.StepX, with.Y + with.StepY)
				&& (game_world->currentBoard.Tiles[with.X - with.StepX][with.Y -
						with.StepY].Element ==
					E_PLAYER))  {
				ElementProcDefs[game_world->currentBoard.Tiles[with.X + with.StepX][with.Y
															  +
															  with.StepY].Element]
				.TouchProc(with.X + with.StepX, with.Y + with.StepY, 0,
					keyboard.InputDeltaX,
					keyboard.InputDeltaY);
			} else {
				if (ValidCoord(with.X - with.StepX, with.Y - with.StepY)
					&& (game_world->currentBoard.Tiles[with.X - with.StepX][with.Y -
							with.StepY].Element !=
						E_EMPTY)) {
					ElementPushablePush(with.X - with.StepX, with.Y - with.StepY, -with.StepX,
						-with.StepY);
				}

				if (ValidCoord(with.X - with.StepX, with.Y - with.StepY)
					&& (game_world->currentBoard.Tiles[with.X - with.StepX][with.Y -
							with.StepY].Element ==
						E_EMPTY))  {
					sourceStatId = GetStatIdAt(with.X + with.StepX, with.Y + with.StepY);
					if (sourceStatId > 0)  {
						if (game_world->currentBoard.StatCount < 174)  {
							AddStat(with.X - with.StepX, with.Y - with.StepY,
								game_world->currentBoard.Tiles[with.X + with.StepX][with.Y +
									with.StepY].Element,
								game_world->currentBoard.Tiles[with.X + with.StepX][with.Y +
									with.StepY].Color,
								game_world->currentBoard.Stats[sourceStatId].Cycle,
								game_world->currentBoard.Stats[sourceStatId]);
							BoardDrawTile(with.X - with.StepX, with.Y - with.StepY);
						}
					} else if ((sourceStatId != 0)
						&& ValidCoord(with.X + with.StepX, with.Y + with.StepY))  {
						game_world->currentBoard.Tiles[with.X - with.StepX][with.Y - with.StepY]
							= game_world->currentBoard.Tiles[with.X + with.StepX][with.Y + with.StepY];
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
		TStat & with = game_world->currentBoard.Stats[statId];
		ColorCycle(with.X, with.Y);
		BoardDrawTile(with.X, with.Y);
	}
}

void ElementScrollTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	TTextWindowState textWindow;
	integer statId;
	integer unk1;

	statId = GetStatIdAt(x, y);

	textWindow.Selectable = false;
	textWindow.LinePos = 1;

	SoundQueue(2, SoundParse("c-c+d-d+e-e+f-f+g-g"));

	if (statId < 0)  {
		game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
		return;
	}

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		with.DataPos = 0;
		OopExecute(statId, with.DataPos, "Scroll");
	}

	RemoveStat(statId);
}

void ElementKeyTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	integer key;

	key = game_world->currentBoard.Tiles[x][y].Color % 8;

	if (game_world->Info.HasKey(key))  {
		DisplayMessage(200, string("You already have a ")+game_world->Info.KeyName(
				key).c_str()+" key!");
		SoundQueue(2, "\60\2\40\2");
	} else {
		game_world->Info.GiveKey(key);
		game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
		GameUpdateSidebar();
		DisplayMessage(200, string("You now have the ")+game_world->Info.KeyName(
				key).c_str()+" key.");
		SoundQueue(2,
			"\100\1\104\1\107\1\100\1\104\1\107\1\100\1\104\1\107\1\120\2");
	}
}

void ElementAmmoTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	game_world->Info.Ammo = game_world->Info.Ammo + 5;

	game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
	GameUpdateSidebar();
	SoundQueue(2, "\60\1\61\1\62\1");

	if (MessageAmmoNotShown)  {
		MessageAmmoNotShown = false;
		DisplayMessage(200, "Ammunition - 5 shots per container.");
	}
}

void ElementGemTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	game_world->Info.Gems = game_world->Info.Gems + 1;
	game_world->Info.Health = game_world->Info.Health + 1;
	game_world->Info.Score = game_world->Info.Score + 10;

	game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
	GameUpdateSidebar();
	SoundQueue(2, "\100\1\67\1\64\1\60\1");

	if (MessageGemNotShown)  {
		MessageGemNotShown = false;
		DisplayMessage(200, "Gems give you Health!");
	}
}

void ElementPassageTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	BoardPassageTeleport(x, y);
	deltaX = 0;
	deltaY = 0;
}

void ElementDoorTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	integer key;

	key = (game_world->currentBoard.Tiles[x][y].Color / 16) % 8;

	if (game_world->Info.HasKey(key))  {
		game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
		BoardDrawTile(x, y);

		game_world->Info.TakeKey(key);
		GameUpdateSidebar();

		DisplayMessage(200, string("The ")+game_world->Info.KeyName(
				key).c_str()+" door is now open.");
		SoundQueue(3, "\60\1\67\1\73\1\60\1\67\1\73\1\100\4");
	} else {
		DisplayMessage(200, string("The ")+game_world->Info.KeyName(
				key).c_str()+" door is locked!");
		SoundQueue(3, "\27\1\20\1");
	}
}


void ElementPushableTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	ElementPushablePush(x, y, deltaX, deltaY);
	SoundQueue(2, "\25\1");
}

void ElementPusherDraw(integer x, integer y, byte & ch) {
	ch = 31;
	if (GetStatIdAt(x, y) == -1) {
		return;
	}
	{
		TStat & with = game_world->currentBoard.Stats[GetStatIdAt(x, y)];
		if (with.StepX == 1) {
			ch = 16;
		} else if (with.StepX == -1) {
			ch = 17;
		} else if (with.StepY == -1) {
			ch = 30;
		} else {
			ch = 31;
		}
	}
}

void ElementPusherTick(integer statId) {
	integer i, startX, startY;

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		startX = with.X;
		startY = with.Y;

		if (ValidCoord(with.X+with.StepX, with.Y+with.StepY)
			&& (! elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
															  with.StepX][with.Y +
															  with.StepY].Element].Walkable))  {
			ElementPushablePush(with.X + with.StepX, with.Y + with.StepY, with.StepX,
				with.StepY);
		}
	}

	statId = GetStatIdAt(startX, startY);
	if (statId < 0) {
		return;
	}

	{
		TStat & with = game_world->currentBoard.Stats[statId];
		if (ValidCoord(with.X+with.StepX, with.Y+with.StepY)
			&& elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
														  with.StepX][with.Y +
														  with.StepY].Element].Walkable)  {
			MoveStat(statId, with.X + with.StepX, with.Y + with.StepY);
			SoundQueue(2, "\25\1");

			if ((ValidCoord(with.X - (with.StepX * 2), with.X - (with.StepX * 2)))
				&& (game_world->currentBoard.Tiles[with.X - (with.StepX * 2)][with.X -
						(with.StepX *
							2)].Element == E_PUSHER))  {
				i = GetStatIdAt(with.X - (with.StepX * 2), with.Y - (with.StepY * 2));
				if (i == -1) {
					return;
				}
				if ((game_world->currentBoard.Stats[i].StepX == with.StepX)
					&& (game_world->currentBoard.Stats[i].StepY == with.StepY)) {
					ElementProcDefs[E_PUSHER].TickProc(i);
				}
			}
		}
	}
}

void ElementTorchTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	game_world->Info.Torches = game_world->Info.Torches + 1;
	game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;

	BoardDrawTile(x, y);
	GameUpdateSidebar();

	if (MessageTorchNotShown)  {
		DisplayMessage(200, "Torch - used for lighting in the underground.");
	}
	MessageTorchNotShown = false;

	SoundQueue(3, "\60\1\71\1\64\2");
}

void ElementInvisibleTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	{
		TTile & with = game_world->currentBoard.Tiles[x][y];
		with.Element = E_NORMAL;
		BoardDrawTile(x, y);

		SoundQueue(3, "\22\1\20\1");
		DisplayMessage(100, "You are blocked by an invisible wall.");
	}
}

void ElementForestTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	game_world->currentBoard.Tiles[x][y].Element = E_EMPTY;
	BoardDrawTile(x, y);

	SoundQueue(3, "\71\1");

	if (MessageForestNotShown)  {
		DisplayMessage(200, "A path is cleared through the forest.");
	}
	MessageForestNotShown = false;
}

void ElementFakeTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	if (MessageFakeNotShown)  {
		DisplayMessage(150, "A fake wall - secret passage!");
	}
	MessageFakeNotShown = false;
}

void ElementBoardEdgeTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	integer neighborId;
	integer boardId;
	integer entryX, entryY;
	integer destBoardId;

	entryX = game_world->currentBoard.Stats[0].X;
	entryY = game_world->currentBoard.Stats[0].Y;
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

	if (game_world->currentBoard.Info.NeighborBoards[neighborId] != 0)  {
		boardId = game_world->Info.CurrentBoardIdx;
		destBoardId = game_world->currentBoard.Info.NeighborBoards[neighborId];

		if (destBoardId > game_world->BoardCount) {
			destBoardId = boardId;
		}

		/* Bail if going through leads to an infinite loop. */
		if (BoardEdgeSeen[destBoardId]) {
			/* Clean up. */
			for (int i = 0; i <= MAX_BOARD; i ++) {
				BoardEdgeSeen[i] = false;
			}
			return;
		}

		/* No need to swap in and out a new board if it's the board
		we're on. */
		if (destBoardId != boardId) {
			BoardChange(destBoardId);
		}

		if (game_world->currentBoard.Tiles[entryX][entryY].Element != E_PLAYER)  {
			BoardEdgeSeen[destBoardId] = true;
			ElementProcDefs[game_world->currentBoard.Tiles[entryX][entryY].Element].TouchProc(
				entryX, entryY, sourceStatId, keyboard.InputDeltaX, keyboard.InputDeltaY);
		}

		if (elem_info_ptr->defs[game_world->currentBoard.Tiles[entryX][entryY].Element].Walkable
			|| (game_world->currentBoard.Tiles[entryX][entryY].Element == E_PLAYER)) {
			if (game_world->currentBoard.Tiles[entryX][entryY].Element != E_PLAYER) {
				MoveStat(0, entryX, entryY);
			}

			TransitionDrawBoardChange();
			deltaX = 0;
			deltaY = 0;
			BoardEnter();
		} else {
			BoardChange(boardId);
		}
	}

	/* Clean up. */
	for (int i = 0; i <= MAX_BOARD; i ++) {
		BoardEdgeSeen[i] = false;
	}
}

void ElementWaterTouch(integer x, integer y, integer sourceStatId,
	integer & deltaX, integer & deltaY) {
	SoundQueue(3, "\100\1\120\1");
	DisplayMessage(100, "Your way is blocked by water.");
}

void DrawPlayerSurroundings(integer x, integer y, integer bombPhase) {
	integer ix, iy;
	integer istat;
	boolean result;

	for (int ix = ((x - TORCH_DX) - 1); ix <= ((x + TORCH_DX) + 1); ix ++)
		if ((ix >= 1) && (ix <= BOARD_WIDTH))
			for (int iy = ((y - TORCH_DY) - 1); iy <= ((y + TORCH_DY) + 1); iy ++)
				if ((iy >= 1) && (iy <= BOARD_HEIGHT)) {
					TTile & with = game_world->currentBoard.Tiles[ix][iy];
					if ((bombPhase > 0) && ((sqr(ix-x) + sqr(iy-y)*2) < TORCH_DIST_SQR))  {
						if (bombPhase == 1)  {
							if (elem_info_ptr->defs[with.Element].ParamTextName.size() != 0)  {
								istat = GetStatIdAt(ix, iy);
								if (istat > 0) {
									result = OopSend(-istat, "BOMBED", false);
								}
							}

							if (elem_info_ptr->defs[with.Element].Destructible
								|| (with.Element == E_STAR)) {
								BoardDamageTile(ix, iy);
							}

							if ((with.Element == E_EMPTY) || (with.Element == E_BREAKABLE))  {
								with.Element = E_BREAKABLE;
								with.Color = 0x9 + rnd.randint(7);
								BoardDrawTile(ix, iy);
							}
						} else {
							if (with.Element == E_BREAKABLE) {
								with.Element = E_EMPTY;
							}
						}
					}
					BoardDrawTile(ix, iy);
				}
}

void GamePromptEndPlay() {
	if (game_world->Info.Health <= 0)  {
		GamePlayExitRequested = true;
		BoardDrawBorder();
	} else {
		GamePlayExitRequested = SidebarPromptYesNo("End this game? ", true);
		if (keyboard.InputKeyPressed == '\33') {
			GamePlayExitRequested = false;
		}
	}
	keyboard.InputKeyPressed = '\0';
}

void ElementPlayerTick(integer statId) {
	integer unk1, unk2, unk3;
	integer i;
	integer bulletCount;
	boolean canAct;

	/* IMP: The player, as a game element, is called every cycle, so it's
	impossible to freeze the game by setting cycle 0 or very high.
		  However, to otherwise be compatible with DOS ZZT, the player can
		  only move or shoot or anything that impacts the world when the
		  cycle is actually right - so setting a higher cycle on the player
		  still slows him down, as in DOS ZZT. */
	/* Running down energizer ticks or torch light counts as affecting
	the world, even though it might make logical sense for torches. */
	{
		TStat & with = game_world->currentBoard.Stats[statId];
		canAct = (with.Cycle != 0)
			&& ((CurrentTick % with.Cycle) == (CurrentStatTicked % with.Cycle));

		// TODO: The code shouldn't just reach into the Element definitions and
		// change the player character like this! But I may have no choice but to
		// allow it because doing otherwise could make my version differ from original
		// ZZT (e.g. imagine quitting while the player character is \2).
		if (game_world->Info.EnergizerTicks > 0)  {
			if (elem_info_ptr->defs[E_PLAYER].Character == '\2') {
				elem_info_ptr->defs[E_PLAYER].Character = '\1';
			} else {
				elem_info_ptr->defs[E_PLAYER].Character = '\2';
			}

			if ((CurrentTick % 2) != 0) {
				game_world->currentBoard.Tiles[with.X][with.Y].Color = 0xf;
			} else {
				game_world->currentBoard.Tiles[with.X][with.Y].Color = (((
								CurrentTick % 7) + 1) *
						16) + 0xf;
			}

			BoardDrawTile(with.X, with.Y);
		} else if ((game_world->currentBoard.Tiles[with.X][with.Y].Color != 0x1f)
			|| (elem_info_ptr->defs[E_PLAYER].Character != '\2'))  {
			game_world->currentBoard.Tiles[with.X][with.Y].Color = 0x1f;
			elem_info_ptr->defs[E_PLAYER].Character = '\2';
			BoardDrawTile(with.X, with.Y);
		}

		if (game_world->Info.Health <= 0)  {
			keyboard.InputDeltaX = 0;
			keyboard.InputDeltaY = 0;
			keyboard.InputShiftPressed = false;

			if (GetStatIdAt(0,0) == -1) {
				DisplayMessage(32000, " Game over  -  Press ESCAPE");
			}

			TickTimeDuration = 0;
			SoundBlockQueueing = true;
		}
		if (keyboard.InputShiftPressed || (keyboard.InputKeyPressed == ' '))  {
			if (keyboard.InputShiftPressed && ((keyboard.InputDeltaX != 0)
					|| (keyboard.InputDeltaY != 0)))  {
				PlayerDirX = keyboard.InputDeltaX;
				PlayerDirY = keyboard.InputDeltaY;
			}

			if (canAct && ((PlayerDirX != 0) || (PlayerDirY != 0)))  {
				if (game_world->currentBoard.Info.MaxShots == 0)  {
					if (MessageNoShootingNotShown) {
						DisplayMessage(200, "Can\47t shoot in this place!");
					}
					MessageNoShootingNotShown = false;
				} else if (game_world->Info.Ammo == 0)  {
					if (MessageOutOfAmmoNotShown) {
						DisplayMessage(200, "You don\47t have any ammo!");
					}
					MessageOutOfAmmoNotShown = false;
				} else {
					bulletCount = 0;
					for (int i = 0; i <= game_world->currentBoard.StatCount; i ++)
						if ((game_world->currentBoard.Tiles[game_world->currentBoard.Stats[i].X][game_world->currentBoard.Stats[i].Y].Element
								== E_BULLET)
							&& (game_world->currentBoard.Stats[i].P1 == 0)) {
							bulletCount = bulletCount + 1;
						}

					if (bulletCount < game_world->currentBoard.Info.MaxShots)  {
						if (BoardShoot(E_BULLET, with.X, with.Y, PlayerDirX, PlayerDirY,
								SHOT_SOURCE_PLAYER))  {
							game_world->Info.Ammo = game_world->Info.Ammo - 1;
							GameUpdateSidebar();

							SoundQueue(2, "\100\1\60\1\40\1");

							keyboard.InputDeltaX = 0;
							keyboard.InputDeltaY = 0;
						}
					}
				}
			}
		} else if ((keyboard.InputDeltaX != 0) || (keyboard.InputDeltaY != 0))  {
			PlayerDirX = keyboard.InputDeltaX;
			PlayerDirY = keyboard.InputDeltaY;

			if (ValidCoord(with.X+keyboard.InputDeltaX, with.Y+keyboard.InputDeltaY))
				ElementProcDefs[game_world->currentBoard.Tiles[with.X +
															  keyboard.InputDeltaX][with.Y
															  +
															  keyboard.InputDeltaY].Element].TouchProc(
							with.X + keyboard.InputDeltaX, with.Y + keyboard.InputDeltaY, 0,
							keyboard.InputDeltaX, keyboard.InputDeltaY);
			if (ValidCoord(with.X+keyboard.InputDeltaX, with.Y+keyboard.InputDeltaY)
				&& canAct
				&& ((keyboard.InputDeltaX != 0) || (keyboard.InputDeltaY != 0)))  {
				if (SoundEnabled && ! SoundIsPlaying) {
					Sound(110);
				}
				if (elem_info_ptr->defs[game_world->currentBoard.Tiles[with.X +
																  keyboard.InputDeltaX][with.Y +
																  keyboard.InputDeltaY].Element].Walkable)  {
					if (SoundEnabled && ! SoundIsPlaying) {
						NoSound();
					}

					MoveStat(0, with.X + keyboard.InputDeltaX, with.Y + keyboard.InputDeltaY);
				} else if (SoundEnabled && ! SoundIsPlaying)  {
					NoSound();
				}
			}
		}

		switch (upcase(keyboard.InputKeyPressed)) {
			case 'T': {
				if (canAct && (game_world->Info.TorchTicks <= 0))  {
					if (game_world->Info.Torches > 0)  {
						if (game_world->currentBoard.Info.IsDark)  {
							game_world->Info.Torches = game_world->Info.Torches - 1;
							game_world->Info.TorchTicks = TORCH_DURATION;

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
				if (game_world->Info.Health > 0) {
					GamePaused = true;
				}
			}
			break;
			case 'B': {
				SoundEnabled = ! SoundEnabled;
				SoundClearQueue();
				GameUpdateSidebar();
				keyboard.InputKeyPressed = ' ';
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
				keyboard.InputKeyPressed = '\0';
			}
			break;
		}

		if (game_world->Info.TorchTicks > 0)  {
			if (canAct) {
				game_world->Info.TorchTicks = game_world->Info.TorchTicks - 1;
			}
			if (game_world->Info.TorchTicks <= 0)  {
				DrawPlayerSurroundings(with.X, with.Y, 0);
				SoundQueue(3, "\60\1\40\1\20\1");
			}

			if ((game_world->Info.TorchTicks % 40) == 0) {
				GameUpdateSidebar();
			}
		}

		if (game_world->Info.EnergizerTicks > 0)  {
			if (canAct) {
				game_world->Info.EnergizerTicks = game_world->Info.EnergizerTicks - 1;
			}

			if (game_world->Info.EnergizerTicks == 10) {
				SoundQueue(9, "\40\3\32\3\27\3\26\3\25\3\23\3\20\3");
			} else if (game_world->Info.EnergizerTicks <= 0)  {
				game_world->currentBoard.Tiles[with.X][with.Y].Color =
					elem_info_ptr->defs[E_PLAYER].Color;
				BoardDrawTile(with.X, with.Y);
			}
		}

		if ((game_world->currentBoard.Info.TimeLimitSec > 0)
			&& (game_world->Info.Health > 0))
			if (SoundHasTimeElapsed(game_world->Info.BoardTimeHsec, 100))  {
				game_world->Info.BoardTimeSec = game_world->Info.BoardTimeSec + 1;

				if ((game_world->currentBoard.Info.TimeLimitSec - 10) ==
					game_world->Info.BoardTimeSec)  {
					DisplayMessage(200, "Running out of time!");
					SoundQueue(3, "\100\6\105\6\100\6\65\6\100\6\105\6\100\n");
				} else if (game_world->Info.BoardTimeSec >
					game_world->currentBoard.Info.TimeLimitSec)  {
					DamageStat(0);
				}

				GameUpdateSidebar();
			}
	}
}

void ElementMonitorTick(integer statId) {
	if (set::of('\33', 'A', 'E', 'H', 'N', 'P', 'Q', 'R', 'S', 'W', '|',
			eos).has(upcase(keyboard.InputKeyPressed))) {
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

	// Set up the global element info parameter. This will go
	// into Game and Editor classes when I make those.
	elem_info_ptr = std::make_shared<ElementInfo>();

	for (int i = 0; i <= MAX_ELEMENT; i ++) {
		TElementProcDef & with = ElementProcDefs[i];
		with.HasDrawProc = false;
		with.TickProc = &ElementDefaultTick;
		with.DrawProc = &ElementDefaultDraw;
		with.TouchProc = &ElementDefaultTouch;
	}
	ElementProcDefs[3].TickProc = &ElementMonitorTick;

	ElementProcDefs[19].TouchProc = &ElementWaterTouch;

	ElementProcDefs[20].TouchProc = &ElementForestTouch;

	ElementProcDefs[4].TickProc = &ElementPlayerTick;

	ElementProcDefs[41].TickProc = &ElementLionTick;
	ElementProcDefs[41].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[42].TickProc = &ElementTigerTick;
	ElementProcDefs[42].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[44].TickProc = &ElementCentipedeHeadTick;
	ElementProcDefs[44].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[45].TickProc = &ElementCentipedeSegmentTick;
	ElementProcDefs[45].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[18].TickProc = &ElementBulletTick;
	ElementProcDefs[18].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[15].TickProc = &ElementStarTick;
	ElementProcDefs[15].TouchProc = &ElementDamagingTouch;
	ElementProcDefs[15].HasDrawProc = true;
	ElementProcDefs[15].DrawProc = &ElementStarDraw;

	ElementProcDefs[8].TouchProc = &ElementKeyTouch;

	ElementProcDefs[5].TouchProc = &ElementAmmoTouch;

	ElementProcDefs[7].TouchProc = &ElementGemTouch;

	ElementProcDefs[11].TouchProc = &ElementPassageTouch;

	ElementProcDefs[9].TouchProc = &ElementDoorTouch;

	ElementProcDefs[10].TouchProc = &ElementScrollTouch;
	ElementProcDefs[10].TickProc = &ElementScrollTick;

	ElementProcDefs[12].TickProc = &ElementDuplicatorTick;
	ElementProcDefs[12].HasDrawProc = true;
	ElementProcDefs[12].DrawProc = &ElementDuplicatorDraw;

	ElementProcDefs[6].TouchProc = &ElementTorchTouch;

	ElementProcDefs[39].TickProc = &ElementSpinningGunTick;
	ElementProcDefs[39].HasDrawProc = true;
	ElementProcDefs[39].DrawProc = &ElementSpinningGunDraw;

	ElementProcDefs[35].TickProc = &ElementRuffianTick;
	ElementProcDefs[35].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[34].TickProc = &ElementBearTick;
	ElementProcDefs[34].TouchProc = &ElementDamagingTouch;

	ElementProcDefs[37].TickProc = &ElementSlimeTick;
	ElementProcDefs[37].TouchProc = &ElementSlimeTouch;

	ElementProcDefs[38].TickProc = &ElementSharkTick;

	ElementProcDefs[16].HasDrawProc = true;
	ElementProcDefs[16].TickProc = &ElementConveyorCWTick;
	ElementProcDefs[16].DrawProc = &ElementConveyorCWDraw;

	ElementProcDefs[17].HasDrawProc = true;
	ElementProcDefs[17].DrawProc = &ElementConveyorCCWDraw;
	ElementProcDefs[17].TickProc = &ElementConveyorCCWTick;

	ElementProcDefs[31].HasDrawProc = true;
	ElementProcDefs[31].DrawProc = &ElementLineDraw;

	ElementProcDefs[24].TouchProc = &ElementPushableTouch;

	ElementProcDefs[25].TouchProc = &ElementPushableTouch;

	ElementProcDefs[26].TouchProc = &ElementPushableTouch;

	ElementProcDefs[30].TouchProc = &ElementTransporterTouch;
	ElementProcDefs[30].HasDrawProc = true;
	ElementProcDefs[30].DrawProc = &ElementTransporterDraw;
	ElementProcDefs[30].TickProc = &ElementTransporterTick;

	ElementProcDefs[40].HasDrawProc = true;
	ElementProcDefs[40].DrawProc = &ElementPusherDraw;
	ElementProcDefs[40].TickProc = &ElementPusherTick;

	ElementProcDefs[13].HasDrawProc = true;
	ElementProcDefs[13].DrawProc = &ElementBombDraw;
	ElementProcDefs[13].TickProc = &ElementBombTick;
	ElementProcDefs[13].TouchProc = &ElementBombTouch;

	ElementProcDefs[14].TouchProc = &ElementEnergizerTouch;

	ElementProcDefs[29].TickProc = &ElementBlinkWallTick;
	ElementProcDefs[29].HasDrawProc = true;
	ElementProcDefs[29].DrawProc = &ElementBlinkWallDraw;

	ElementProcDefs[27].TouchProc = &ElementFakeTouch;

	ElementProcDefs[28].TouchProc = &ElementInvisibleTouch;

	ElementProcDefs[36].HasDrawProc = true;
	ElementProcDefs[36].DrawProc = &ElementObjectDraw;
	ElementProcDefs[36].TickProc = &ElementObjectTick;
	ElementProcDefs[36].TouchProc = &ElementObjectTouch;

	ElementProcDefs[2].TickProc = &ElementMessageTimerTick;

	ElementProcDefs[1].TouchProc = &ElementBoardEdgeTouch;

	EditorPatternCount = 5;
	EditorPatterns[1] = E_SOLID;
	EditorPatterns[2] = E_NORMAL;
	EditorPatterns[3] = E_BREAKABLE;
	EditorPatterns[4] = E_EMPTY;
	EditorPatterns[5] = E_LINE;
}

void InitElementsEditor() {
	InitElementDefs();
	elem_info_ptr->defs[28].Character = '\260';
	elem_info_ptr->defs[28].Color = COLOR_CHOICE_ON_BLACK;
	ForceDarknessOff = true;
}

void InitElementsGame() {
	InitElementDefs();
	ForceDarknessOff = false;

	for (int i = 0; i <= MAX_BOARD; i ++) {
		BoardEdgeSeen[i] = false;
	}
}

void InitEditorStatSettings() {
	integer i;

	PlayerDirX = 0;
	PlayerDirY = 0;

	for (int i = 0; i <= MAX_ELEMENT; i ++) {
		TEditorStatSetting & with = EditorStatSettings[i];
		with.P1 = 4;
		with.P2 = 4;
		with.P3 = 0;
		with.StepX = 0;
		with.StepY = -1;
	}

	EditorStatSettings[E_OBJECT].P1 = 1;
	EditorStatSettings[E_BEAR].P1 = 8;
}

class unit_Elements_initialize {
	public: unit_Elements_initialize();
};
static unit_Elements_initialize Elements_constructor;

unit_Elements_initialize::unit_Elements_initialize() {
	;
}
