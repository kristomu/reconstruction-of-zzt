#include "ptoc.h"
#include "gamevars.h"
#include "array.h"
#include "serialization.h"
#include "tools.h"

const size_t MAX_BOARD_NAME_LENGTH = 50;

// Note, all of this still operates on some coupling of the size of
// the internal variables and what's being serialized. BLUESKY: Fix that.
// The packed size shouldn't depend on the internal variables, only on
// the format that the output takes. But then we have to also construct
// compatibility warning checks if the variables exceed the value that
// can be represented in a classic ZZT format, etc... later.

size_t TTile::packed_size() const {
	return sizeof(Element) + sizeof(Color);
}

void TTile::dump(std::vector<unsigned char> & out) const {
	append_lsb_element(Element, out);
	append_lsb_element(Color, out);
}

std::vector<unsigned char>::const_iterator TTile::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end) {

	if (end - ptr < packed_size()) {
		throw std::runtime_error("Insufficient data to load TTile");
	}

	ptr = load_lsb_element(ptr, Element);
	ptr = load_lsb_element(ptr, Color);

	return ptr;
}

// TODO: Don't count the data itself if it exists somewhere else
// (unless we're the first object???). For now it overestimates the
// necessary size.

// A better implementation may split away the size counting for stats
// so that there's a separate object for the contents. That's what DOS
// ZZT does "in effect", because it only considers sizeof(TStat) to be
// the literal size, which treats the data piece as a 4-byte pointer.
// Consider that in greater detail. For now, there'll be errors with
// stats coming close to the max board limit.

// I'm guessing I'll eventually end up using some kind of associative
// array or something. Or maybe just as simple as serializing every time
// AddStat is called to see if we exceed the limit...

size_t TStat::packed_size() const {
	return sizeof(X) + sizeof(Y) + sizeof(StepX) + sizeof(StepY) +
		sizeof(Cycle) + sizeof(P1) + sizeof(P2) + sizeof(P3) +
		sizeof(Follower) + sizeof(Leader) + Under.packed_size() +
		4 + sizeof(DataPos) + sizeof(DataLen) + DataLen + 8;
}

void TStat::dump(std::vector<unsigned char> & out) const {
	// Dump the stats data.
	append_array(std::vector<unsigned char>{X, Y}, out);
	append_array(std::vector<short>{StepX, StepY, Cycle}, out);
	append_array(std::vector<unsigned char>{P1, P2, P3}, out);
	append_array(std::vector<short>{Follower, Leader}, out);
	Under.dump(out);
	// Four bytes of padding.
	append_zeroes(4, out);
	append_array(std::vector<short>{DataPos, DataLen}, out);
	// Dump the data itself.
	std::copy(data.get(), data.get() + DataLen,
		std::back_inserter<std::vector<unsigned char> >(out));
	// Dump eight zeroes - this is where the padding went in original
	// ZZT and the pointer was put in FPC.
	append_zeroes(8, out);
}

std::vector<unsigned char>::const_iterator TStat::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end) {

	std::vector<unsigned char>::const_iterator start_ptr = ptr;

	if (end - start_ptr < packed_size()) {
		throw std::runtime_error("Insufficient data to load TStat");
	}

	ptr = load_lsb_element(ptr, X);
	ptr = load_lsb_element(ptr, Y);
	ptr = load_lsb_element(ptr, StepX);
	ptr = load_lsb_element(ptr, StepY);
	ptr = load_lsb_element(ptr, Cycle);
	ptr = load_lsb_element(ptr, P1);
	ptr = load_lsb_element(ptr, P2);
	ptr = load_lsb_element(ptr, P3);
	ptr = load_lsb_element(ptr, Follower);
	ptr = load_lsb_element(ptr, Leader);
	ptr = Under.load(ptr, end);
	ptr += 4;								// skip four zeroes of padding.
	ptr = load_lsb_element(ptr, DataPos);
	ptr = load_lsb_element(ptr, DataLen);

	// We've loaded data length, now find out if we've got enough space
	// to load it.
	// TODO: Some kind of graceful recovery where, if there isn't enough
	// space, we only read up to the end of the pointer? That's what the
	// Pascal version does.

	if (end - start_ptr < packed_size()) {
		throw std::runtime_error("Insufficient data to load TStat contents");
	}

	data = std::shared_ptr<unsigned char[]>(new unsigned char[DataLen]);
	std::copy(ptr, ptr + DataLen, data.get());

	ptr += 8; // Skip eight zeroes of padding

	return ptr;
}

// The size the board info takes when in BRD/ZZT format.
size_t TBoardInfo::packed_size() const {
	return sizeof(MaxShots) + sizeof(IsDark) + NeighborBoards.size() +
		sizeof(ReenterWhenZapped) + 59 + sizeof(StartPlayerX) +
		sizeof(StartPlayerY) + sizeof(TimeLimitSec);
}

void TBoardInfo::dump(std::vector<unsigned char> & out) const {
	// Metadata and stats info
	append_lsb_element(MaxShots, out);
	append_lsb_element(IsDark, out);
	// Neighboring boards
	append_array(NeighborBoards, out);
	append_lsb_element(ReenterWhenZapped, out);
	append_pascal_string(Message, 58, out);
	append_lsb_element(StartPlayerX, out);
	append_lsb_element(StartPlayerY, out);
	append_lsb_element(TimeLimitSec, out);
	append_zeroes(16, out);
}

std::vector<unsigned char>::const_iterator TBoardInfo::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end) {

	bool truncated = false;

	if (end - ptr < packed_size()) {
		throw std::runtime_error("Insufficient data to load TBoardInfo");
	}

	ptr = load_lsb_element(ptr, MaxShots);
	ptr = load_lsb_element(ptr, IsDark);
	ptr = load_array(ptr, NeighborBoards);
	ptr = load_lsb_element(ptr, ReenterWhenZapped);
	ptr = get_pascal_string(ptr, end, 58, true, Message, truncated);
	ptr = load_lsb_element(ptr, StartPlayerX);
	ptr = load_lsb_element(ptr, StartPlayerY);
	ptr = load_lsb_element(ptr, TimeLimitSec);
	ptr += 16;	// Skip padding

	return ptr;
}

/* Clean up stats by processing DataLen reference chains, clamping out-of-
   bounds stat values, and placing a player on the board if there is none
   already. */
void TBoard::adjust_board_stats() {
	short ix, iy;

	/* SANITY: Process referential DataLen variables. */

	for( ix = 0; ix <= Board.StatCount; ix ++) {
		TStat& with = Board.Stats[ix];
		/* Deal with aliased objects. The two problems we can get are
		   objects pointing to objects that themselves point to other
		   objects; and objects pointing to objects that don't exist. */
		if (with.DataLen < 0)  {
			/* Well-behaved linear reference chains do nothing (don't work)
			   in DOS ZZT, so cyclical ones should do nothing either.
			   If we're pointing at another reference or out of bounds, do
			   nothing.

			   Furthermore, if we're the player, drop all data and
			   references.
			   Due to the way that serialization works, we can't let the
			   player refer to a later object's data; because when the
			   serializer then tries to correct this by making the latter
			   object point to the player, the reference DataLen would be
			   -0, which is the same as 0, and thus can't be detected as
			   a reference to begin with. */
			if ((ix == 0) || (-with.DataLen > Board.StatCount) ||
			        (Board.Stats[-with.DataLen].DataLen < 0)) {
				with.DataLen = 0;
			}

			// Dereference aliases
			if (with.DataLen < 0)  {
				with.data = Board.Stats[-with.DataLen].data;
				with.DataLen = Board.Stats[-with.DataLen].DataLen;
			}

			// If it's still aliased (DataLen < 0), then we have a chain
			// or a cycle. Break it.
			if (with.DataLen < 0)
				with.DataLen = 0;
		}
	}

	/* SANITY: Positive Leader and Follower values must be indices
	   to stats. If they're too large, they're corrupt: set them to zero.
	   Furthermore, there's no need for StepX and StepY to be out of
	   range of the board area, and clamping these values helps
	   avoid a ton of over/underflow problems whose fixes would
	   otherwise clutter up the code... */
	for( ix = 0; ix <= Board.StatCount; ix ++) {
			TStat& with = Board.Stats[ix];
			if (with.Follower > Board.StatCount) { with.Follower = 0; }
			if (with.Leader > Board.StatCount)	{ with.Leader = 0; }

			if (with.StepX < -BOARD_WIDTH)	{ with.StepX = -BOARD_WIDTH; }
			if (with.StepX > BOARD_WIDTH)	{ with.StepX = BOARD_WIDTH; }

			if (with.StepY < -BOARD_HEIGHT)	{ with.StepY = -BOARD_HEIGHT; }
			if (with.StepY > BOARD_HEIGHT)	{ with.StepY = BOARD_HEIGHT; }
	}

	/* SANITY: If there's neither a player nor a monitor at the position
	   indicated by stats 0, place a player there to keep the invariant
	   that one should always exist on every board. */
		TStat& with = Board.Stats[0];
		if ((Board.Tiles[with.X][with.Y].Element != E_PLAYER) &&
			(Board.Tiles[with.X][with.Y].Element != E_MONITOR))
			Board.Tiles[with.X][with.Y].Element = E_PLAYER;
}

// Create a yellow-border board with standard parameters
void TBoard::create() {
	short ix, iy, i;

	Name = "";
	Info.Message = "";
	Info.MaxShots = -1;			// unlimited
	Info.IsDark = false;
	Info.ReenterWhenZapped = false;
	Info.TimeLimitSec = 0;

	for( i = 0; i < 4; i ++)
		Info.NeighborBoards[i] = 0;

	// BOARD_WIDTH and BOARD_HEIGHT gives the width and height of
	// the part of the board that the player can see. This visible
	// region is surrounded by board edges.

	for( ix = 0; ix <= BOARD_WIDTH+1; ix ++) {
		Tiles[ix][0] = TileBoardEdge;
		Tiles[ix][BOARD_HEIGHT+1] = TileBoardEdge;
	}

	for( iy = 0; iy <= BOARD_HEIGHT+1; iy ++) {
		Tiles[0][iy] = TileBoardEdge;
		Tiles[BOARD_WIDTH+1][iy] = TileBoardEdge;
	}

	// Fill the interior with empties.

	for( ix = 1; ix <= BOARD_WIDTH; ix ++)
		for( iy = 1; iy <= BOARD_HEIGHT; iy ++) {
			Tiles[ix][iy].Element = E_EMPTY;
			Tiles[ix][iy].Color = 0;
		}

	// Then do the yellow border.

	for( ix = 1; ix <= BOARD_WIDTH; ix ++) {
		Tiles[ix][1] = TileBorder;
		Tiles[ix][BOARD_HEIGHT] = TileBorder;
	}
	for( iy = 1; iy <= BOARD_HEIGHT; iy ++) {
		Tiles[1][iy] = TileBorder;
		Tiles[BOARD_WIDTH][iy] = TileBorder;
	}

	// Place the player.
	// TODO: Use stats placement functions once refactored properly.

	Tiles[BOARD_WIDTH / 2][BOARD_HEIGHT / 2].Element = E_PLAYER;
	Tiles[BOARD_WIDTH / 2][BOARD_HEIGHT / 2].Color =
	    ElementDefs[E_PLAYER].Color;
	StatCount = 0;
	Stats[0].X = BOARD_WIDTH / 2;
	Stats[0].Y = BOARD_HEIGHT / 2;
	Stats[0].Cycle = 1;
	Stats[0].Under.Element = E_EMPTY;
	Stats[0].Under.Color = 0;
	Stats[0].data = NULL;
	Stats[0].DataLen = 0;

	// CHANGE: Not actually done in DOS ZZT, but we want every
	// returned board to be judged valid by the checks in load(),
	// even if serialized right after create().

	Info.StartPlayerX = Stats[0].X;
	Info.StartPlayerY = Stats[0].Y;
}

bool TBoard::valid_coord(short x, short y) const {
	return x >= 0 && y >= 0 && x <= BOARD_WIDTH+1 && y <= BOARD_HEIGHT+1;
}

// load() loads the char representation of a board (given by the
// source vector) into itself. If the board is corrupted, the function
// reconstructs the board as best as it can and then returns an error
// string; otherwise, it returns the empty string.

// This function has an extreme number of checks against potential
// corruption, so that it will pass every fuzzed crash test and deal
// with corrupted boards as gracefully as possible.

// NOTE: The serialized representation does not include the initial two
// length bytes in Pascal; these are considered to be part of the world
// structure, where the board structure is just the bytes that follow
// the length integer.

std::string TBoard::load(const std::vector<unsigned char> & source,
	int cur_board_id, int number_of_boards) {

	std::vector<unsigned char>::const_iterator ptr = source.begin();
	// We assume the vector has been set to reflect either the data
	// read from the file, or the world boardLen parameter, whichever
	// is smaller. The outside should check that boardLen matches.

	short i, ix, iy;

	bool boardIsDamaged = false, truncated = false;

	/* Create a default yellow border board, because we might need
	to abort before the board is fully specced. */
	create();

	/* Check that the sanity check on board titles have been executed. */

	/* SANITY: Reconstruct the title. We need at least a size of two bytes
	   for the title: a size designation and the first letter of the title.
	   If we don't even have that, let the title be blank.*/

	// ---------------- Board name  ----------------
	ptr = get_pascal_string(source.begin(), source.end(),
		MAX_BOARD_NAME_LENGTH, true, Name, truncated);

	if (truncated) {
		/* This board is damaged. */
		return "Board name is truncated";
	}

	// ---------------- RLE board layout  ----------------
	TRleTile rle;
	ix = 1;
	iy = 1;
	rle.Count = 0;
	bool detected_zero_RLE = false;

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

		BoardClose and BoardOpen may still desynchronize, but with dynamic
		memory allocation being easy in 32 bit land, that's no longer a
		problem. Allowing rle count 0 would cause forward compatibility
		problems if the worlds thus created were opened in classical ZZT
		instead, so we don't. */
		if (rle.Count <= 0)  {
			/* Not enough space? Get outta here. */
			if (source.end() - ptr < sizeof(rle)) {
				return "Abrupt end to RLE chunk";
			}
			rle.Count = *ptr++;
			rle.Tile.Element = *ptr++;
			rle.Tile.Color = *ptr++;

			if (rle.Count == 0)  {
				detected_zero_RLE = true;
				continue;
			}
		}

		/* SANITY: If the element is unknown, replace it with a normal. */
		if (rle.Tile.Element > MAX_ELEMENT)  {
			rle.Tile.Element = E_NORMAL;
			boardIsDamaged = true;
		}

		Tiles[ix++][iy] = rle.Tile;
		if (ix > BOARD_WIDTH)  {
			ix = 1;
			iy = iy + 1;
		}

		rle.Count = rle.Count - 1;

	} while (!((iy > BOARD_HEIGHT) || (ptr == source.end())));

	// ---------------- Metadata and stats info  ----------------
;
	/* SANITY: If reading board info and the stats count byte would
		get us out of bounds, we have a board that's truncated too early.
		Do the best we can, then exit with an error. */
	try {
		ptr = Info.load(ptr, source.end());
	} catch (const std::exception & e) {
		return e.what();
	}

	/* Clamp out-of-bounds Info variables. They'll cause problems
		in the editor otherwise. */
	for (size_t i = 0; i < Info.NeighborBoards.size(); ++i) {
		if (Info.NeighborBoards[i] > number_of_boards) {
			Info.NeighborBoards[i] = cur_board_id;
		}
		boardIsDamaged = true;
	}

	/* Clamp an out-of-board player location, if there is any.
	   This is actually only damage in certain cases (save files with zap
	   on entry, etc.) */
	if (!valid_coord(Info.StartPlayerX, Info.StartPlayerY))  {
		Info.StartPlayerX = 1;
		Info.StartPlayerY = 1;
		boardIsDamaged = true;
	}

	if (source.end() - ptr < sizeof(StatCount)) {
		return "Board is truncated after Info";
	}

	// This is just a short in little endian format.
	ptr = load_lsb_element(ptr, StatCount);

	// Clamp to be positive and to not exceed max_stat.
	StatCount = std::max((short)0, std::min(StatCount, MAX_STAT));

	for (ix = 0; ix <= StatCount; ++ix) {
		TStat& with = Stats[ix];
		with.DataLen = 0;

		/* SANITY: Handle too few stats items for the stats count. */
		if (source.end() - ptr < with.packed_size()) {
			StatCount = std::max(ix - 1, 0);
			boardIsDamaged = true;
			break;
		}

		try {
			ptr = Stats[ix].load(ptr, source.end());
		} catch (const std::exception & e) {
			return "Stats[" + itos(ix) + "]: " + e.what();
		}

		/* SANITY: If the element underneath is unknown, replace it
		with a normal. */
		if (with.Under.Element > MAX_ELEMENT)  {
			with.Under.Element = E_NORMAL;
			boardIsDamaged = true;
		}

		/* SANITY: Handle objects that are out of bounds. */
		if (!valid_coord(with.X, with.Y))  {
			with.X = std::min(std::max((int)with.X, 0), BOARD_WIDTH+1);
			with.Y = std::min(std::max((int)with.Y, 0), BOARD_HEIGHT+1);
			boardIsDamaged = true;
		}

		/* SANITY: (0,0) is not available: it's used by one-line messages.
			So if the stat is at (0,0) or another unavailable position, put
			it into (1,1). */

		/* The compromise to the Postelic position is probably to be
			generous, but show a warning message that the board was
			corrupted and attempted fixed. */
		if ((with.X == 0) && (with.Y == 0))  {
			with.X = 1;
			with.Y = 1;
			boardIsDamaged = true;
		}

		/* SANITY: If DataLen is much too large, truncate. We'll
		then stop processing more objects next round around
		the loop. */
		// This shouldn't be needed as we're doing it below, no ???
		/*if (bytesRead + with.DataLen > board_length)  {
			with.DataLen = board_length - bytesRead;
			boardIsDamaged = true;
		}*/

		if (with.DataLen > 0)
			/* SANITY: If DataLen is too long, truncate it. */
			if (source.end() - ptr < with.DataLen)  {
				with.DataLen = source.end() - ptr;
				boardIsDamaged = true;
			}

		/* Only allocate if data length is still positive... */
		if (with.DataLen > 0)  {
			with.data = std::shared_ptr<unsigned char[]>(
				new unsigned char[with.DataLen]);
			std::copy(ptr, ptr + with.DataLen, with.data.get());

			ptr += with.DataLen;
		}

		/* Otherwise, clear Data to avoid potential leaks later. */
		if (with.DataLen == 0)  with.data = NULL;
	}

	adjust_board_stats();

	if (detected_zero_RLE) {
		return "Zero RLE count detected";
	}

	if (boardIsDamaged) {
		return "Some parameters are out of bounds";
	}

	return "";
}

std::string TBoard::load(const std::vector<unsigned char> & source) {
	return load(source, 0, MAX_BOARD);
}

std::vector<unsigned char> TBoard::dump(std::string & out_load_error) {

	std::vector<unsigned char> out;
	out.reserve(MAX_RLE_OVERFLOW);

	out_load_error = "";

	// Board name
	append_pascal_string(Name, MAX_BOARD_NAME_LENGTH, out);

	// RLE board layout
	TRleTile rle;
	rle.Count = 0;
	rle.Tile = Tiles[1][1];

	for (int iy = 1; iy <= BOARD_HEIGHT; ++iy) {
		for (int ix = 1; ix <= BOARD_WIDTH; ++ix) {
			if (Tiles[ix][iy] == rle.Tile && rle.Count < 255) {
				++rle.Count;
			} else {
				out.push_back(rle.Count);
				rle.Tile.dump(out);

				rle.Tile = Tiles[ix][iy];
				rle.Count = 1;
			}
		}
	}

	out.push_back(rle.Count);
	rle.Tile.dump(out);

	// Metadata and stats info
	Info.dump(out);

	// Stats
	append_lsb_element(StatCount, out);

	for(int stat_idx = 0; stat_idx <= StatCount; ++stat_idx) {
		// Clean up #BIND references so they all point to the first object.
		if (Stats[stat_idx].DataLen > 0)  {
			for(int primary = 1; primary < stat_idx; ++primary) {
				/* If two pointers share the same code (same pointer),
				   link the latter to the former. */
				if (Stats[primary].data == Stats[stat_idx].data)  {
					Stats[stat_idx].DataLen = -primary;
					Stats[stat_idx].data = NULL;
					break;
				}
			}
		}

		Stats[stat_idx].dump(out);
	}

	/* Needs to be done elsewhere, since it relates to the world itself. */
	//World.BoardLen[World.Info.CurrentBoard] = ptr - ptrStart;

	/* If we're using too much space, truncate the size, feed the
		whole thing back through load() to fix the inevitable
		corruption, then run BoardClose again.
		This smart-ass solution should allow us to keep all the smarts of
		board parsing in BoardOpen and nowt have to duplicate any logic.

		Such a situation should *only* happen if RLE is too large (see
		RLEFLOW.ZZT), because AddStat should reject adding stats when
		there's no room.; */
	if (out.size() > MAX_BOARD_LEN) {
		out_load_error = load(out);
		// Propagate the error by ignoring any error we get from the
		// second dump.
		std::string throwaway;
		return dump(throwaway);
	}

	return out;
}

// For debugging/testing purposes.
std::vector<unsigned char> TBoard::dump() {
	std::string throwaway;
	return(dump(throwaway));
}