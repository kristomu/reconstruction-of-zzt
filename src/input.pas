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

unit Input;

interface
	const
		KEY_BACKSPACE = #8;
		KEY_TAB = #9;
		KEY_ENTER = #13;
		KEY_CTRL_Y = #25;
		KEY_ESCAPE = #27;
		KEY_ALT_P = #153;
		KEY_F1 = #187;
		KEY_F2 = #188;
		KEY_F3 = #189;
		KEY_F4 = #190;
		KEY_F5 = #191;
		KEY_F6 = #192;
		KEY_F7 = #193;
		KEY_F8 = #194;
		KEY_F9 = #195;
		KEY_F10 = #196;
		KEY_UP = #200;
		KEY_PAGE_UP = #201;
		KEY_LEFT = #203;
		KEY_RIGHT = #205;
		KEY_DOWN = #208;
		KEY_PAGE_DOWN = #209;
		KEY_INSERT = #210;
		KEY_DELETE = #211;
		KEY_HOME = #212;
		KEY_END = #213;
		KEY_SHIFT_UP = #193;
		KEY_SHIFT_DOWN = #194;
		KEY_SHIFT_LEFT = #196;
		KEY_SHIFT_RIGHT = #195;

		{ Quick and dirty hack from video.pas }
		cp437ToUnicode: array[0 .. 255] of smallint = (
			$0020, $263A, $263B, $2665, $2666, $2663, $2660, $2022,
			$25D8, $25CB, $25D9, $2642, $2640, $266A, $266B, $263C,
			$25BA, $25C4, $2195, $203C, $00B6, $00A7, $25AC, $21A8,
			$2191, $2193, $2192, $2190, $221F, $2194, $25B2, $25BC,

			$0020, $0021, $0022, $0023, $0024, $0025, $0026, $0027,
			$0028, $0029, $002A, $002B, $002C, $002D, $002E, $002F,
			$0030, $0031, $0032, $0033, $0034, $0035, $0036, $0037,
			$0038, $0039, $003A, $003B, $003C, $003D, $003E, $003F,
			$0040, $0041, $0042, $0043, $0044, $0045, $0046, $0047,
			$0048, $0049, $004A, $004B, $004C, $004D, $004E, $004F,
			$0050, $0051, $0052, $0053, $0054, $0055, $0056, $0057,
			$0058, $0059, $005A, $005B, $005C, $005D, $005E, $005F,
			$0060, $0061, $0062, $0063, $0064, $0065, $0066, $0067,
			$0068, $0069, $006A, $006B, $006C, $006D, $006E, $006F,
			$0070, $0071, $0072, $0073, $0074, $0075, $0076, $0077,
			$0078, $0079, $007A, $007B, $007C, $007D, $007E, $2302,

			$00C7, $00FC, $00E9, $00E2, $00E4, $00E0, $00E5, $00E7,
			$00EA, $00EB, $00E8, $00EF, $00EE, $00EC, $00C4, $00C5,
			$00C9, $00E6, $00C6, $00F4, $00F6, $00F2, $00FB, $00F9,
			$00FF, $00D6, $00DC, $00A2, $00A3, $00A5, $20A7, $0192,
			$00E1, $00ED, $00F3, $00FA, $00F1, $00D1, $00AA, $00BA,
			$00BF, $2310, $00AC, $00BD, $00BC, $00A1, $00AB, $00BB,
			$2591, $2592, $2593, $2502, $2524, $2561, $2562, $2556,
			$2555, $2563, $2551, $2557, $255D, $255C, $255B, $2510,
			$2514, $2534, $252C, $251C, $2500, $253C, $255E, $255F,
			$255A, $2554, $2569, $2566, $2560, $2550, $256C, $2567,
			$2568, $2564, $2565, $2559, $2558, $2552, $2553, $256B,
			$256A, $2518, $250C, $2588, $2584, $258C, $2590, $2580,
			$03B1, $00DF, $0393, $03C0, $03A3, $03C3, $00B5, $03C4,
			$03A6, $0398, $03A9, $03B4, $221E, $03D5, $03B5, $2229,
			$2261, $00B1, $2265, $2264, $2320, $2321, $00F7, $2248,
			$00B0, $2219, $00B7, $221A, $207F, $00B2, $25A0, $0020);
	var
		InputDeltaX, InputDeltaY: integer;
		InputShiftPressed: boolean;
		InputShiftAccepted: boolean;
		InputJoystickEnabled: boolean;
		InputMouseEnabled: boolean;
		InputKeyPressed: char;
		InputMouseX, InputMouseY: integer;
		InputMouseActivationX, InputMouseActivationY: integer;
		InputMouseButtonX, InputMouseButtonY: integer;
		InputJoystickMoved: boolean;
		UnicodeToCP437: array[0 .. 65536] of char;
	procedure InputUpdate;
	procedure InputInitDevices;
	procedure InputReadWaitKey;
	function InputConfigure: boolean;

implementation
uses Dos, Crt, Sounds, Termio;

const
	PORT_JOYSTICK = $0201;
var
	JoystickXInitial, JoystickYInitial: integer;
	InputLastDeltaX, InputLastDeltaY: integer;
	JoystickXMin, JoystickXCenter, JoystickXMax: integer;
	JoystickYMin, JoystickYCenter, JoystickYMax: integer;
	InputKeyBuffer: string;

{ Everything joystick is right out in non-DOS. Ditto mouse. }

{ https://rosettacode.org/wiki/UTF-8_encode_and_decode#PureBasic }

function UTF8ToCodepoint(var utf: array of byte): longint;
	begin
		if utf[0] <= %01111111 then UTF8ToCodepoint := Ord(utf[0])
		else if utf[0] <= %11011111 then begin { 2 byte encoding }
			UTF8ToCodepoint :=
				((Ord(utf[0]) and %00011111) shl 6) or
				((Ord(utf[1]) and %00111111));
		end else if utf[0] <= %11101111 then begin {3 byte encoding}
			UTF8ToCodepoint :=
				((Ord(utf[0]) and %00001111) shl 12) or
				((Ord(utf[1]) and %00111111) shl 6) or
				((Ord(utf[2]) and %00111111));
		end else if utf[0] <= %11110111 then begin {4 byte encoding}
			UTF8ToCodepoint :=
				((Ord(utf[0]) and %00000111) shl 18) or
				((Ord(utf[1]) and %00111111) shl 12) or
				((Ord(utf[2]) and %00111111) shl 6) or
				((Ord(utf[3]) and %00111111));
		end else
			UTF8ToCodePoint := 0 {unknown}
	end;

procedure SetupCodepointToCP437;
	var
		i : integer;
	begin
		{ Everything that's not known maps to the character '?' }
		fillChar(UnicodeToCP437, 65536, '?');
		{ First reverse the cp437 to unicode codepoint table. }
		for i := 0 to 255 do
			UnicodeToCP437[cp437ToUnicode[i]] := Chr(i);

		{ Then add some "near-equivalents" }
		{lowercase ø: make it look like phi}
		UnicodeToCP437[248] := Chr(237);

		{uppercase Ø: make it look like Theta}
		UnicodeToCP437[216] := Chr(233);
	end;

function CodepointToCP437(cp: longint): char;
	begin
		if cp > 65535 then CodepointToCP437 := '?'
		else CodepointToCP437 := UnicodeToCP437[cp];
	end;

procedure InputUpdate;
	var
		utf: array[0..3] of Byte;	{ UTF8 storage array }
		i: Byte;					{ Counter for UTF8 recording }
	begin
		InputDeltaX := 0;
		InputDeltaY := 0;
		InputShiftPressed := false;
		InputJoystickMoved := false;
		{ The raw text mode that Crt places a Linux terminal into seems
		  to make it possible to fill the keypress queue faster than
		  ReadKey can dispose of it. So we can only read once and then
		  need to flush the whole queue afterwards. This will lead to
		  nonstandard behavior if objects hog all the processing power.}
		while KeyPressed do begin
			InputKeyPressed := ReadKey;
			utf[0] := Byte(InputKeyPressed);
			{ Special keys }
			if (InputKeyPressed = #0) or (InputKeyPressed = #1) or (InputKeyPressed = #2) then
				InputKeyBuffer := InputKeyBuffer + Chr(Ord(ReadKey) or $80)
			{ Key corresponding to multi-byte UTF8 }
			else if utf[0] > $7F then begin
				i := 1;
				while (i <= 3) and ByteBool((utf[0] shl i) and %10000000) do begin
					utf[i] := Byte(ReadKey);
					inc(i);
				end;

				InputKeyBuffer := InputKeyBuffer + CodepointToCP437(UTF8ToCodepoint(utf));

			end else { Ordinary key }
				InputKeyBuffer := InputKeyBuffer + InputKeyPressed;
			TCFlush(0, TCIFLUSH); { Flush the keybuffer. }
		end;
		if Length(InputKeyBuffer) <> 0 then begin
			InputKeyPressed := InputKeyBuffer[1];
			if Length(InputKeyBuffer) = 1 then
				InputKeyBuffer := ''
			else
				InputKeyBuffer := Copy(InputKeyBuffer, Length(InputKeyBuffer) - 1, 1);
			{ If the player pressed SHIFT+arrow, set
			  InputShiftPressed.}

			InputShiftPressed := False;

			case InputKeyPressed of
				KEY_SHIFT_UP, KEY_SHIFT_DOWN,
				KEY_SHIFT_LEFT, KEY_SHIFT_RIGHT: begin
					InputShiftPressed := True;
				end;
			end;

			case InputKeyPressed of
				KEY_UP, KEY_SHIFT_UP, '8': begin
					InputDeltaX := 0;
					InputDeltaY := -1;
				end;
				KEY_LEFT, KEY_SHIFT_LEFT, '4': begin
					InputDeltaX := -1;
					InputDeltaY := 0;
				end;
				KEY_RIGHT, KEY_SHIFT_RIGHT, '6': begin
					InputDeltaX := 1;
					InputDeltaY := 0;
				end;
				KEY_DOWN, KEY_SHIFT_DOWN, '2': begin
					InputDeltaX := 0;
					InputDeltaY := 1;
				end;
			end;
		end else begin
			InputKeyPressed := #0;
		end;

		if (InputDeltaX <> 0) or (InputDeltaY <> 0) then begin
			InputLastDeltaX := InputDeltaX;
			InputLastDeltaY := InputDeltaY;
		end;
	end;

procedure InputInitDevices;
	begin
		InputJoystickEnabled := False;
		InputMouseEnabled := False;
	end;

function InputConfigure: boolean;
	var
		charTyped: char;
	begin
		charTyped := ' ';
		if InputJoystickEnabled or InputMouseEnabled then begin
			Writeln;
			Write('  Game controller:  K)eyboard');
			if InputJoystickEnabled then
				Write(',  J)oystick');
			if InputMouseEnabled then
				Write(',  M)ouse');
			Write('?  ');

			repeat
				repeat until KeyPressed;
				charTyped := UpCase(ReadKey);
			until (charTyped = 'K')
				or (charTyped = #27);
			Writeln;

			InputJoystickEnabled := false;
			InputMouseEnabled := false;
			Writeln;
		end;
		InputConfigure := charTyped <> #27;
	end;

procedure InputReadWaitKey;
	begin
		repeat
			InputUpdate;
			{ Don't busy-wait too much. }
			Delay(10);
		until InputKeyPressed <> #0;
	end;

begin
	InputLastDeltaX := 0;
	InputLastDeltaY := 0;
	InputDeltaX := 0;
	InputDeltaY := 0;
	InputShiftPressed := false;
	InputShiftAccepted := false;
	InputMouseX := 0;
	InputMouseY := 0;
	InputMouseActivationX := 60;
	InputMouseActivationY := 60;
	InputMouseButtonX := 0;
	InputMouseButtonY := 0;
	InputKeyBuffer := '';
	SetupCodepointToCP437;
end.
