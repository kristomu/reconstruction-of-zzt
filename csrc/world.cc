#include "world.h"
#include "serialization.h"

bool TWorldInfo::HasKey(int keyColor) const {
	// TODO: Black key functionality
	if (keyColor < 1 || keyColor >= 7) return false;

	return Keys[keyColor-1];
}

void TWorldInfo::GiveKey(int keyColor) {
	if (keyColor < 1 || keyColor >= 7) return;

	Keys[keyColor-1] = true;
}

// Returns false if you don't have the key.
bool TWorldInfo::TakeKey(int keyColor) {
	if (!HasKey(keyColor)) return false;

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