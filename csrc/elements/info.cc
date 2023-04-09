#include "info.h"
#include "../cgamevars.h"

// Set the default properties that an element assumes unless otherwise
// specified.

ElementInfoDef::ElementInfoDef() {
	Character = ' ';
	Color = COLOR_CHOICE_ON_BLACK;
	Destructible = false;
	Pushable = false;
	VisibleInDark = false;
	PlaceableOnTop = false;
	Walkable = false;
	Cycle = -1;
	EditorCategory = 0;
	EditorShortcut = '\0';
	Name = "";
	CategoryName = "";
	Param1Name = "";
	Param2Name = "";
	ParamBulletTypeName = "";
	ParamBoardName = "";
	ParamDirName = "";
	ParamTextName = "";
	ScoreValue = 0;
}

// Initialize the various properties of the different elements.
// It might be better to make the ElementInfo also contain touch/tick/etc, but
// pass it a dummy for editor and external libraries that don't care about
// gameplay. Consider that possibility later... TODO?

ElementInfo::ElementInfo() {

	defs[0].Character = ' ';
	defs[0].Color = 0x70;
	defs[0].Pushable = true;
	defs[0].Walkable = true;
	defs[0].Name = "Empty";

	defs[3].Character = ' ';
	defs[3].Color = 0x7;
	defs[3].Cycle = 1;
	defs[3].Name = "Monitor";

	defs[19].Character = '\260';
	defs[19].Color = 0xf9;
	defs[19].PlaceableOnTop = true;
	defs[19].EditorCategory = CATEGORY_TERRAIN;
	defs[19].EditorShortcut = 'W';
	defs[19].Name = "Water";
	defs[19].CategoryName = "Terrains:";

	defs[20].Character = '\260';
	defs[20].Color = 0x20;
	defs[20].Walkable = false;
	defs[20].EditorCategory = CATEGORY_TERRAIN;
	defs[20].EditorShortcut = 'F';
	defs[20].Name = "Forest";

	defs[4].Character = '\2';
	defs[4].Color = 0x1f;
	defs[4].Destructible = true;
	defs[4].Pushable = true;
	defs[4].VisibleInDark = true;
	defs[4].Cycle = 1;
	defs[4].EditorCategory = CATEGORY_ITEM;
	defs[4].EditorShortcut = 'Z';
	defs[4].Name = "Player";
	defs[4].CategoryName = "Items:";

	defs[41].Character = '\352';
	defs[41].Color = 0xc;
	defs[41].Destructible = true;
	defs[41].Pushable = true;
	defs[41].Cycle = 2;
	defs[41].EditorCategory = CATEGORY_CREATURE;
	defs[41].EditorShortcut = 'L';
	defs[41].Name = "Lion";
	defs[41].CategoryName = "Beasts:";
	defs[41].Param1Name = "Intelligence?";
	defs[41].ScoreValue = 1;

	defs[42].Character = '\343';
	defs[42].Color = 0xb;
	defs[42].Destructible = true;
	defs[42].Pushable = true;
	defs[42].Cycle = 2;
	defs[42].EditorCategory = CATEGORY_CREATURE;
	defs[42].EditorShortcut = 'T';
	defs[42].Name = "Tiger";
	defs[42].Param1Name = "Intelligence?";
	defs[42].Param2Name = "Firing rate?";
	defs[42].ParamBulletTypeName = "Firing type?";
	defs[42].ScoreValue = 2;

	defs[44].Character = '\351';
	defs[44].Destructible = true;
	defs[44].Cycle = 2;
	defs[44].EditorCategory = CATEGORY_CREATURE;
	defs[44].EditorShortcut = 'H';
	defs[44].Name = "Head";
	defs[44].CategoryName = "Centipedes";
	defs[44].Param1Name = "Intelligence?";
	defs[44].Param2Name = "Deviance?";
	defs[44].ScoreValue = 1;

	defs[45].Character = 'O';
	defs[45].Destructible = true;
	defs[45].Cycle = 2;
	defs[45].EditorCategory = CATEGORY_CREATURE;
	defs[45].EditorShortcut = 'S';
	defs[45].Name = "Segment";
	defs[45].ScoreValue = 3;

	defs[18].Character = '\370';
	defs[18].Color = 0xf;
	defs[18].Destructible = true;
	defs[18].Cycle = 1;
	defs[18].Name = "Bullet";

	defs[15].Character = 'S';
	defs[15].Color = 0xf;
	defs[15].Destructible = false;
	defs[15].Cycle = 1;
	defs[15].Name = "Star";

	defs[8].Character = '\14';
	defs[8].Pushable = true;
	defs[8].EditorCategory = CATEGORY_ITEM;
	defs[8].EditorShortcut = 'K';
	defs[8].Name = "Key";

	defs[5].Character = '\204';
	defs[5].Color = 0x3;
	defs[5].Pushable = true;
	defs[5].EditorCategory = CATEGORY_ITEM;
	defs[5].EditorShortcut = 'A';
	defs[5].Name = "Ammo";

	defs[7].Character = '\4';
	defs[7].Pushable = true;
	defs[7].Destructible = true;
	defs[7].EditorCategory = CATEGORY_ITEM;
	defs[7].EditorShortcut = 'G';
	defs[7].Name = "Gem";

	defs[11].Character = '\360';
	defs[11].Color = COLOR_WHITE_ON_CHOICE;
	defs[11].Cycle = 0;
	defs[11].VisibleInDark = true;
	defs[11].EditorCategory = CATEGORY_ITEM;
	defs[11].EditorShortcut = 'P';
	defs[11].Name = "Passage";
	defs[11].ParamBoardName = "Room thru passage?";

	defs[9].Character = '\12';
	defs[9].Color = COLOR_WHITE_ON_CHOICE;
	defs[9].EditorCategory = CATEGORY_ITEM;
	defs[9].EditorShortcut = 'D';
	defs[9].Name = "Door";

	defs[10].Character = '\350';
	defs[10].Color = 0xf;
	defs[10].Pushable = true;
	defs[10].Cycle = 1;
	defs[10].EditorCategory = CATEGORY_ITEM;
	defs[10].EditorShortcut = 'S';
	defs[10].Name = "Scroll";
	defs[10].ParamTextName = "Edit text of scroll";

	defs[12].Character = '\372';
	defs[12].Color = 0xf;
	defs[12].Cycle = 2;
	defs[12].EditorCategory = CATEGORY_ITEM;
	defs[12].EditorShortcut = 'U';
	defs[12].Name = "Duplicator";
	defs[12].ParamDirName = "Source direction?";
	defs[12].Param2Name = "Duplication rate?;SF";

	defs[6].Character = '\235';
	defs[6].Color = 0x6;
	defs[6].VisibleInDark = true;
	defs[6].EditorCategory = CATEGORY_ITEM;
	defs[6].EditorShortcut = 'T';
	defs[6].Name = "Torch";

	defs[39].Character = '\30';
	defs[39].Cycle = 2;
	defs[39].EditorCategory = CATEGORY_CREATURE;
	defs[39].EditorShortcut = 'G';
	defs[39].Name = "Spinning gun";
	defs[39].Param1Name = "Intelligence?";
	defs[39].Param2Name = "Firing rate?";
	defs[39].ParamBulletTypeName = "Firing type?";

	defs[35].Character = '\5';
	defs[35].Color = 0xd;
	defs[35].Destructible = true;
	defs[35].Pushable = true;
	defs[35].Cycle = 1;
	defs[35].EditorCategory = CATEGORY_CREATURE;
	defs[35].EditorShortcut = 'R';
	defs[35].Name = "Ruffian";
	defs[35].Param1Name = "Intelligence?";
	defs[35].Param2Name = "Resting time?";
	defs[35].ScoreValue = 2;

	defs[34].Character = '\231';
	defs[34].Color = 0x6;
	defs[34].Destructible = true;
	defs[34].Pushable = true;
	defs[34].Cycle = 3;
	defs[34].EditorCategory = CATEGORY_CREATURE;
	defs[34].EditorShortcut = 'B';
	defs[34].Name = "Bear";
	defs[34].CategoryName = "Creatures:";
	defs[34].Param1Name = "Sensitivity?";
	defs[34].ScoreValue = 1;

	defs[37].Character = '*';
	defs[37].Color = COLOR_CHOICE_ON_BLACK;
	defs[37].Destructible = false;
	defs[37].Cycle = 3;
	defs[37].EditorCategory = CATEGORY_CREATURE;
	defs[37].EditorShortcut = 'V';
	defs[37].Name = "Slime";
	defs[37].Param2Name = "Movement speed?;FS";

	defs[38].Character = '^';
	defs[38].Color = 0x7;
	defs[38].Destructible = false;
	defs[38].Cycle = 3;
	defs[38].EditorCategory = CATEGORY_CREATURE;
	defs[38].EditorShortcut = 'Y';
	defs[38].Name = "Shark";
	defs[38].Param1Name = "Intelligence?";

	defs[16].Character = '/';
	defs[16].Cycle = 3;
	defs[16].EditorCategory = CATEGORY_ITEM;
	defs[16].EditorShortcut = '1';
	defs[16].Name = "Clockwise";
	defs[16].CategoryName = "Conveyors:";

	defs[17].Character = '\\';
	defs[17].Cycle = 2;
	defs[17].EditorCategory = CATEGORY_ITEM;
	defs[17].EditorShortcut = '2';
	defs[17].Name = "Counter";

	defs[21].Character = '\333';
	defs[21].EditorCategory = CATEGORY_TERRAIN;
	defs[21].CategoryName = "Walls:";
	defs[21].EditorShortcut = 'S';
	defs[21].Name = "Solid";

	defs[22].Character = '\262';
	defs[22].EditorCategory = CATEGORY_TERRAIN;
	defs[22].EditorShortcut = 'N';
	defs[22].Name = "Normal";

	defs[31].Character = '\316';
	defs[31].Name = "Line";

	defs[43].Character = '\272';

	defs[33].Character = '\315';

	defs[32].Character = '*';
	defs[32].Color = 0xa;
	defs[32].EditorCategory = CATEGORY_TERRAIN;
	defs[32].EditorShortcut = 'R';
	defs[32].Name = "Ricochet";

	defs[23].Character = '\261';
	defs[23].Destructible = false;
	defs[23].EditorCategory = CATEGORY_TERRAIN;
	defs[23].EditorShortcut = 'B';
	defs[23].Name = "Breakable";

	defs[24].Character = '\376';
	defs[24].Pushable = true;
	defs[24].EditorCategory = CATEGORY_TERRAIN;
	defs[24].EditorShortcut = 'O';
	defs[24].Name = "Boulder";

	defs[25].Character = '\22';
	defs[25].EditorCategory = CATEGORY_TERRAIN;
	defs[25].EditorShortcut = '1';
	defs[25].Name = "Slider (NS)";

	defs[26].Character = '\35';
	defs[26].EditorCategory = CATEGORY_TERRAIN;
	defs[26].EditorShortcut = '2';
	defs[26].Name = "Slider (EW)";

	defs[30].Character = '\305';
	defs[30].Cycle = 2;
	defs[30].EditorCategory = CATEGORY_TERRAIN;
	defs[30].EditorShortcut = 'T';
	defs[30].Name = "Transporter";
	defs[30].ParamDirName = "Direction?";

	defs[40].Character = '\20';
	defs[40].Color = COLOR_CHOICE_ON_BLACK;
	defs[40].Cycle = 4;
	defs[40].EditorCategory = CATEGORY_CREATURE;
	defs[40].EditorShortcut = 'P';
	defs[40].Name = "Pusher";
	defs[40].ParamDirName = "Push direction?";

	defs[13].Character = '\13';
	defs[13].Pushable = true;
	defs[13].Cycle = 6;
	defs[13].EditorCategory = CATEGORY_ITEM;
	defs[13].EditorShortcut = 'B';
	defs[13].Name = "Bomb";

	defs[14].Character = '\177';
	defs[14].Color = 0x5;
	defs[14].EditorCategory = CATEGORY_ITEM;
	defs[14].EditorShortcut = 'E';
	defs[14].Name = "Energizer";

	defs[29].Character = '\316';
	defs[29].Cycle = 1;
	defs[29].EditorCategory = CATEGORY_TERRAIN;
	defs[29].EditorShortcut = 'L';
	defs[29].Name = "Blink wall";
	defs[29].Param1Name = "Starting time";
	defs[29].Param2Name = "Period";
	defs[29].ParamDirName = "Wall direction";

	defs[27].Character = '\262';
	defs[27].EditorCategory = CATEGORY_TERRAIN;
	defs[27].PlaceableOnTop = true;
	defs[27].Walkable = true;
	defs[27].EditorShortcut = 'A';
	defs[27].Name = "Fake";

	defs[28].Character = ' ';
	defs[28].EditorCategory = CATEGORY_TERRAIN;
	defs[28].EditorShortcut = 'I';
	defs[28].Name = "Invisible";

	defs[36].Character = '\2';
	defs[36].EditorCategory = CATEGORY_CREATURE;
	defs[36].Cycle = 3;
	defs[36].EditorShortcut = 'O';
	defs[36].Name = "Object";
	defs[36].Param1Name = "Character?";
	defs[36].ParamTextName = "Edit Program";
}