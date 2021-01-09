{
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
}

{$I-}
{$mode objfpc}{$H+}
library testzzt;
uses Crt, Dos, Video, Sounds, Input, TxtWind, GameVars, Elements, Editor, Oop, Game, Fileops, BaseUnix, Fuzz, ctypes;

var
	FailFlag : boolean; cvar;

procedure ParseArguments;
	var
		i: integer;
		pArg: string;
	begin
		for i := 1 to ParamCount do begin
			pArg := ParamStr(i);
			if pArg[1] = '/' then begin
				case UpCase(pArg[2]) of
					'T': begin
						SoundTimeCheckCounter := 0;
						UseSystemTimeForElapsed := false;
					end;
					'R': ResetConfig := true;
				end;
			end else begin
				StartupWorldFileName := pArg;
				if (Length(StartupWorldFileName) > 4) and (StartupWorldFileName[Length(StartupWorldFileName) - 3] = '.') then begin
					StartupWorldFileName := Copy(StartupWorldFileName, 1, Length(StartupWorldFileName) - 4);
				end;
			end;
		end;
	end;

procedure GameConfigure;
	var
		unk1: integer;
		joystickEnabled, mouseEnabled: boolean;
		cfgFile: text;
		bottomRow: integer;
	begin
		ParsingConfigFile := true;
		EditorEnabled := true;
		ConfigRegistration := '';
		ConfigWorldFile := '';
		GameVersion := '3.2';

		Assign(cfgFile, 'zzt.cfg');
		OpenForRead(cfgFile);
		if IOResult = 0 then begin
			Readln(cfgFile, ConfigWorldFile);
			Readln(cfgFile, ConfigRegistration);
		end;
		if ConfigWorldFile[1] = '*' then begin
			EditorEnabled := false;
			ConfigWorldFile := Copy(ConfigWorldFile, 2, Length(ConfigWorldFile) - 1);
		end;
		if Length(ConfigWorldFile) <> 0 then begin
			StartupWorldFileName := ConfigWorldFile;
		end;

		InputInitDevices;
		joystickEnabled := InputJoystickEnabled;
		mouseEnabled := InputMouseEnabled;

		{ Define the bottom row of the 80x25 terminal layout, or
		  the bottom of the screen if it's smaller. }
		bottomRow := WindMaxY - WindMinY;
		if bottomRow > 25 then bottomRow := 25;

		ParsingConfigFile := false;

		Window(1, 1, 80, bottomRow+1);
		TextBackground(Black);
		ClrScr;
		TextColor(White);
		TextColor(White);
		Writeln;
		Writeln('                                 <=-  ZZT  -=>');
		TextColor(Yellow);
		if Length(ConfigRegistration) = 0 then
			Writeln('                             Shareware version 3.2')
		else
			Writeln('                                  Version  3.2');
		Writeln('                            Created by Tim Sweeney');
		TextColor(LightGray);
		if bottomRow < 24 then Writeln('                        Best played in 80x25 or larger.');
		GotoXY(1, 7);
		TextColor(Blue);
		Write('================================================================================');
		GotoXY(1, bottomRow);
		Write('================================================================================');
		TextColor(White);
		GotoXY(30, 7);
		Write(' Game Configuration ');
		GotoXY(1, bottomRow+1);
		Write(' Copyright (c) 1991 Epic MegaGames                         Press ... to abort');
		TextColor(Black);
		TextBackground(LightGray);
		GotoXY(66, bottomRow+1);
		Write('ESC');
		Window(1, 8, 80, bottomRow-2);
		TextColor(Yellow);
		TextBackground(Black);
		ClrScr;
		TextColor(Yellow);
		if not InputConfigure then
			GameTitleExitRequested := true
		else begin
			TextColor(LightGreen);
			if not VideoConfigure then
				GameTitleExitRequested := true;
		end;
		Window(1, 1, 80, bottomRow+1);
	end;

function SetFailFlag(Addr: CodePointer) : ShortString;
	begin
		FailFlag := true;
		SetFailFlag := '';
	end;

procedure EvolveZZT(worldInput: PChar; worldInputLen: longint);
	var
		i: integer;
	begin
		WorldFileDescCount := 0;
		FailFlag := false;

		{ Make the backtrace set the fail flag. }
		Pointer(BackTraceStrFunc) := @SetFailFlag;

		{ Set the same random seed every time in case the crash depends on it. }
		Randomize;
		RandSeed := 0;

		TFuzzMode := true;
		VFuzzMode := true;
		SFuzzMode := true;
		FuzzMode := true;

		if not VFuzzMode then begin
			SetCBreak(false);
			InitialTextAttr := TextAttr;
		end;

		ResourceDataFileName := 'ZZT.DAT';
		ResetConfig := false;
		GameTitleExitRequested := false;

		if not VFuzzMode then begin
			VideoInstall(80, Blue);
			VideoHideCursor;
			ClrScr;
		end;


		TextWindowInit(5, 3, 50, 18);

		New(IoTmpBuf);

		TickSpeed := 4;
		DebugEnabled := false;
		SavedGameFileName := 'SAVED';
		SavedBoardFileName := 'TEMP';
		GenerateTransitionTable;
		WorldCreate;

		GameRunFewCycles(20, worldInput, worldInputLen, false);

		{ Go through every board to check that they can be loaded. }
		for i := 0 to World.BoardCount do begin
			EditorLoop;
			BoardChange(i);
		end;

		{LEAKFIX: Remember to dispose of *everything* in use. }
		{There's a leak here somewhere. Find out how to deal with it.}
		{WorldUnload;}
		Dispose(IoTmpBuf);

		if not SFuzzMode then begin
			SoundUninstall;
			SoundClearQueue;
		end;

		if not VFuzzMode then begin
			VideoUninstall;
			TextAttr := InitialTextAttr;
			ClrScr;
			VideoShowCursor;
		end;
	end;
exports
	EvolveZZT, FailFlag;
end.