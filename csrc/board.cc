#include "ptoc.h"
#include "gamevars.h"
#include "array.h"
#include "serialization.h"

const size_t MAX_BOARD_NAME_LENGTH = 50;

// Note, all of this still operates on some coupling of the size of
// the internal variables and what's being serialized. TODO: Fix that.
// The packed size shouldn't depend on the internal variables, only on
// the format that the output takes.

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
	std::copy(Data, Data + DataLen,
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

	// There will be leaks. Need to be fixed later, by turning Data into
	// either a std::string or a vector of unsigned char. TODO.

	Data = new unsigned char[DataLen];
	std::copy(ptr, ptr + DataLen, Data);

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
				with.Data = Board.Stats[-with.DataLen].Data;
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
	Stats[0].Data = nil;
	Stats[0].DataLen = 0;
}

bool TBoard::valid_coord(short x, short y) const {
	return x >= 0 && y >= 0 && x <= BOARD_WIDTH+1 && y <= BOARD_HEIGHT+1;
}

// Returns true if the board is in OK condition or false if it is
// corrupted.
// This function has an extreme number of checks against potential
// corruption, so that it will pass every fuzzed crash test and deal
// with corrupted boards as gracefully as possible.

// TODO? Somehow indicate that this load routine doesn't except on an
// error, just tries to gracefully deal with what's given? Or actually
// *throw* an exception and use that as a signal to the world loader?
bool TBoard::load(const std::vector<unsigned char> & source) {
	std::vector<unsigned char>::const_iterator ptr = source.begin();

	// XXX: Must be outside because it refers to the world state.
	// Error handling also leaves something to be desired.

	// https://stackoverflow.com/questions/43483669/how-to-catch-an-incrementing-iterator-exception

	//World.Info.CurrentBoard = boardId;
	// We assume the vector has been set to reflect either the data
	// read from the file, or the world boardLen parameter, whichever
	// is smaller. The outside should check that boardLen matches.
	// TODO: Handle cases where the board is smaller than expected and
	// the excess bytes can be interpreted as the next board instead...
	// But in that case, I think it just skips the excess (trusting the
	// metadata).

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
		return false;
	}

	ptr += MAX_BOARD_NAME_LENGTH + 1;

	// ---------------- RLE board layout  ----------------
	TRleTile rle;
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

		BoardClose and BoardOpen may still desynchronize, but with dynamic
		memory allocation being easy in 32 bit land, that's no longer a
		problem. Allowing rle count 0 would cause forward compatibility
		problems if the worlds thus created were opened in classical ZZT
		instead, so we don't. */
		if (rle.Count <= 0)  {
			/* Not enough space? Get outta here. */
			if (source.end() - ptr > sizeof(rle)) return false;
			rle.Count = *ptr++;
			rle.Tile.Element = *ptr++;
			rle.Tile.Color = *ptr++;

			if (rle.Count == 0)  {
				boardIsDamaged = true;
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

	// MORE TO COME ...

	// ---------------- Metadata and stats info  ----------------

	/* SANITY: If reading board info and the stats count byte would
	get us out of bounds, we have a board that's truncated too early.
		  Do the best we can, then show the damaged board note and exit. */
	ptr = Info.load(ptr, source.end());
	if (ptr == source.end()) {
		//World.Info.CurrentBoard = boardId;
		adjust_board_stats();

		return false;
	}

	/* Clamp out-of-bounds Info variables. They'll cause problems
	in the editor otherwise. */
	// TODO: Put this outside, or pass in the BoardCount variable...
	/*for( i = 0; i <= 3; i ++)
		/v* This behavior is from elements.pas, BoardEdgeTouch. *v/
		if (Info.NeighborBoards[i] > World.BoardCount)
			Info.NeighborBoards[i] = boardId;*/

	/* Clamp an out-of-board player location, if there is any. */
	if (!valid_coord(Info.StartPlayerX, Info.StartPlayerY))  {
		Info.StartPlayerX = 1;
		Info.StartPlayerY = 1;
		boardIsDamaged = true;
	}

	// This is just a short in little endian format.

	ptr = load_lsb_element(ptr, StatCount); // Error handling req'd!

	// Clamp to be positive and to not exceed max_stat.
	StatCount = std::max((short)0, std::min(StatCount, MAX_STAT));

	for (ix = 0; ix <= StatCount; ++ix) {
		TStat& with = Stats[ix];
		with.DataLen = 0;

		/* SANITY: Handle too few stats items for the stats count. */
		if (source.end() - ptr < with.packed_size()) {
			StatCount = std::max(ix - 1, 0);
			// TBD
//			World.Info.CurrentBoard = boardId;
			boardIsDamaged = true;
			break;
		}

		ptr = Stats[ix].load(ptr, source.end());

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
			it into (1,1). TODO? Make a note of which are thus placed, and
			place them on empty spots on the board instead if possible... */

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
		// This shouldn't be needed... ???
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
#ifdef TBD
			//GetMem(with.Data, with.DataLen);
            with.Data = (unsigned char *)malloc(with.DataLen);
			MoveP(*ptr, *with.Data, with.DataLen);
#endif
			ptr += with.DataLen;
		}

		/* Otherwise, clear Data to avoid potential leaks later. */
		if (with.DataLen == 0)  with.Data = nil;
	}

	adjust_board_stats();
	//World.Info.CurrentBoard = boardId;

	// TBD
/*	if (boardIsDamaged)
		DisplayCorruptionNote();*/

	return !boardIsDamaged;

}

std::vector<unsigned char> TBoard::dump() {

	std::vector<unsigned char> out;
	out.reserve(MAX_RLE_OVERFLOW);

	bool cleanupNeeded = false;

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
				if (Stats[primary].Data == Stats[stat_idx].Data)  {
					Stats[stat_idx].DataLen = -primary;
					Stats[stat_idx].Data = nil;
					break;
				}
			}
		}

		Stats[stat_idx].dump(out);
	}

	/* Needs to be done elsewhere, since it relates to the world itself. */
	//World.BoardLen[World.Info.CurrentBoard] = ptr - ptrStart;

	/* If we're using too much space, truncate the size, feed the
	whole thing back through BoardOpen to fix the inevitable
		  corruption, then run BoardClose again.
		  This smart-ass solution should allow us to keep all the smarts of
		  board parsing in BoardOpen and nowt have to duplicate any logic. */
	/* Such a situation should *only* happen if RLE is too large (see
	RLEFLOW.ZZT), because AddStat should reject adding stats when
		  there's no room. */
	if (out.size() > MAX_BOARD_LEN)  {
		cleanupNeeded = true;
	}

	// I need to actually implement BoardOpen to make *this* work.
	// And then a bunch of tests to see if BoardOpen(BoardClose(x)) == x.
	/*if (cleanupNeeded)  {
		BoardOpen(World.Info.CurrentBoard, false);
		BoardClose(false);
		if (showTruncationNote)  DisplayTruncationNote();
	}*/

	return out;
}