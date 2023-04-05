#pragma once

#include "elements.h"
#include "io/colors.h"      // colors, perhaps put somewhere else?

#include <vector>
#include <string>
#include <memory>

const short MAX_BOARD = 100;
const short MAX_STAT = 150;
const short BOARD_WIDTH = 60;
const short BOARD_HEIGHT = 25;
const short MAX_BOARD_LEN = 20000;

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
		std::string dump_to_readable(int num_indents) const;

		std::vector<unsigned char>::const_iterator load(
			std::vector<unsigned char>::const_iterator ptr,
			const std::vector<unsigned char>::const_iterator end);

		bool operator==(const TTile & x) const {
			return Element == x.Element && Color == x.Color;
		}
};

const TTile TileBorder = {E_NORMAL, Yellow};
const TTile TileBoardEdge = {E_BOARD_EDGE, Black};

struct TRleTile {
	unsigned char Count;
	TTile Tile;
};

class TStat {
	public:
		unsigned char X, Y;
		short StepX, StepY;
		short Cycle;
		unsigned char P1, P2, P3;
		short Follower;
		short Leader;
		TTile Under;
		// In the serialized format, the following four
		// bytes was used to store a pointer to data. It's not used
		// here, TODO: remove and such.
		std::array<unsigned char, 4> unk1;
		short DataPos;
		short DataLen;
		std::shared_ptr<unsigned char[]> data;

		size_t packed_size(int my_idx, int owner_idx) const;
		size_t marginal_packed_size() const;
		size_t upper_bound_packed_size() const;

		void dump(std::vector<unsigned char> & out) const;
		std::string dump_to_readable(int num_indents,
			bool literal_data) const;

		std::vector<unsigned char>::const_iterator load(
			std::vector<unsigned char>::const_iterator ptr,
			const std::vector<unsigned char>::const_iterator end,
			bool load_data);
};

// Perhaps enforce minimum and maximum size with get/set? Feels kinda
// ugly/kludgy though.

struct TBoardInfo {
	unsigned char MaxShots;
	bool IsDark;
	std::array<unsigned char, 4> NeighborBoards;
	bool ReenterWhenZapped;
	std::string Message;        // Max length 58
	unsigned char StartPlayerX;
	unsigned char StartPlayerY;
	short TimeLimitSec;
	std::array<unsigned char, 16> unk1;

	size_t packed_size() const;

	void dump(std::vector<unsigned char> & out) const;
	std::string dump_to_readable(int num_indents) const;

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
const short MAX_RLE_OVERFLOW = BOARD_WIDTH * BOARD_HEIGHT *
	sizeof(TRleTile);

class TBoard {
	private:
		// The two parts to board serialization: trying to do our best
		// with what's provided, and a final fixup that clamps stats,
		// places the player, and in general ensures invariants are satisfied.
		std::string parse_board(const std::vector<unsigned char> & source,
			int cur_board_id, int number_of_boards);
		void adjust_board_stats();

		// Cached_packed_size starts off as -1 when we don't know what
		// the packed size is. Any call to get_packed_size() will set it
		// through dump() in that case. Note that it may also be wrong due
		// to RLE shenanigans, like what RLEFLOW.ZZT exploits.
		// (TODO: Fix later.)
		mutable size_t cached_packed_size = -1;

	public:
		std::string Name;
		// The tiles span from 0 to height/width+1 inclusive. The extreme
		// coordinates of a board are dedicated to the board edge tile and
		// not stored in the board file.
		std::array<std::array<TTile, BOARD_HEIGHT + 2>, BOARD_WIDTH+2> Tiles;
		short StatCount;
		std::array<TStat, MAX_STAT + 1> Stats;
		TBoardInfo Info;

		bool valid_coord(short x, short y) const;

		void create();

		// Serialize
		std::vector<unsigned char> dump() const;
		std::string dump_to_readable(int num_indents) const;

		size_t get_packed_size() const;

		// Dump in a way that respects MAX_BOARD_LEN
		std::vector<unsigned char> dump_and_truncate(
			std::string & out_load_error);
		std::vector<unsigned char> dump_and_truncate();

		// Deserialize
		std::string load(const std::vector<unsigned char> & source,
			int board_num, int number_of_boards);
		std::string load(const std::vector<unsigned char> & source);

		// Add item with stats, copying data if required. Returns true
		// if the addition was done, false otherwise.
		bool add_stat(size_t x, size_t y, element_t element,
			unsigned char color, short cycle, TStat stat_template);

		// Remove an item with stats. out_x and out_y will be set
		// to the coordinates where it resided. (Is that too hacky?)
		// current_stat_ticked is a reference to the global variable that
		// keeps track of the last stat that ticked. It's done like this
		// to make board.h possible to use in other programs without having
		// to pull in all of gamevars.
		bool remove_stat(int stat_id, int & out_x, int & out_y,
			short & current_stat_ticked);
};