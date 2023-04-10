#pragma once

#include <array>
#include <string>

#include "indices.h"

class ElementInfoDef {
	public:
		char Character;

		// must be unsigned due to oop.cxx:GetColorForTileMatch
		unsigned char Color;

		bool Destructible;
		bool Pushable;
		bool VisibleInDark;
		bool PlaceableOnTop;
		bool Walkable;
		short Cycle;
		short EditorCategory;
		char EditorShortcut;
		std::string Name;
		std::string CategoryName;
		std::string Param1Name;
		std::string Param2Name;
		std::string ParamBulletTypeName;
		std::string ParamBoardName;
		std::string ParamDirName;
		std::string ParamTextName;
		short ScoreValue;

		ElementInfoDef();
};

class ElementInfo {
	public:
		std::array<ElementInfoDef, MAX_ELEMENT+1> defs;

		ElementInfo();
};