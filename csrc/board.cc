#include "board.h"
#include "serialization.h"
#include "tools.h"

#include <cassert>

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

std::string TTile::dump_to_readable(int num_indents,
	std::shared_ptr<const ElementInfo> element_info) const {

	std::string out;

	std::string indent(num_indents, '\t');

	out += indent + "Element: " + element_info->defs[Element].Name + "\n";
	out += indent + "Color: " + itos(Color) + "\n";

	return out;
}

std::vector<unsigned char>::const_iterator TTile::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end) {

	if (end - ptr < packed_size()) {
		throw std::runtime_error("Insufficient data to load TTile");
	}

	std::vector<unsigned char>::const_iterator start_ptr = ptr;

	ptr = load_lsb_element(ptr, Element);
	ptr = load_lsb_element(ptr, Color);

	assert(ptr - start_ptr == packed_size());

	return ptr;
}

// This does what you expect. my_idx and owner_idx are indices to the
// object array (e.g. the values that you'd negate when serializing).
// If my_idx is owner_idx, then data is counted. Otherwise, it's not.
// If the data is unique and my_idx != owner_idx, it throws an
// exception. If DataLen < 0, it assumes the data is not unique.
size_t TStat::packed_size(int my_idx, int owner_idx) const {
	size_t space_for_data = 0;

	// If it's a bound object, there's no data here.
	if (DataLen > 0) {
		if (data.unique() && my_idx != owner_idx) {
			throw std::logic_error(
				"TStat: my_idx != owner_idx with unique data");
		}
		if (data.unique() || my_idx == owner_idx) {
			space_for_data += DataLen;
		}
	}

	return sizeof(X) + sizeof(Y) + sizeof(StepX) + sizeof(StepY) +
		sizeof(Cycle) + sizeof(P1) + sizeof(P2) + sizeof(P3) +
		sizeof(Follower) + sizeof(Leader) + Under.packed_size() +
		4 + sizeof(DataPos) + sizeof(DataLen) + space_for_data + 8;
}

// Don't count the data itself if it exists somewhere else.
// This works as long as #BIND is strictly kept from binding to
// objects on other boards; doing so will make the stat underestimate
// its size if the resulting world is dumped on a board-by-board basis.

// NOTE: this function will lie when serializing with TBoard::dump().
// Suppose you have two objects, the latter bound to the former. Then
// because TBoard::dump() doesn't invalidate pointers before copying,
// both objects will count as having no data, even though one of them must
// hold the data. But when used with AddStat and RemoveStat, everything
// works as intended because they always concern themselves with the
// object that's being added or removed, hence "marginal".

size_t TStat::marginal_packed_size() const {
	// If the data is unique, we're the owner by definition.
	if (DataLen > 0 && data.unique()) {
		return packed_size(-1, -1);
	}

	// On the margin, we're never the owner of a non-unique
	// data chunk.
	return packed_size(-1, -2);
}

// Always counts the data, e.g. if you're using a template that has
// non-unique data but you know you're going to do a deep copy and
// want to check if it'll fit before actually doing that copy.
size_t TStat::upper_bound_packed_size() const {
	return packed_size(-1, -1);
}

void TStat::dump(std::vector<unsigned char> & out) const {
	// Dump the stats data.
	append_array(std::vector<unsigned char> {X, Y}, out);
	append_array(std::vector<short> {StepX, StepY, Cycle}, out);
	append_array(std::vector<unsigned char> {P1, P2, P3}, out);
	append_array(std::vector<short> {Follower, Leader}, out);
	Under.dump(out);
	// Four bytes of padding.
	append_zeroes(4, out);
	append_array(std::vector<short> {DataPos, DataLen}, out);
	// Dump eight zeroes - this is where the padding went in original
	// ZZT and the pointer was put in FPC.
	append_zeroes(8, out);
	// Dump the data itself - if there is any.
	if (DataLen > 0 && data) {
		std::copy(data.get(), data.get() + DataLen,
			std::back_inserter<std::vector<unsigned char> >(out));
	}
}

std::string TStat::dump_to_readable(int num_indents,
	bool literal_data, std::shared_ptr<const ElementInfo> element_info) const {

	std::string out, indent(num_indents, '\t');

	out += indent + "X: " + itos(X) + "\n";
	out += indent + "Y: " + itos(Y) + "\n";
	out += indent + "StepX: " + itos(StepX) + "\n";
	out += indent + "StepY: " + itos(StepY) + "\n";
	out += indent + "StepY: " + itos(StepY) + "\n";
	out += indent + "P1: " + itos(P1) + "\n";
	out += indent + "P2: " + itos(P2) + "\n";
	out += indent + "P3: " + itos(P3) + "\n";
	out += indent + "Follower: " + itos(Follower) + "\n";
	out += indent + "Leader: " + itos(Leader) + "\n";
	out += indent + "Under: " + itos(Leader) + "\n" +
		Under.dump_to_readable(num_indents + 1,
			element_info);
	out += indent + "DataLen: " + itos(DataLen) + "\n";
	if (DataLen < 0 || !data) {
		return out;
	}
	out += indent + "Data reference count: " + itos(data.use_count()) + "\n";
	out += indent + "Data:";
	if (literal_data) {
		out += " ";
		std::copy(data.get(), data.get() + DataLen,
			std::back_inserter<std::string>(out));
	} else {
		for (size_t i = 0; i < DataLen; ++i) {
			out += " " + str_toupper(itos_hex(data[i], 2));
		}
	}
	out += "\n";

	return out;
}

std::vector<unsigned char>::const_iterator TStat::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end,
	bool load_data) {

	std::vector<unsigned char>::const_iterator start_ptr = ptr;

	if (end - start_ptr < marginal_packed_size()) {
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
	ptr += 8; 								// Skip eight zeroes of padding

	// We've loaded data length, now find out if we've got enough space
	// to load it. If not, adjust DataLen accordingly.

	// TODO? signal error somehow? this is off-spec and should be
	// "rewarded" with a dialog box.

	if (end - start_ptr < marginal_packed_size()) {
		DataLen = end - ptr;
	}

	if (DataPos > DataLen) {
		DataPos = 0;
	}

	if (load_data && DataLen > 0) {
		data = std::shared_ptr<unsigned char[]>(new unsigned char[DataLen]);
		std::copy(ptr, ptr + DataLen, data.get());

		ptr += DataLen;
		assert(ptr - start_ptr == marginal_packed_size());
	} else {
		data = NULL;
	}

	return ptr;
}

// The size the board info takes when in BRD/ZZT format.
size_t TBoardInfo::packed_size() const {
	return sizeof(MaxShots) + sizeof(IsDark) + NeighborBoards.size() +
		sizeof(ReenterWhenZapped) + 59 + sizeof(StartPlayerX) +
		sizeof(StartPlayerY) + sizeof(TimeLimitSec) + 16;
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

std::string TBoardInfo::dump_to_readable(int num_indents) const {

	std::string out, indent(num_indents, '\t');

	out += indent + "Max shots: " + itos(MaxShots) + "\n";
	out += indent + "Is dark: " + yes_no(IsDark) + "\n";
	for (size_t i = 0; i < NeighborBoards.size(); ++i) {
		out += indent + "Neighbor board " + itos(i) + ": " +
			itos(NeighborBoards[i]) + "\n";
	}
	out += indent + "Reenter when zapped: " +
		yes_no(ReenterWhenZapped) + "\n";
	out += indent + "Message: " + Message + "\n";
	out += indent + "Player starting X: " + itos(StartPlayerX) + "\n";
	out += indent + "Player starting Y: " + itos(StartPlayerY) + "\n";
	out += indent + "Time limit (secs): " + itos(TimeLimitSec) + "\n";

	return out;
}

std::vector<unsigned char>::const_iterator TBoardInfo::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end) {

	bool truncated = false;

	if (end - ptr < packed_size()) {
		throw std::runtime_error("Insufficient data to load TBoardInfo");
	}

	std::vector<unsigned char>::const_iterator start_ptr = ptr;

	ptr = load_lsb_element(ptr, MaxShots);
	ptr = load_lsb_element(ptr, IsDark);
	ptr = load_array(ptr, NeighborBoards);
	ptr = load_lsb_element(ptr, ReenterWhenZapped);
	ptr = get_pascal_string(ptr, end, 58, true, Message, truncated);
	ptr = load_lsb_element(ptr, StartPlayerX);
	ptr = load_lsb_element(ptr, StartPlayerY);
	ptr = load_lsb_element(ptr, TimeLimitSec);
	ptr += 16;	// Skip padding

	assert(ptr - start_ptr == packed_size());

	return ptr;
}

// parse_board() loads the char representation of a board (given by the
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

std::string TBoard::parse_board(const std::vector<unsigned char> & source,
	int cur_board_id, int number_of_boards) {

	std::vector<unsigned char>::const_iterator ptr = source.begin();
	// We assume the vector has been set to reflect either the data
	// read from the file, or the world boardLen parameter, whichever
	// is smaller. The outside should check that boardLen matches.

	short i, ix, iy;

	std::string board_error = "";
	bool truncated = false;

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
				update(board_error, "Zero RLE count detected");
				continue;
			}
		}

		/* SANITY: If the element is unknown, replace it with a normal. */
		if (rle.Tile.Element > MAX_ELEMENT)  {
			rle.Tile.Element = E_NORMAL;
			update(board_error, "RLE: Unknown element detected");
		}

		Tiles[ix++][iy] = rle.Tile;
		if (ix > BOARD_WIDTH)  {
			ix = 1;
			iy = iy + 1;
		}

		rle.Count = rle.Count - 1;

	} while (!((iy > BOARD_HEIGHT) || (ptr == source.end())));


	// ---------------- Metadata and stats info  ----------------

	/* SANITY: If reading board info and the stats count byte would
		get us out of bounds, we have a board that's truncated too early.
		Do the best we can, then exit with an error. */
	try {
		ptr = Info.load(ptr, source.end());
	} catch (const std::runtime_error & e) {
		return e.what();
	}

	/* Clamp out-of-bounds Info variables. They'll cause problems
		in the editor otherwise. */
	for (size_t i = 0; i < Info.NeighborBoards.size(); ++i) {
		if (Info.NeighborBoards[i] > number_of_boards) {
			Info.NeighborBoards[i] = cur_board_id;
			update(board_error, "Neighboring board doesn't exist");
		}
	}

	/* Clamp an out-of-board player location, if there is any.
	   This is actually only damage in certain cases (save files with zap
	   on entry, etc.) */
	if (!valid_coord(Info.StartPlayerX, Info.StartPlayerY))  {
		Info.StartPlayerX = 1;
		Info.StartPlayerY = 1;
		update(board_error, "Player is not at a valid coordinate");
	}

	if (source.end() - ptr < sizeof(StatCount)) {
		return "Board is truncated after Info";
	}

	// This is just a short in little endian format.
	ptr = load_lsb_element(ptr, StatCount);

	// Clamp to be positive and to not exceed max_stat.
	StatCount = std::max((short)0, std::min(StatCount, MAX_STAT));

	for (ix = 0; ix <= StatCount; ++ix) {
		TStat & with = Stats[ix];
		with.DataLen = 0;

		/* SANITY: Handle too few stats items for the stats count. */
		if (source.end() - ptr < with.marginal_packed_size()) {
			StatCount = std::max(ix - 1, 0);
			update(board_error,
				"StatCount claims more stats than there is data");
			break;
		}

		try {
			// Don't load the data inside the Stats serialization function
			// since we want to be more careful about believing its
			// metadata.
			ptr = Stats[ix].load(ptr, source.end(), false);
		} catch (const std::runtime_error & e) {
			update(board_error, "Stats[" + itos(ix) + "]: " + e.what());
			break;
		}

		/* SANITY: If the element underneath is unknown, replace it
		with a normal. */
		if (with.Under.Element > MAX_ELEMENT)  {
			with.Under.Element = E_NORMAL;
			update(board_error, "Stat has unknown element underneath");
		}

		/* SANITY: Handle objects that are out of bounds. */
		if (!valid_coord(with.X, with.Y))  {
			with.X = std::min(std::max((int)with.X, 0), BOARD_WIDTH+1);
			with.Y = std::min(std::max((int)with.Y, 0), BOARD_HEIGHT+1);
			update(board_error, "Stat owner is not at a valid coordinate");
		}

		/* The compromise to the Postelic position is probably to be
			generous, but show a warning message that the board was
			corrupted and attempted fixed. */

		/* SANITY: (0,0) is not available: it's used by one-line messages.
			So if the stat is at (0,0) or another unavailable position, put
			it into (1,1). */

		// Fix the error quietly because TOWN.ZZT's Lab 5 board has it -
		// and we don't want to throw an error on the included world.

		// The "proper" way of doing this would be to force the board tile
		// at 0,0 to be a messenger, because that's the only way you can get
		// stats at 0,0 in ZZT. But then that would resume showing a one-
		// liner message after loading a saved game, which doesn't happen
		// in the original, and we're trying to behave exactly like it does,
		// even if the original is strictly speaking wrong.

		if ((with.X == 0) && (with.Y == 0))  {
			with.X = 1;
			with.Y = 1;
			//update(board_error, "Stat owner is at (0,0)");
		}

		if (with.DataLen > 0)
			/* SANITY: If DataLen is too long, truncate it. */
			if (source.end() - ptr < with.DataLen)  {
				with.DataLen = source.end() - ptr;
				update(board_error, "Stat DataLen extends past end of board");
			}

		/* Only allocate if data length is still positive... */
		if (with.DataLen > 0)  {
			with.data = std::shared_ptr<unsigned char[]>(
					new unsigned char[with.DataLen]);
			std::copy(ptr, ptr + with.DataLen, with.data.get());

			ptr += with.DataLen;
		}

		/* Otherwise, clear Data to avoid potential leaks later. */
		if (with.DataLen == 0) {
			with.data = NULL;
		}
	}

	return board_error;
}

/* Clean up stats by processing DataLen reference chains, clamping out-of-
   bounds stat values, and placing a player on the board if there is none
   already. */
void TBoard::adjust_board_stats() {
	short ix, iy;

	/* SANITY: Process referential DataLen variables. */

	for (ix = 0; ix <= StatCount; ix ++) {
		TStat & with = Stats[ix];
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
			if ((ix == 0) || (-with.DataLen > StatCount) ||
				(Stats[-with.DataLen].DataLen < 0)) {
				with.DataLen = 0;
			}

			// Dereference aliases
			if (with.DataLen < 0)  {
				with.data = Stats[-with.DataLen].data;
				with.DataLen = Stats[-with.DataLen].DataLen;
			}

			// If it's still aliased (DataLen < 0), then we have a chain
			// or a cycle. Break it.
			if (with.DataLen < 0) {
				with.DataLen = 0;
			}
		}
	}

	/* SANITY: Positive Leader and Follower values must be indices
	   to stats. If they're too large, they're corrupt: set them to zero.
	   Furthermore, there's no need for StepX and StepY to be out of
	   range of the board area, and clamping these values helps
	   avoid a ton of over/underflow problems whose fixes would
	   otherwise clutter up the code... */
	for (ix = 0; ix <= StatCount; ix ++) {
		TStat & with = Stats[ix];
		if (with.Follower > StatCount) {
			with.Follower = 0;
		}
		if (with.Leader > StatCount)	{
			with.Leader = 0;
		}

		if (with.StepX < -BOARD_WIDTH)	{
			with.StepX = -BOARD_WIDTH;
		}
		if (with.StepX > BOARD_WIDTH)	{
			with.StepX = BOARD_WIDTH;
		}

		if (with.StepY < -BOARD_HEIGHT)	{
			with.StepY = -BOARD_HEIGHT;
		}
		if (with.StepY > BOARD_HEIGHT)	{
			with.StepY = BOARD_HEIGHT;
		}
	}

	/* SANITY: If there's neither a player nor a monitor at the position
	   indicated by stats 0, place a player there to keep the invariant
	   that one should always exist on every board. */
	TStat & with = Stats[0];
	if ((Tiles[with.X][with.Y].Element != E_PLAYER) &&
		(Tiles[with.X][with.Y].Element != E_MONITOR)) {
		Tiles[with.X][with.Y].Element = E_PLAYER;
	}
}

// Create a yellow-border (or blank) board with standard parameters
void TBoard::create(bool yellow_border) {
	short ix, iy, i;

	Name = "";
	Info.clear();

	// BOARD_WIDTH and BOARD_HEIGHT gives the width and height of
	// the part of the board that the player can see. This visible
	// region is surrounded by board edges.

	// In a very rare situation, the player may be at (1,0) and try
	// to go right. Then DOS ZZT will transport the player to the
	// neighbor board's initial position because the player at (1,0)
	// is preserved. Before this fix, it would cause a crash on Linux
	// due to violating the player invariant (the edge would overwrite
	// the player).

	for (ix = 0; ix <= BOARD_WIDTH+1; ix ++) {
		if (Tiles[ix][0].Element != E_PLAYER) {
			Tiles[ix][0] = TileBoardEdge;
		}
		if (Tiles[ix][BOARD_HEIGHT+1].Element != E_PLAYER) {
			Tiles[ix][BOARD_HEIGHT+1] = TileBoardEdge;
		}
	}

	for (iy = 0; iy <= BOARD_HEIGHT+1; iy ++) {
		if (Tiles[0][iy].Element != E_PLAYER) {
			Tiles[0][iy] = TileBoardEdge;
		}
		if (Tiles[BOARD_WIDTH+1][iy].Element != E_PLAYER) {
			Tiles[BOARD_WIDTH+1][iy] = TileBoardEdge;
		}
	}

	// Fill the interior with empties.

	for (ix = 1; ix <= BOARD_WIDTH; ix ++)
		for (iy = 1; iy <= BOARD_HEIGHT; iy ++) {
			Tiles[ix][iy].Element = E_EMPTY;
			Tiles[ix][iy].Color = 0;
		}

	// Then do the yellow border.

	if (yellow_border) {
		for (ix = 1; ix <= BOARD_WIDTH; ix ++) {
			Tiles[ix][1] = TileBorder;
			Tiles[ix][BOARD_HEIGHT] = TileBorder;
		}
		for (iy = 1; iy <= BOARD_HEIGHT; iy ++) {
			Tiles[1][iy] = TileBorder;
			Tiles[BOARD_WIDTH][iy] = TileBorder;
		}
	}

	// Place the player.
	// TODO: Use stats placement functions once refactored properly.
	// The stats' default values are now the same as below, but it's
	// still instructive to set the coordinates to match, as a stats
	// placement function would.

	Tiles[BOARD_WIDTH / 2][BOARD_HEIGHT / 2].Element = E_PLAYER;
	Tiles[BOARD_WIDTH / 2][BOARD_HEIGHT / 2].Color =
		element_info->defs[E_PLAYER].Color;

	StatCount = 0;
	Stats[0].clear();
	Stats[0].X = BOARD_WIDTH / 2;
	Stats[0].Y = BOARD_HEIGHT / 2;

	// CHANGE: Not actually done in DOS ZZT, but we want every
	// returned board to be judged valid by the checks in load(),
	// even if serialized right after create().

	Info.StartPlayerX = Stats[0].X;
	Info.StartPlayerY = Stats[0].Y;
}

bool TBoard::valid_coord(short x, short y) const {
	return x >= 0 && y >= 0 && x <= BOARD_WIDTH+1 && y <= BOARD_HEIGHT+1;
}

std::string TBoard::load(const std::vector<unsigned char> & source,
	int cur_board_id, int number_of_boards) {

	std::string errors = parse_board(source, cur_board_id, number_of_boards);
	adjust_board_stats();

	return errors;
}

std::string TBoard::load(const std::vector<unsigned char> & source) {
	return load(source, 0, MAX_BOARD);
}

std::vector<unsigned char> TBoard::dump() const {

	std::vector<unsigned char> out;
	out.reserve(MAX_RLE_OVERFLOW);

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

	// TODO: Check if this actually works, and if it replicates BIND
	// objects. RLEFLOW hangs; looks like the problem may be located here
	// somewhere.
	for (int stat_idx = 0; stat_idx <= StatCount; ++stat_idx) {
		TStat stat_to_dump = Stats[stat_idx];

		// Clean up #BIND references so they all point to the first object.
		if (stat_to_dump.DataLen > 0)  {
			for (int primary = 1; primary < stat_idx; ++primary) {
				/* If two pointers share the same code (same pointer),
				   link the latter to the former. */
				if (Stats[primary].data == Stats[stat_idx].data)  {
					stat_to_dump.DataLen = -primary;
					stat_to_dump.data = NULL;
					break;
				}
			}
		}

		stat_to_dump.dump(out);
	}

	return out;
}

std::string TBoard::dump_to_readable(int num_indents) const {
	std::string out, indent(num_indents, '\t');

	out += indent + "Board name: " + Name + "\n";
	out += indent + "Uncompressed tiles dump: " + "\n";
	for (int iy = 1; iy <= BOARD_HEIGHT; ++iy) {
		for (int ix = 1; ix <= BOARD_WIDTH; ++ix) {
			out += indent + "\t(" + itos(ix) + ", " + itos(iy) + ")\n";
			out += Tiles[ix][iy].dump_to_readable(num_indents+2,
					element_info);
		}
	}

	out += Info.dump_to_readable(num_indents+1);

	out += indent + "Number of stats: " + itos(StatCount) + "\n";
	for (int stat_idx = 0; stat_idx <= StatCount; ++stat_idx) {
		out += indent + "\tStat number " + itos(stat_idx) + ":\n";
		out += Stats[stat_idx].dump_to_readable(num_indents+2, false,
				element_info);
	}

	return out;
}

size_t TBoard::get_packed_size() const {
	if (cached_packed_size == -1) {
		cached_packed_size = dump().size();
	}

	return cached_packed_size;
}

std::vector<unsigned char> TBoard::dump_and_truncate(
	std::string & out_load_error) {

	std::vector<unsigned char> out = dump();

	/* If we're using too much space, truncate the size, feed the
		whole thing back through load() to fix the inevitable
		corruption, then run BoardClose again.
		This smart-ass solution should allow us to keep all the smarts of
		board parsing in BoardOpen and nowt have to duplicate any logic.

		Such a situation should *only* happen if RLE is too large (see
		RLEFLOW.ZZT), because AddStat should reject adding stats when
		there's no room.; */
	if (out.size() > MAX_BOARD_LEN) {
		// Maybe I should try actually resizing this.... oops :-P
		out.resize(MAX_BOARD_LEN);
		out_load_error = load(out);
		// Propagate the error by ignoring any error we get from the
		// second dump.
		std::string throwaway;
		return dump_and_truncate(throwaway);
	}

	return out;
}

// For debugging/testing purposes.
std::vector<unsigned char> TBoard::dump_and_truncate() {
	std::string throwaway;
	return (dump_and_truncate(throwaway));
}

bool TBoard::add_stat(size_t x, size_t y, element_t element,
	unsigned char color, short cycle, TStat stat_template) {

	/* First of all: check if we have space. If not, no can do! */
	if (get_packed_size() + stat_template.upper_bound_packed_size() >
		MAX_BOARD_LEN) {
		return false;
	}

	/* Can't put anything on top of the player. */
	if (x == Stats[0].X && y == Stats[0].Y) {
		return false;
	}

	/* ... or out of bounds. */
	if (!valid_coord(x, y)) {
		return false;
	}

	// Can't make more than MAX_STAT stats.
	if (StatCount >= MAX_STAT) {
		return false;
	}

	++StatCount;
	Stats[StatCount] = stat_template;

	Stats[StatCount].X = x;
	Stats[StatCount].Y = y;
	Stats[StatCount].Cycle = cycle;
	Stats[StatCount].Under = Tiles[x][y];
	Stats[StatCount].DataPos = 0;

	// Copy the data.
	if (stat_template.DataLen > 0) {
		Stats[StatCount].data = std::shared_ptr<unsigned char[]>(
				new unsigned char[stat_template.DataLen]);

		std::copy(stat_template.data.get(),
			stat_template.data.get() + stat_template.DataLen,
			Stats[StatCount].data.get());
	}

	if (element_info->defs[Tiles[x][y].Element].PlaceableOnTop) {
		Tiles[x][y].Color = (color & 0xf) + (Tiles[x][y].Color & 0x70);
	} else {
		Tiles[x][y].Color = color;
	}

	Tiles[x][y].Element = element;

	// Update board size.
	if (cached_packed_size != -1) {
		cached_packed_size += Stats[StatCount].marginal_packed_size();
	}

	return true;
}

bool TBoard::remove_stat(int statId, int & out_x, int & out_y,
	short & current_stat_ticked) {

	size_t i;

	if (statId > MAX_STAT) {
		throw std::logic_error(
			"Trying to remove statId greater than MAX_STAT");
	}
	if (statId == -1) {
		throw std::logic_error(
			"Trying to remove noexisting stat (-1)");
	}
	if (statId == 0) {	// Can't remove the player.
		return false;
	}
	if (statId > StatCount) {
		return false;    /* Already removed. */
	}

	// Update board size and output parameters.
	if (cached_packed_size != -1) {
		cached_packed_size -= Stats[statId].marginal_packed_size();
	}

	out_x = Stats[statId].X;
	out_y = Stats[statId].Y;

	TStat & with = Stats[statId];

	if (statId < current_stat_ticked) {
		current_stat_ticked = current_stat_ticked - 1;
	}

	/* Don't remove the player if he's at the old position. This can
	   happen with multiple stats with the same coordinate. */
	if ((with.X != Stats[0].X) || (with.Y != Stats[0].Y)) {
		Tiles[with.X][with.Y] = with.Under;
	}

	// Update leaders and followers.
	for (i = 1; i <= StatCount; i ++) {
		if (Stats[i].Follower >= statId)  {
			if (Stats[i].Follower == statId) {
				Stats[i].Follower = -1;
			} else {
				Stats[i].Follower = Stats[i].Follower - 1;
			}
		}

		if (Stats[i].Leader >= statId)  {
			if (Stats[i].Leader == statId) {
				Stats[i].Leader = -1;
			} else {
				Stats[i].Leader = Stats[i].Leader - 1;
			}
		}
	}

	// Clear the stat and deallocate data.
	Stats[statId] = TStat();

	// Shift every stat with a greater index one down.
	for (i = statId + 1; i <= StatCount; ++i) {
		Stats[i - 1] = Stats[i];
	}
	StatCount = StatCount - 1;

	return true;
}

// XXX: Doesn't support blinking, find out how to do that later.
bool TBoard::add_char(size_t x, size_t y, char character,
	unsigned char color) {

	if (x == 0 || x > BOARD_WIDTH || y == 0 || y > BOARD_HEIGHT) {
		return false;
	}

	element_t text_type;

	switch(color & 7) {
		case Black:		text_type = E_TEXT_WHITE; break;
		case Blue:		text_type = E_TEXT_BLUE; break;
		case Green:		text_type = E_TEXT_GREEN; break;
		case Cyan:		text_type = E_TEXT_CYAN; break;
		case Red:		text_type = E_TEXT_RED; break;
		case Magenta:	text_type = E_TEXT_PURPLE; break;
		case Brown:		text_type = E_TEXT_YELLOW; break;
		case LightGray:	text_type = E_TEXT_WHITE; break;
	}

	Tiles[x][y].Color = character;
	Tiles[x][y].Element = text_type;

	return true;
}

bool TBoard::add_string(size_t x, size_t y, std::string text,
	unsigned char color) {

	for (size_t i = 0; i < text.size(); ++i) {
		if (!add_char(x + i, y, text[i], color)) {
			return false;
		}
	}

	return true;
}
