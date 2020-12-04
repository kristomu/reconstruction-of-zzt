#include "ptoc.h"

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

/*$I-*/

#include "hardware.h"
#include <unistd.h>

/*#include "Crt.h"*/
#include "gamevars.h"
//#include "sounds.h"
#include "fileops.h"
//#include "input.h"
#include "video.h"
/*#include "dos.h"
#include "txtwind.h"
#include "elements.h"
#include "editor.h"
#include "oop.h"
#include "game.h"
*/


void ParseArguments() {
    integer i;
    string pArg;

    for( i = 1; i <= ParamCount; i ++) {
        pArg = ParamStr(i);
        if (pArg[1] == '/')  {
            switch (upcase(pArg[2])) {
            case 'T': {
                // TBD: sounds.pas
                /* SoundTimeCheckCounter = 0;
                UseSystemTimeForElapsed = false;*/
            }
            break;
            case 'R': ResetConfig = true; break;
            }
        } else {
            StartupWorldFileName = pArg;
            if ((length(StartupWorldFileName) > 4)
                    && (StartupWorldFileName[length(StartupWorldFileName) - 3] == '.'))  {
                StartupWorldFileName = copy(StartupWorldFileName, 1,
                                            length(StartupWorldFileName) - 4);
            }
        }
    }
}

void GameConfigure() {
    integer unk1;
    boolean joystickEnabled, mouseEnabled;
    text cfgFile;
    integer bottomRow;

    ParsingConfigFile = true;
    EditorEnabled = true;
    ConfigRegistration = "";
    ConfigWorldFile = "";
    GameVersion = "3.2";

    assign(cfgFile, "zzt.cfg");
    OpenForRead(cfgFile);
    if (ioResult == 0)  {
        cfgFile >> ConfigWorldFile >> NL;
        cfgFile >> ConfigRegistration >> NL;
    }
    if (ConfigWorldFile[1] == '*')  {
        EditorEnabled = false;
        ConfigWorldFile = copy(ConfigWorldFile, 2, length(ConfigWorldFile) - 1);
    }
    if (length(ConfigWorldFile) != 0)  {
        StartupWorldFileName = ConfigWorldFile;
    }

    // TBD: inputs.pas
    /*InputInitDevices();
    joystickEnabled = InputJoystickEnabled;
    mouseEnabled = InputMouseEnabled;*/

    /* Define the bottom row of the 80x25 terminal layout, or
    the bottom of the screen if it's smaller. */
    bottomRow = WindMaxY - WindMinY;
    if (bottomRow > 25)  bottomRow = 25;

    ParsingConfigFile = false;

    Window(1, 1, 80, bottomRow+1);
    initCurses();
    TextBackground(Black);
    ClrScr();
    TextColor(White);
    TextColor(White);
    cursesWriteLn("");
    cursesWriteLn("                                 <=-  ZZT  -=>");
    TextColor(Yellow);
    if (length(ConfigRegistration) == 0)
        cursesWriteLn("                             Shareware version 3.2");
    else
        cursesWriteLn("                                  Version  3.2");
    cursesWriteLn("                            Created by Tim Sweeney");
    TextColor(LightGray);
    if (bottomRow < 24)
        cursesWriteLn("                        Best played in 80x25 or larger.");
    GotoXY(1, 7);
    TextColor(Blue);
    cursesWriteLn("================================================================================");
    GotoXY(1, bottomRow);
    cursesWriteLn("================================================================================");
    TextColor(White);
    GotoXY(30, 7);
    cursesWrite(" Game Configuration ");
    GotoXY(1, bottomRow+1);
    cursesWriteLn(" Copyright (c) 1991 Epic MegaGames                         Press ... to abort");
    TextColor(Black);
    TextBackground(LightGray);
    GotoXY(66, bottomRow+1);
    cursesWrite("ESC");
    getch(); // hax
    /*Window(1, 8, 80, bottomRow-2);
    TextColor(Yellow);
    TextBackground(Black);
    ClrScr();
    TextColor(Yellow);*/
    // TBD: video.pas
    GotoXY(1, 8);
    TextColor(LightGreen);
    TextBackground(Black);
    if (! VideoConfigure())
        GameTitleExitRequested = true;

    Window(1, 1, 80, bottomRow+1);
}

int main(int argc, const char* argv[]) {
    pio_initialize(argc, argv);
    WorldFileDescCount = 7;
    WorldFileDescKeys[1] = "TOWN";
    WorldFileDescValues[1] = "TOWN       The Town of ZZT";
    WorldFileDescKeys[2] = "DEMO";
    WorldFileDescValues[2] = "DEMO       Demo of the ZZT World Editor";
    WorldFileDescKeys[3] = "CAVES";
    WorldFileDescValues[3] = "CAVES      The Caves of ZZT";
    WorldFileDescKeys[4] = "DUNGEONS";
    WorldFileDescValues[4] = "DUNGEONS   The Dungeons of ZZT";
    WorldFileDescKeys[5] = "CITY";
    WorldFileDescValues[5] = "CITY       Underground City of ZZT";
    WorldFileDescKeys[6] = "BEST";
    WorldFileDescValues[6] = "BEST       The Best of ZZT";
    WorldFileDescKeys[7] = "TOUR";
    WorldFileDescValues[7] = "TOUR       Guided Tour ZZT\47s Other Worlds";

    Randomize();
    SetCBreak(false);
    InitialTextAttr = TextAttr;

    StartupWorldFileName = "TOWN";
    //ResourceDataFileName = "ZZT.DAT";
    ResetConfig = false;
    GameTitleExitRequested = false;
    GameConfigure();
    ParseArguments();

    /*if (! GameTitleExitRequested)  {
            VideoInstall(80, Blue);
            OrderPrintId = &GameVersion;
            TextWindowInit(5, 3, 50, 18);
            IoTmpBuf = new TIoTmpBuf;

            VideoHideCursor();
            ClrScr;

            TickSpeed = 4;
            DebugEnabled = false;
            SavedGameFileName = "SAVED";
            SavedBoardFileName = "TEMP";
            GenerateTransitionTable();
            WorldCreate();

            GameTitleLoop();

            //LEAKFIX: Remember to dispose of *everything* in use.
            WorldUnload();
            delete IoTmpBuf;
    }

    SoundUninstall();
    SoundClearQueue();

    VideoUninstall();
    TextAttr = InitialTextAttr;
    ClrScr;

    if (length(ConfigRegistration) == 0)  {
            GamePrintRegisterMessage();
    } else {
            output << NL;
            output << "  Registered version -- Thank you for playing ZZT." << NL;
            output << NL;
    }

    VideoShowCursor();
    return EXIT_SUCCESS;*/
    uninitCurses();
}
