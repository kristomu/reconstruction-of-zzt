#pragma once

#include "ptoc.h"
#include "array.h"
#include "elements.h"
#include "curses_io.h"      // colors, perhaps put somewhere else?

#include <vector>
#include <string>

const integer MAX_STAT = 150;
const integer BOARD_WIDTH = 60;
const integer BOARD_HEIGHT = 25;
const integer MAX_BOARD_LEN = 20000;

const int BOARD_NORTH = 0,
          BOARD_SOUTH = 1,
          BOARD_WEST = 2,
          BOARD_EAST = 3;

class TTile {
public:
	element_t Element;
	unsigned char Color;

	size_t packed_size() const;
	void dump(std::vector<unsigned char> & out) const;

	std::vector<unsigned char>::const_iterator load(
		std::vector<unsigned char>::const_iterator ptr,
		const std::vector<unsigned char>::const_iterator end);

	bool operator==(const TTile & x) {
		return Element == x.Element && Color == x.Color;
	}
};

const TTile TileBorder = {E_NORMAL, Yellow};
const TTile TileBoardEdge = {E_BOARD_EDGE, Black};

struct TRleTile {
	unsigned char Count;
	TTile Tile;
};

struct TStat {
public:
	unsigned char X, Y;
	short StepX, StepY;
	short Cycle;
	unsigned char P1, P2, P3;
	short Follower;
	short Leader;
	TTile Under;
	array<0, 3,unsigned char> unk1;
	short DataPos;
	short DataLen;
	unsigned char* Data;
	size_t packed_size() const;
	void dump(std::vector<unsigned char> & out) const;
	std::vector<unsigned char>::const_iterator load(
		std::vector<unsigned char>::const_iterator ptr,
		const std::vector<unsigned char>::const_iterator end);
};

// Perhaps enforce minimum and maximum size with get/set? Feels kinda
// ugly/kludgy though.

struct TBoardInfo {
	unsigned char MaxShots;
	bool IsDark;
	array<0, 3,unsigned char> NeighborBoards;
	bool ReenterWhenZapped;
	std::string Message;        // Max length 58
	unsigned char StartPlayerX;
	unsigned char StartPlayerY;
	short TimeLimitSec;
	array<70, 85,unsigned char> unk1;

	size_t packed_size() const;

	void dump(std::vector<unsigned char> & out) const;
	std::vector<unsigned char>::const_iterator load(
		std::vector<unsigned char>::const_iterator ptr,
		const std::vector<unsigned char>::const_iterator end);
};

/* This is used to make sure IoTmpBuf is always large enough to hold
   what changes may happen to the board. In the worst case, the board
   is completely empty when loaded but takes max space due to stats,
   then during play, the board gets filled with random tiles. That
   will require MAX_RLE_OVERFLOW additional bytes to hold. So by
   dimensioning IoTmpBuf with an excess of MAX_RLE_OVERFLOW, we ensure
   that it can never run out of space from RLE shenanigans. */
const integer MAX_RLE_OVERFLOW = BOARD_WIDTH * BOARD_HEIGHT *
	sizeof(TRleTile);

class TBoard {
private:
	void adjust_board_stats();

public:
	std::string Name;
	matrix<0, BOARD_WIDTH + 1,0, BOARD_HEIGHT + 1,TTile> Tiles;
	short StatCount;
	array<0, MAX_STAT + 1,TStat> Stats;
	TBoardInfo Info;

	bool valid_coord(short x, short y) const;

	void create();

	// Serialize
	// BLUESKY: Make these const, because output shouldn't alter the
	// board itself.
	std::vector<unsigned char> dump(std::string & out_load_error);
	std::vector<unsigned char> dump();

	// Deserialize
	std::string load(const std::vector<unsigned char> & source,
		int board_num, int number_of_boards);
	std::string load(const std::vector<unsigned char> & source);
};