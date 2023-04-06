#pragma once

#include <string>
#include <vector>
#include <array>

#include "board.h"
#include "editor.h"

const int MAX_FLAG = 10;

class TWorldInfo {
	private:
		bool Keys[7];
	public:
		short Ammo;
		short Gems;
		//array<1,7,bool> Keys;
		short Health;
		short CurrentBoardIdx;
		short Torches;
		short TorchTicks;
		short EnergizerTicks;
		short unk1;
		short Score;
		std::string Name;
		std::array<std::string, MAX_FLAG+1> Flags;
		short BoardTimeSec;
		short BoardTimeHsec;
		bool IsSave;
		std::array<unsigned char, 13> unkPad;

		size_t packed_size() const {
			return 7 + sizeof(Ammo) + sizeof(Gems) + sizeof(Health) +
				sizeof(CurrentBoardIdx) + sizeof(Torches) + sizeof(TorchTicks) +
				sizeof(EnergizerTicks) + sizeof(unk1) + sizeof(Score) + 21 +
				21 * MAX_FLAG + sizeof(BoardTimeSec) + sizeof(BoardTimeHsec) +
				sizeof(IsSave) + 13;
		}

		// Hiding keys like that makes it easier to deal with the black key
		// weirdness and offset-by-one stuff.
		std::string KeyName(int keyColor) const;
		bool HasKey(int keyColor) const;
		void GiveKey(int keyColor);
		bool TakeKey(int keyColor);

		void dump(std::vector<unsigned char> & out) const;

		std::vector<unsigned char>::const_iterator load(
			std::vector<unsigned char>::const_iterator ptr,
			const std::vector<unsigned char>::const_iterator end);
};

class TWorld {
	public:
		short BoardCount;
		TBoard currentBoard;
		// dynamic board length.
		// TODO later, turn this into a vector of vectors and use .size()
		// instead of BoardCount.
		std::array<std::vector<unsigned char>, MAX_BOARD+1> BoardData;
		TWorldInfo Info;
		std::array<TEditorStatSetting, MAX_ELEMENT+1> EditorStatSettings;
};

extern TWorld World;