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
		KEY_HOME = #199;
		KEY_END = #207;
		KEY_SHIFT_UP = #193;
		KEY_SHIFT_DOWN = #194;
		KEY_SHIFT_LEFT = #196;
		KEY_SHIFT_RIGHT = #195;

	var
		InputDeltaX, InputDeltaY: integer;
		InputShiftPressed: boolean;
		InputShiftAccepted: boolean;
		InputJoystickEnabled: boolean;
		InputMouseEnabled: boolean;
		InputSpecialKeyPressed: boolean;
		InputKeyPressed: char;
		InputMouseX, InputMouseY: integer;
		InputMouseActivationX, InputMouseActivationY: integer;
		InputMouseButtonX, InputMouseButtonY: integer;
		InputJoystickMoved: boolean;
		FuzzMode: boolean;
	procedure InputUpdate;
	procedure InputInitDevices;
	procedure InputReadWaitKey;
	function InputConfigure: boolean;

implementation
uses Dos, Crt, Sounds, Termio, Unicode;

const
	PORT_JOYSTICK = $0201;
var
	JoystickXInitial, JoystickYInitial: integer;
	InputLastDeltaX, InputLastDeltaY: integer;
	JoystickXMin, JoystickXCenter, JoystickXMax: integer;
	JoystickYMin, JoystickYCenter, JoystickYMax: integer;
	InputKeyBuffer: string;

{ Everything joystick is right out in non-DOS. Ditto mouse. }

{ UTF-8 input part from https://bit.ly/39nWLu5 p. 22 }
procedure InputUpdate;
	var
		utf: array[0..3] of Byte;	{ UTF8 storage array }
		i: Byte;					{ Counter for UTF8 recording }
	begin
		if FuzzMode then begin
			InputKeyPressed := KEY_ESCAPE;
			InputKeyBuffer := KEY_ESCAPE;
			InputSpecialKeyPressed := true;
			Exit;
		end;
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
			if (InputKeyPressed = #0) or (InputKeyPressed = #1) or (InputKeyPressed = #2) then begin
				InputKeyBuffer := InputKeyBuffer + Chr(Ord(ReadKey) or $80);
				InputSpecialKeyPressed := True;
			{ Key corresponding to multi-byte UTF8 }
			end else if utf[0] > $7F then begin
				i := 1;
				while (i <= 3) and ByteBool((utf[0] shl i) and %10000000) do begin
					utf[i] := Byte(ReadKey);
					inc(i);
				end;

				InputKeyBuffer := InputKeyBuffer + CodepointToCP437(UTF8ToCodepoint(utf));
				InputSpecialKeyPressed := False;
			end else begin { Ordinary key, or... }
				InputSpecialKeyPressed := False;
				{ Ugly hack for Linux console and the end key. For
				  some reason it outputs F[F[. }
				if (InputKeyPressed = #70) and KeyPressed then begin
					InputKeyPressed := ReadKey;
					if InputKeyPressed = '[' then begin
						{ Get rid of the second F[. }
						if KeyPressed then ReadKey;
						if KeyPressed then ReadKey;

						InputKeyPressed := KEY_END;
						InputSpecialKeyPressed := True;
					end else InputKeyPressed := #70;
				end;
				InputKeyBuffer := InputKeyBuffer + InputKeyPressed;
			end;
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
		if FuzzMode then begin
			InputKeyPressed := 'Y';
			Exit;
		end;

		repeat
			InputUpdate;
			{ Don't busy-wait too much. }
			Delay(10);
		until InputKeyPressed <> #0;
	end;

begin
	FuzzMode := false;
	InputLastDeltaX := 0;
	InputLastDeltaY := 0;
	InputDeltaX := 0;
	InputDeltaY := 0;
	InputShiftPressed := false;
	InputShiftAccepted := false;
	InputSpecialKeyPressed := false;
	InputMouseX := 0;
	InputMouseY := 0;
	InputMouseActivationX := 60;
	InputMouseActivationY := 60;
	InputMouseButtonX := 0;
	InputMouseButtonY := 0;
	InputKeyBuffer := '';
	SetupCodepointToCP437;
end.
