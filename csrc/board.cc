#include "ptoc.h"
#include "gamevars.h"
#include "array.h"
#include "serialization.h"

const size_t MAX_BOARD_NAME_LENGTH = 50;

void TTile::dump(std::vector<unsigned char> & out) const {
	append_lsb_element(Element, out);
	append_lsb_element(Color, out);
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

// Returns true if the board is in OK condition or false if it is
// corrupted. This function is also has an extreme number of checks
// against potential corruption; the point is to pass every fuzzed
// crash test and deal with corrupted boards as gracefully as possible.
bool TBoard::open(const std::vector<unsigned char> & source) {
	std::vector<unsigned char>::const_iterator ptr = source.begin();
#ifdef TO_BE_DONE

	short i, ix, iy;

	short bytesRead;
	bool boardIsDamaged = false, truncated = false;

	bytesRead = 0;

	/* Create a default yellow border board, because we might need
	to abort before the board is fully specced. */
	create();

	/* Check that the sanity check on board titles have been executed. */

	/* SANITY: Reconstruct the title. We need at least a size of two bytes
	   for the title: a size designation and the first letter of the title.
	   If we don't even have that, let the title be blank.*/

	// XXX: Must be outside because it refers to the world state.

	//World.Info.CurrentBoard = boardId;
	// We assume the vector has been set to reflect either the data
	// read from the file, or the world boardLen parameter, whichever
	// is smaller. The outside should check that boardLen matches.
	// TODO: Handle cases where the board is smaller than expected and
	// the excess bytes can be interpreted as the next board instead...
	// But in that case, I think it just skips the excess (trusting the
	// metadata).

	// ---------------- Board name  ----------------
	size_t board_length = source.size();

	if (board_length > 1)  {
		ptr = get_pascal_string(ptr, source.end(),
			MAX_BOARD_NAME_LENGTH, Name, truncated);
	} else {
		Name = "";
		truncated = true;
	}

	if (truncated) {
		/* This board is damaged. */
		return false;
	}

	ptr += MAX_BOARD_NAME_LENGTH + 1;
	bytesRead += MAX_BOARD_NAME_LENGTH + 1;

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
			if (bytesRead + sizeof(rle) > board_length)  break;
			rle.Count = *ptr++;
			rle.Tile.Element = *ptr++;
			rle.Tile.Color = *ptr++;

            bytesRead += 3;
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

	} while (!((iy > BOARD_HEIGHT) || (bytesRead >= board_length)));

	// MORE TO COME ...

	// ---------------- Metadata and stats info  ----------------

	/* SANITY: If reading board info and the stats count byte would
	get us out of bounds, we have a board that's truncated too early.
		  Do the best we can, then show the damaged board note and exit. */
	if ((sizeof(Info) + sizeof(StatCount) + bytesRead) >=
	        board_length)  {
		//World.Info.CurrentBoard = boardId;
		AdjustBoardStats();

		return false;
	}

    bcopy(ptr, &Info, sizeof(Info));
	ptr += sizeof(Info);
	bytesRead = bytesRead + sizeof(Info);

	/* Clamp out-of-bounds Info variables. They'll cause problems
	in the editor otherwise. */
	// TODO: Put this outside, or pass in the BoardCount variable...
	/*for( i = 0; i <= 3; i ++)
		/v* This behavior is from elements.pas, BoardEdgeTouch. *v/
		if (Info.NeighborBoards[i] > World.BoardCount)
			Info.NeighborBoards[i] = boardId;*/

	/* Clamp an out-of-board player location, if there is any. */
	if (! ValidCoord(Info.StartPlayerX, Info.StartPlayerY))  {
		Info.StartPlayerX = 1;
		Info.StartPlayerY = 1;
		boardIsDamaged = true;
	}

	// This is just a short in little endian format.

    bcopy(ptr, &StatCount, sizeof(StatCount));
	ptr += sizeof(StatCount);
	bytesRead = bytesRead + sizeof(StatCount);

	StatCount = Max(0, Min(StatCount, MAX_STAT));

	for( ix = 0; ix <= StatCount; ix ++) {
		TStat& with = Stats[ix];
		/* SANITY: Handle too few stats items for the stats count. */
		if ((bytesRead + sizeof(TStat)) > board_length)  {
			StatCount = Max(ix - 1, 0);
			World.Info.CurrentBoard = boardId;
			boardIsDamaged = true;
			break;
		}

		MoveP(*ptr, Stats[ix], sizeof(TStat));
		ptr += sizeof(TStat);
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
		if (bytesRead + with.DataLen > board_length)  {
			with.DataLen = board_length - bytesRead;
			boardIsDamaged = true;
		}

		if (with.DataLen > 0)
			/* SANITY: If DataLen is too long, truncate it. */
			if (with.DataLen > board_length-bytesRead)  {
				with.DataLen = board_length-bytesRead;
				boardIsDamaged = true;
			}

		/* Only allocate if data length is still positive... */
		if (with.DataLen > 0)  {
			//GetMem(with.Data, with.DataLen);
            with.Data = (unsigned char *)malloc(with.DataLen);
			MoveP(*ptr, *with.Data, with.DataLen);
			ptr += with.DataLen;
			bytesRead = bytesRead + with.DataLen;
		}

		/* Otherwise, clear Data to avoid potential leaks later. */
		if (with.DataLen == 0)  with.Data = nil;
	}

	AdjustBoardStats();
	World.Info.CurrentBoard = boardId;

	if (boardIsDamaged)
		DisplayCorruptionNote();
#endif
	return false;

}

std::vector<unsigned char> TBoard::dump() {
	integer ix, iy;

	std::vector<unsigned char> out;
	out.reserve(MAX_RLE_OVERFLOW);

	std::vector<unsigned char>::const_iterator ptr = out.begin(),
		ptr_start;

	ptr_start = ptr;

	TRleTile rle;
	boolean cleanupNeeded = false;

	// ---------------- Board name  ----------------

	append_pascal_string(Name, MAX_BOARD_NAME_LENGTH, out);

	// ---------------- RLE board layout  ----------------

	// To refactor later, because this is uh-glee.

	ix = 1;
	iy = 1;
	rle.Count = 1;
	rle.Tile = Tiles[ix][iy];
	do {
		ix = ix + 1;
		if (ix > BOARD_WIDTH)  {
			ix = 1;
			iy = iy + 1;
		}
		if ((Tiles[ix][iy].Color == rle.Tile.Color) &&
		        (Tiles[ix][iy].Element == rle.Tile.Element) &&
		        (rle.Count < 255) && (iy <= BOARD_HEIGHT)) {
			rle.Count = rle.Count + 1;
		} else {
			out.push_back(rle.Count);
			rle.Tile.dump(out);

			rle.Tile = Tiles[ix][iy];
			rle.Count = 1;
		}
	} while (!(iy > BOARD_HEIGHT));

	// ---------------- Metadata and stats info  ----------------

	Info.dump(out);

	// ---------------- Stats  ----------------

	append_lsb_element(StatCount, out);

	for( ix = 0; ix <= StatCount; ix ++) {
		// Clean up #BIND references so they all point to the first object.
		if (Stats[ix].DataLen > 0)  {
			for( iy = 1; iy < ix; iy ++) {
				/* IMP: Make all bound objects link to the same one. */
				if (Stats[iy].Data == Stats[ix].Data)  {
					Stats[ix].DataLen = -iy;
					Stats[ix].Data = nil;
					break;
				}
			}
		}

		Stats[ix].dump(out);
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