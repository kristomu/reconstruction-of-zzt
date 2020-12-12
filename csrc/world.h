#pragma once

#include "ptoc.h"
#include "array.h"

#include <vector>
#include <array>

const integer MAX_FLAG = 10;

class TWorldInfo {
	private:
		bool Keys[7];
	public:
		short Ammo;
		short Gems;
		//array<1,7,bool> Keys;
		short Health;
		short CurrentBoard;
		short Torches;
		short TorchTicks;
		short EnergizerTicks;
		short unk1;
		short Score;
		asciiz Name;
		array<1 , MAX_FLAG,asciiz> Flags;
		short BoardTimeSec;
		short BoardTimeHsec;
		bool IsSave;
		std::array<byte, 13> unkPad;

		// Hiding keys like that makes it easier to deal with the black key
		// weirdness and offset-by-one stuff.
		bool HasKey(int keyColor) const;
		void GiveKey(int keyColor);
		bool TakeKey(int keyColor);

		void dump(std::vector<unsigned char> & out) const;
};