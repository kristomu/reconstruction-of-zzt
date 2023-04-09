#include "world.h"
#include "cgamevars.h"
#include "serialization.h"

#include <cstring>
#include <vector>

std::string TWorldInfo::KeyName(int keyColor) const {
	keyColor = keyColor % 8;

	if (keyColor == 0) {
		// Emulate ZZT bug
		return ".-\05....\04Blue    \05Green   \04Cyan    \03Red     "
			"\06Yellow  \05White";
	}

	return ColorNames[keyColor];

}

bool TWorldInfo::HasKey(int keyColor) const {
	// TODO: Black key functionality
	if (keyColor < 1) {
		return false;
	}

	keyColor = keyColor % 8;

	// Emulate ZZT bug: a bit in the gems field decides whether the player
	// has the black key.
	if (keyColor == 0) {
		return Gems >= 256;
	}

	return Keys[keyColor-1];
}

void TWorldInfo::GiveKey(int keyColor) {
	if (keyColor < 1 || keyColor >= 7) {
		return;
	}

	keyColor = keyColor % 8;

	if (keyColor == 0) {
		Gems += 256;
	}

	Keys[keyColor-1] = true;
}

// Returns false if you don't have the key.
bool TWorldInfo::TakeKey(int keyColor) {
	if (!HasKey(keyColor)) {
		return false;
	}

	if (keyColor == 0) {
		Gems -= 256;
	}

	Keys[keyColor-1] = false;

	return true;
}

void TWorldInfo::dump(std::vector<unsigned char> & out) const {

	append_array(std::vector<short> {Ammo, Gems}, out);
	append_array(Keys, 7, out);
	append_array(std::vector<short> {Health, CurrentBoardIdx, Torches,
			TorchTicks, EnergizerTicks, unk1, Score
		}, out);
	append_pascal_string(Name, 20, out);		// World name (pascal string)
	// Flags
	for (size_t i = 1; i <= MAX_FLAG; ++i) {
		append_pascal_string(Flags[i], 20, out);
	}
	append_array(std::vector<short> {BoardTimeSec, BoardTimeHsec}, out);
	append_lsb_element(IsSave, out);
	append_zeroes(14, out);
}

std::vector<unsigned char>::const_iterator TWorldInfo::load(
	std::vector<unsigned char>::const_iterator ptr,
	const std::vector<unsigned char>::const_iterator end) {

	// TODO: Do something with this truncation flag.
	bool truncated = false;

	if (end - ptr < (int)packed_size()) {
		throw std::runtime_error("Insufficient data to load TWorldInfo");
	}

	ptr = load_lsb_element(ptr, Ammo);
	ptr = load_lsb_element(ptr, Gems);
	ptr = load_array(ptr, 7, Keys);
	ptr = load_lsb_element(ptr, Health);
	ptr = load_lsb_element(ptr, CurrentBoardIdx);
	ptr = load_lsb_element(ptr, Torches);
	ptr = load_lsb_element(ptr, TorchTicks);
	ptr = load_lsb_element(ptr, EnergizerTicks);
	ptr = load_lsb_element(ptr, unk1);
	ptr = load_lsb_element(ptr, Score);
	ptr = get_pascal_string(ptr, end, 20, true, Name, truncated);

	for (size_t i = 1; i <= MAX_FLAG; ++i) {
		ptr = get_pascal_string(ptr, end, 20, true, Flags[i], truncated);
	}
	ptr = load_lsb_element(ptr, BoardTimeSec);
	ptr = load_lsb_element(ptr, BoardTimeHsec);
	ptr = load_lsb_element(ptr, IsSave);
	ptr += 14; // Skip padding

	return ptr;
}

void TWorld::save(std::ostream & stream, bool close_board) {

	if (close_board) {
		BoardData[Info.CurrentBoardIdx] =
			currentBoard.dump_and_truncate();
	}

	std::vector<unsigned char> world_header;
	append_lsb_element((short)-1, world_header); // Version
	append_lsb_element(BoardCount, world_header);
	Info.dump(world_header);
	// Pad to 512
	append_zeroes(512-world_header.size(), world_header);

	stream.write((const char *)world_header.data(), 512);

	// TODO? Replace with some kind of C++ file error detection.
	if (errno != 0) {
		throw std::runtime_error(strerror(errno));
	}

	for (int i = 0; i <= BoardCount; i ++) {
		// TODO: Replace with a serialization procedure that's
		// machine endian agnostic.
		unsigned short board_len = BoardData[i].size();
		stream.write((char *)&board_len, 2);
		if (errno != 0) {
			throw std::runtime_error(strerror(errno));
		}

		stream.write((const char *)BoardData[i].data(),
			BoardData[i].size());
	}
}

std::ostream & TWorld::operator<< (std::ostream & stream) {
	save(stream, true);
	return stream;
}
