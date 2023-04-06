#include "world.h"
#include "serialization.h"
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

	if (end - ptr < packed_size()) {
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

/*void WorldSave(std::string filename, std::string extension) {
	int i;
	int unk1;
	TIoTmpBuf * ptr;
	int version;


	BoardClose(true);
	video.write(63, 5, 0x1f, "Saving...");

	std::string full_filename = std::string(filename + extension);
	std::ofstream out_file = OpenForWrite(full_filename);

	// TODO IMP? Perhaps write to a temporary filename and then move it over
	// the original to create some kind of atomicity?
	// https://en.cppreference.com/w/cpp/io/c/tmpnam etc.

	if (! DisplayIOError())  {
		std::vector<unsigned char> world_header;
		append_lsb_element((short)-1, world_header); // Version
		append_lsb_element(World.BoardCount, world_header);
		World.Info.dump(world_header);
		// Pad to 512
		append_zeroes(512-world_header.size(), world_header);

		word actually_written;
		out_file.write((const char *)world_header.data(), 512);

		if (DisplayIOError())  {
			goto LOnError;
		}

		for (i = 0; i <= World.BoardCount; i ++) {
			// TODO: Replace with a serialization procedure that's
			// machine endian agnostic.
			unsigned short board_len = World.BoardData[i].size();
			out_file.write((char *)&board_len, 2);
			if (DisplayIOError())  {
				goto LOnError;
			}

			out_file.write((const char *)World.BoardData[i].data(),
				World.BoardData[i].size());

			if (DisplayIOError())  {
				goto LOnError;
			}
		}

		out_file.close();
	}

	BoardOpen(World.Info.CurrentBoard, false);
	SidebarClearLine(5);
	return;

LOnError:
	out_file.close();
	std::remove(full_filename.c_str()); // Delete the corrupted file.
	// IMP? Give error message? But the above already does.
	BoardOpen(World.Info.CurrentBoard, false);
	SidebarClearLine(5);
}*/