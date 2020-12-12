#pragma once

#include "ptoc.h"
#include "array.h"

#include <vector>
#include <string>

const integer MAX_STAT = 150;
const integer BOARD_WIDTH = 60;
const integer BOARD_HEIGHT = 25;
const integer MAX_BOARD_LEN = 20000;

class TTile {
	public:
        unsigned char Element;
        unsigned char Color;
        // TODO? Operator overloading
        void dump(std::vector<unsigned char> & out) const;
};

struct TRleTile {
        unsigned char Count;
        TTile Tile;
};

struct TStat {
	public:
        unsigned char X, Y;
        short StepX, StepY;
        short Cycle;
        unsigned char P1, P2, P3;
        short Follower;
        short Leader;
        TTile Under;
        array<0 , 3,unsigned char> unk1;
        short DataPos;
        short DataLen;
        unsigned char* Data;
        void dump(std::vector<unsigned char> & out) const;
};

struct TBoardInfo {
        unsigned char MaxShots;
        bool IsDark;
        array<0 , 3,unsigned char> NeighborBoards;
        bool ReenterWhenZapped;
        asciiz Message;
        unsigned char StartPlayerX;
        unsigned char StartPlayerY;
        short TimeLimitSec;
        array<70 , 85,unsigned char> unk1;
        void dump(std::vector<unsigned char> & out) const;
};

/* This is used to make sure IoTmpBuf is always large enough to hold
   what changes may happen to the board. In the worst case, the board
   is completely empty when loaded but takes max space due to stats,
   then during play, the board gets filled with random tiles. That
   will require MAX_RLE_OVERFLOW additional bytes to hold. So by
   dimensioning IoTmpBuf with an excess of MAX_RLE_OVERFLOW, we ensure
   that it can never run out of space from RLE shenanigans. */
const integer MAX_RLE_OVERFLOW = BOARD_WIDTH * BOARD_HEIGHT * sizeof(TRleTile);

class TBoard {
	public:
        std::string Name;
        matrix<0 , BOARD_WIDTH + 1,0 , BOARD_HEIGHT + 1,TTile> Tiles;
        short StatCount;
        array<0 , MAX_STAT + 1,TStat> Stats;
        TBoardInfo Info;

        void create();					// Construct a yellow border board.
        // TODO later: use proper size-containing classes, e.g. vectors?
        std::vector<unsigned char> dump();	// Serialize
        bool open(const std::vector<unsigned char> & source);	// Deserialize
};