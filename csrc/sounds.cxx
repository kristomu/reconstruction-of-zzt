/*
	Copyright (c) 2020 Adrian Siekierka

	Based on a reconstruction of code from ZZT,
	Copyright 1991 Epic MegaGames, used with permission.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

// Doesn't work yet on Linux -- will probably need a fresh implementation
// that uses SDL or something to synthesize PC beeps.

// If I don't end up using samples, the drum tables etc. should all be
// determinized so they're completely reproducible. There's no reason for
// the sound renderer to need randomness when it's dithering the same
// sound all the time.

#define __Sounds_implementation__

#include "ptoc.h"
#include "testing.h"
#include "sounds.h"

/*#include "Crt.h"*/
/*#include "Dos.h"*/
#include "minmax.h"
#include "hardware.h"
#include "gamevars.h"


void SoundQueue(integer priority, string pattern) {
	if (! SoundBlockQueueing &&
		(! SoundIsPlaying || (((priority >= SoundCurrentPriority)
					&& (SoundCurrentPriority != -1)) || (priority == -1)))) {
		if ((priority >= 0) || ! SoundIsPlaying)  {
			SoundCurrentPriority = priority;
			SoundBuffer = pattern;
			SoundBufferPos = 1;
			SoundDurationCounter = 1;
		} else {
			SoundBuffer = copy(SoundBuffer, SoundBufferPos,
					length(SoundBuffer) - SoundBufferPos + 1);
			SoundBufferPos = 1;
			if ((length(SoundBuffer) + length(pattern)) < 255)  {
				SoundBuffer = SoundBuffer + pattern;
			}
		}
		SoundIsPlaying = true;
	}
}

void SoundClearQueue() {
	SoundBuffer = "";
	SoundIsPlaying = false;
	/*NoSound();*/
}

void SoundInitFreqTable() {
	integer octave, note;
	real freqC1, noteStep, noteBase, ln2;

	freqC1 = 32.0;
	ln2 = log(2.0);
	noteStep = exp(ln2 / 12.0);
	for (octave = 1; octave <= 15; octave ++) {
		noteBase = exp(octave * ln2) * freqC1;
		for (note = 0; note <= 11; note ++) {
			/* IMP: Fix integer overflow */
			SoundFreqTable[octave * 16 + note] = Min(65535, trunc(noteBase));
			noteBase = noteBase * noteStep;
		}
	}
}

void SoundInitDrumTable() {
	integer i;

	SoundDrumTable[0].Len = 1;
	SoundDrumTable[0].Data[1] = 3200;
	for (i = 1; i <= 9; i ++) {
		SoundDrumTable[i].Len = 14;
	}
	for (i = 1; i <= 14; i ++) {
		SoundDrumTable[1].Data[i] = i * 100 + 1000;
	}
	for (i = 1; i <= 16; i ++) {
		SoundDrumTable[2].Data[i] = (i % 2) * 1600 + 1600 + (i % 4) * 1600;
	}
	for (i = 1; i <= 14; i ++) {
		SoundDrumTable[4].Data[i] = rnd.randint(5000) + 500;
	}
	for (i = 1; i <= 8; i ++) {
		SoundDrumTable[5].Data[i * 2 - 1] = 1600;
		SoundDrumTable[5].Data[i * 2] = rnd.randint(1600) + 800;
	}
	for (i = 1; i <= 14; i ++) {
		SoundDrumTable[6].Data[i] = ((i % 2) * 880) + 880 + ((i % 3) * 440);
	}
	for (i = 1; i <= 14; i ++) {
		SoundDrumTable[7].Data[i] = 700 - (i * 12);
	}
	for (i = 1; i <= 14; i ++) {
		SoundDrumTable[8].Data[i] = (i * 20 + 1200) - rnd.randint(i * 40);
	}
	for (i = 1; i <= 14; i ++) {
		SoundDrumTable[9].Data[i] = rnd.randint(440) + 220;
	}
}

void SoundPlayDrum(TDrumData & drum) {
	integer i;

	for (i = 1; i <= drum.Len; i ++) {
		Sound(drum.Data[i]);
		Delay(1);
	}
	NoSound();
}

void SoundCheckTimeIntr() {
	short hour, minute, sec, hSec;

	GetTime(hour, minute, sec, hSec);
	if ((SoundTimeCheckHsec != 0)
		&& ((integer)(hSec) != SoundTimeCheckHsec))  {
		SoundTimeCheckCounter = 0;
		UseSystemTimeForElapsed = true;
	}
	SoundTimeCheckHsec = (integer)(hSec);
}

/* This procedure has to be changed to take into account VERY FAST computers.
  Hopefully in a way that still works in DOS.
  Maybe a phase locked loop-ish solution where the procedure sleeps a certain
  number of milliseconds if the elapsed time since last check is below say,
  half a tick. Or just delaying until the end of the tick period if we get here
  early.
  Currently just kludges this by using system time, which is slow. */

boolean SoundHasTimeElapsed(integer & counter, integer duration) {
	short hour, minute, sec, hSec;
	word hSecsDiff;
	integer hSecsTotal;

	if (test_mode_disable_delay) {
		return true;
	}

	boolean SoundHasTimeElapsed_result;
	if ((SoundTimeCheckCounter > 0) && ((SoundTimeCheckCounter % 2) == 1))  {
		SoundTimeCheckCounter = SoundTimeCheckCounter - 1;
		SoundCheckTimeIntr();
	}

	if (UseSystemTimeForElapsed)  {
		GetTime(hour, minute, sec, hSec);
		hSecsTotal = sec * 100 + hSec;
		hSecsDiff = (word)((hSecsTotal - counter) + 6000) % 6000;
	} else {
		hSecsTotal = TimerTicks * 6;
		hSecsDiff = hSecsTotal - counter;
	}

	if (hSecsDiff >= duration)  {
		SoundHasTimeElapsed_result = true;
		counter = hSecsTotal;
	} else {
		SoundHasTimeElapsed_result = false;
		/* Duration seems to be in units of 100th seconds,
		possibly influenced by the game speed slider.
		Perhaps some kind of tick thread would be better,
		because then querying for time would take no time. */
		Delay(20);
	}
	return SoundHasTimeElapsed_result;
}

void SoundTimerHandler() { /*interrupt;*/
	TimerTicks += 1;
	if ((SoundTimeCheckCounter > 0) && ((SoundTimeCheckCounter % 2) == 0))  {
		SoundTimeCheckCounter = SoundTimeCheckCounter - 1;
	}

	if (! SoundEnabled)  {
		SoundIsPlaying = false;
		NoSound();
	} else if (SoundIsPlaying)  {
		SoundDurationCounter -= 1;
		if (SoundDurationCounter <= 0)  {
			NoSound();
			if (SoundBufferPos >= length(SoundBuffer))  {
				NoSound();
				SoundIsPlaying = false;
			} else {
				if (SoundBuffer[SoundBufferPos] == '\0') {
					NoSound();
				} else if (SoundBuffer[SoundBufferPos] < '\360') {
					Sound(SoundFreqTable[ord(SoundBuffer[SoundBufferPos])]);
				} else {
					SoundPlayDrum(SoundDrumTable[ord(SoundBuffer[SoundBufferPos]) - 240]);
				}
				SoundBufferPos += 1;

				SoundDurationCounter = SoundDurationMultiplier * ord(
						SoundBuffer[SoundBufferPos]);
				SoundBufferPos += 1;
			}
		}
	}
}


void SoundUninstall() {
	/*SetIntVec($1C, SoundOldVector);*/
}

string SoundParse(string input);

static void AdvanceInput(string & input) {
	input = copy(input, 2, length(input) - 1);
}

string SoundParse(string input) {
	integer noteOctave;
	integer noteDuration;
	string output;
	integer noteTone;

	string SoundParse_result;
	output = "";
	noteOctave = 3;
	noteDuration = 1;

	while (length(input) != 0)  {
		noteTone = -1;
		switch (upcase(input[1])) {
			case 'T': {
				noteDuration = 1;
				AdvanceInput(input);
			}
			break;
			case 'S': {
				noteDuration = 2;
				AdvanceInput(input);
			}
			break;
			case 'I': {
				noteDuration = 4;
				AdvanceInput(input);
			}
			break;
			case 'Q': {
				noteDuration = 8;
				AdvanceInput(input);
			}
			break;
			case 'H': {
				noteDuration = 16;
				AdvanceInput(input);
			}
			break;
			case 'W': {
				noteDuration = 32;
				AdvanceInput(input);
			}
			break;
			case '.': {
				noteDuration = (noteDuration * 3) / 2;
				AdvanceInput(input);
			}
			break;
			case '3': {
				noteDuration = noteDuration / 3;
				AdvanceInput(input);
			}
			break;
			case '+': {
				if (noteOctave < 6) {
					noteOctave = noteOctave + 1;
				}
				AdvanceInput(input);
			}
			break;
			case '-': {
				if (noteOctave > 1) {
					noteOctave = noteOctave - 1;
				}
				AdvanceInput(input);
			}
			break;
			case RANGE_7('A','G'): {
				switch (upcase(input[1])) {
					case 'C': {
						noteTone = 0;
						AdvanceInput(input);
					}
					break;
					case 'D': {
						noteTone = 2;
						AdvanceInput(input);
					}
					break;
					case 'E': {
						noteTone = 4;
						AdvanceInput(input);
					}
					break;
					case 'F': {
						noteTone = 5;
						AdvanceInput(input);
					}
					break;
					case 'G': {
						noteTone = 7;
						AdvanceInput(input);
					}
					break;
					case 'A': {
						noteTone = 9;
						AdvanceInput(input);
					}
					break;
					case 'B': {
						noteTone = 11;
						AdvanceInput(input);
					}
					break;
				}

				switch (upcase(input[1])) {
					case '!': {
						noteTone = noteTone - 1;
						AdvanceInput(input);
					}
					break;
					case '#': {
						noteTone = noteTone + 1;
						AdvanceInput(input);
					}
					break;
				}

				output = output + chr(noteOctave * 0x10 + noteTone) + chr(noteDuration);
			}
			break;
			case 'X': {
				output = output + '\0' + chr(noteDuration);
				AdvanceInput(input);
			}
			break;
			case RANGE_3('0','2'): case RANGE_6('4','9'): {
				output = output + chr(ord(input[1]) + 0xf0 - ord('0')) + chr(noteDuration);
				AdvanceInput(input);
			}
			break;
			default: AdvanceInput(input);
		}
	}
	SoundParse_result = output;
	return SoundParse_result;
}

class unit_Sounds_initialize {
	public: unit_Sounds_initialize();
};
static unit_Sounds_initialize Sounds_constructor;

unit_Sounds_initialize::unit_Sounds_initialize() {
	SoundInitFreqTable();
	SoundInitDrumTable();
	SoundTimeCheckCounter = 36;
	UseSystemTimeForElapsed = true;
	TimerTicks = 0;
	SoundTimeCheckHsec = 0;
	SoundEnabled = false;
	SoundBlockQueueing = false;
	SoundClearQueue();
	SoundDurationMultiplier = 1;
	SoundIsPlaying = false;
	TimerTicks = 0;
	/* Disabled in non-DOS. Fix later.*/
	//SoundNewVector = &SoundTimerHandler;
	/*GetIntVec($1C, SoundOldVector);
	SetIntVec($1C, SoundNewVector);*/
}
