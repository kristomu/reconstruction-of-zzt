#pragma once

#include <string>
#include <vector>
#include <array>

#include "board.h"

const int MAX_FLAG = 10;

class TWorldInfo {
	private:
		bool Keys[7];
	public:
		short Ammo;
		short Gems;
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

		// Set default values.
		void clear() {
			IsSave = false;
			Ammo = 0;
			Gems = 0;
			Health = 100;
			EnergizerTicks = 0;
			Torches = 0;
			TorchTicks = 0;
			Score = 0;
			BoardTimeSec = 0;
			BoardTimeHsec = 0;

			for (int i = 1; i <= 7; i ++) {
				TakeKey(i);
				Flags[i] = "";
			}
		}

		TWorldInfo() {
			CurrentBoardIdx = 0;
			clear();
		}
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

		// Set close_board if you want the save function to
		// serialize the current board before saving. This is usually
		// the right thing to do, but in ZZT itself, the board closing has
		// to be done separately to warn the user if truncation happened.
		void save(std::ostream & stream, bool close_board);

		// This ostream operator function does serialize the current
		// board before saving.
		std::ostream & operator<< (std::ostream & stream);

		// We need to pass element info to the board so that it can correctly
		// remove stuff from tiles with stats. This is kinda ugly...
		TWorld(std::shared_ptr<ElementInfo> element_info_ptr) :
			currentBoard(element_info_ptr) {}
};