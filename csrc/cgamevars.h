#pragma once

#include <array>
#include <string>

// Game variables that don't need Pascal-to-C code.

const std::array<std::string, 8> ColorNames =
{"Black", "Blue", "Green", "Cyan", "Red", "Purple", "Yellow", "White"};

/**/
const short CATEGORY_ITEM = 1;
const short CATEGORY_CREATURE = 2;
const short CATEGORY_TERRAIN = 3;
/**/
const short COLOR_SPECIAL_MIN = 0xf0;
const short COLOR_CHOICE_ON_BLACK = 0xff;
const short COLOR_WHITE_ON_CHOICE = 0xfe;
const short COLOR_CHOICE_ON_CHOICE = 0xfd;
/**/
const short SHOT_SOURCE_PLAYER = 0;
const short SHOT_SOURCE_ENEMY = 1;