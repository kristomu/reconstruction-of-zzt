#include "world.h"
#include "serialization.h"
#include "gamevars.h"

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
	if (keyColor < 1) return false;

	keyColor = keyColor % 8;

	// Emulate ZZT bug: a bit in the gems field decides whether the player
	// has the black key.
	if (keyColor == 0) {
		return Gems >= 256;
	}

	return Keys[keyColor-1];
}

void TWorldInfo::GiveKey(int keyColor) {
	if (keyColor < 1 || keyColor >= 7) return;

	keyColor = keyColor % 8;

	if (keyColor == 0) {
		Gems += 256;
	}

	Keys[keyColor-1] = true;
}

// Returns false if you don't have the key.
bool TWorldInfo::TakeKey(int keyColor) {
	if (!HasKey(keyColor)) return false;

	if (keyColor == 0) {
		Gems -= 256;
	}

	Keys[keyColor-1] = false;

	return true;
}

void TWorldInfo::dump(std::vector<unsigned char> & out) const {

	append_array(std::vector<short>{Ammo, Gems}, out);
	append_array(Keys, 7, out);
	append_array(std::vector<short>{Health, CurrentBoard, Torches, TorchTicks,
		EnergizerTicks, unk1, Score}, out);
	append_pascal_string(Name, 20, out);		// World name (pascal string)
	// Flags
	for (size_t i = 1; i <= MAX_FLAG; ++i) {
		append_pascal_string(Flags[i], 20, out);
	}
	append_array(std::vector<short>{BoardTimeSec, BoardTimeHsec}, out);
	append_lsb_element(IsSave, out);
	append_zeroes(14, out);
}