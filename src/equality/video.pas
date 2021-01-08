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
unit Video;

interface
	type
		TVideoLine = string[80];
		TTextChar = record
			Char: Char;
			Color: byte;
		end;

		TVideoBuffer = array[1..80, 1..25] of TTextChar;
	var
		VFuzzMode: boolean;
		VideoMonochrome: boolean;
		MainBuffer: TVideoBuffer;

	function VideoConfigure: boolean;
	procedure VideoWriteText(x, y, color: byte; text: TVideoLine);
	procedure VideoToggleEGAMode(EGA: Boolean);
	procedure VideoInstall(columns, borderColor: integer);
	procedure VideoUninstall;
	procedure VideoShowCursor;
	procedure VideoHideCursor;
	procedure VideoSetBorderColor(value: integer);
	procedure VideoCopy(xfrom, yfrom, width, height: integer;
		var buf: TVideoBuffer; toVideo: boolean);

implementation
uses Crt, Dos, Unicode, Fuzz;
var
	VideoColumns: integer;
	VideoBorderColor: integer;
	VideoTextSegment: word;
	VideoTextPointer: pointer;
	VideoCursorVisible: boolean;

procedure WriteUnicodeAsUTF8(codepoint: smallint);
	begin
		if codepoint < $80 then
			Write(Chr(codepoint))
		else if codepoint < $0800 then begin
			Write(Chr(((codepoint shr  6) and $1F) or $C0));
			Write(Chr(((codepoint shr  0) and $3F) or $80));
		end else begin { codepoint < $10000 }
			Write(Chr(((codepoint shr 12) and $0F) or $E0));
			Write(Chr(((codepoint shr  6) and $3F) or $80));
			Write(Chr(((codepoint shr  0) and $3F) or $80));
		end;
	end;

{$F+}

{ The input x,y values are offset by 0, i.e. 0,0 is upper left. }
procedure VideoWriteTextAsUTF8(x, y, color: byte; text: TVideoLine);
	var
		offset: Integer = 0;
		C: Char;

		terminalWidth: Integer;
		terminalHeight: Integer;
		charPseudoEnd: Integer;

	begin
		{Get the terminal height and width to avoid printing
		 outside it.
		 https://stackoverflow.com/questions/26776980 }

		terminalWidth := WindMaxX - WindMinX + 1;
		terminalHeight := WindMaxY - WindMinY + 1;

		if y >= terminalHeight then Exit;

		if color > $7F then
			TextColor(color and $F + Blink)
		else
			TextColor(color and $F);
		TextBackground(color shr 4);

		{ Hack from https://stackoverflow.com/a/35140822
		  A better solution will have to move away from Crt altogether.}

		{ Possible performance improvement: extract the contents of
		  this loop to a separate procedure. }
		for C in text do begin
			if x+offset >= terminalWidth then Exit;

			{ The performance enhancement below doesn't work due to
			  a strange bug I can't be arsed to track down. Somehow
			  MainBuffer is going out of sync. TODO, fix }

			{
			if (MainBuffer[x+offset+1][y+1].Color = color) and
			   (MainBuffer[x+offset+1][y+1].Char = C) then begin
				offset := offset + 1;
				Continue;
			end;
			}

			{ Since Crt believes we're outputting ASCII, it'll
			  "helpfully" scroll the terminal if we output multi-
			  char unicode while at the very lower right. There's
			  no way to avoid this misfeature, so just don't print
			  it in that case. }
			charPseudoEnd := x+offset+UTF8Len(CP437ToCodepoint(ord(C)));
			if (y = terminalHeight-1) and (charPseudoEnd >= terminalWidth)
				then Continue;

			GotoXY(1, 1);
			GotoXY(x+offset+1, y+1);
			MainBuffer[x+offset+1][y+1].Color := color;
			MainBuffer[x+offset+1][y+1].Char := C;

			{ For the same reason, it'll corrupt every "too-wide" utf8
			  point, so if we have any of those, just print a black-on-grey
			  question mark. }
			if (charPseudoEnd > terminalWidth) then begin
				TextColor($0);
				TextBackground($7);
				Write('?');
			end else
				WriteUnicodeAsUTF8(CP437ToCodepoint(ord(C)));

			offset := offset + 1;
		end;
		{ Move the cursor out of the way of the playing field. }
		if (not VideoCursorVisible) and (terminalWidth > 61) then
			GotoXY(61, 1);
	end;

procedure VideoWriteTextColor(x, y, color: byte; text: TVideoLine);
	begin
		VideoWriteTextAsUTF8(x, y, color, text);
	end;

procedure VideoWriteTextBW(x, y, color: byte; text: TVideoLine);
	begin
		if (color and $08) = $08 then begin
			if (color and $F0) = 0 then
				color := $0F
			else
				color := $7F;
		end else begin
			if (color and $07) <> 0 then
				color := $07
			else
				color := $70;
		end;
		VideoWriteTextAsUTF8(x, y, color, text);
	end;

{$F-}

procedure VideoWriteText(x, y, color: byte; text: TVideoLine);
	begin
		if VFuzzMode then Exit;
		if VideoMonochrome then
			VideoWriteTextBW(x, y, color, text)
		else
			VideoWriteTextColor(x, y, color, text);
	end;

{ Does nothing in Linux. The point in DOS is to change the charset from 9x16 to
8x14. It will have to be done in some other way in Linux. I'm keeping the empty
function as reminder to myself. TODO }
procedure VideoToggleEGAMode(EGA: Boolean);
	begin
	end;

function VideoConfigure: boolean;
	var
		charTyped: Char;
	begin
		charTyped := ' ';
		if LastMode = 7 then begin
			VideoMonochrome := true;
		end else begin
			Writeln;
			Write('  Video mode:  C)olor,  M)onochrome?  ');
			repeat
				repeat
					{ Don't busy-wait too much. }
					Wait(100);
				until KeyPressed;
				charTyped := UpCase(ReadKey);
			until charTyped in [#27, 'C', 'M'];
			case charTyped of
				'C': VideoMonochrome := false;
				'M': VideoMonochrome := true;
				#27: VideoMonochrome := (LastMode = 7);
			end;
		end;
		VideoConfigure := charTyped <> #27;
	end;

procedure VideoInstall(columns, borderColor: integer);
	begin
		VideoToggleEGAMode(True);

		if not VideoMonochrome then
			TextBackground(borderColor);

		VideoColumns := columns;
		if VideoMonochrome then begin
			if LastMode in [0, 1, 2, 3] then begin
				if columns = 80 then begin
					TextMode(BW80);
				end else begin
					TextMode(BW40);
				end;
			end else begin
				TextMode(7);
				VideoColumns := 80;
			end;
		end else begin
			if VideoColumns = 80 then begin
				TextMode(CO80);
			end else begin
				TextMode(CO40);
			end;
			if not VideoMonochrome then
				TextBackground(borderColor);
			ClrScr;
		end;
		if not VideoCursorVisible then
			VideoHideCursor;
		VideoSetBorderColor(borderColor);
	end;

procedure VideoUninstall;
	begin
		VideoToggleEGAMode(False);
		TextBackground(0);
		VideoColumns := 80;
		if VideoMonochrome then
			TextMode(BW80)
		else
			TextMode(CO80);
		VideoSetBorderColor(0);
		ClrScr;
	end;

{ These do nothing in Linux, but are meant to show or hide the terminal cursor.
It will have to be done in some other way. TODO }
procedure VideoShowCursor;
	begin
		VideoCursorVisible := true;
	end;

procedure VideoHideCursor;
	begin
		VideoCursorVisible := false;
	end;

{ This does nothing in Linux either. I'm keeping the empty function in case
someone who makes e.g. an SDL version would like to implement it. }
procedure VideoSetBorderColor(value: integer);
	begin
	end;

{ X,Y are zero-indexed. If toVideo is true, it copies stored
  character/color data from the given array to screen by using WriteText. If
  toVideo is false, it copies the character/color data from the video memory
  mirror to the given array. }
procedure VideoCopy(xfrom, yfrom, width, height: integer; var buf: TVideoBuffer;
	toVideo: boolean);
	var
		x, y: integer;

	begin
		for y := yfrom to yfrom + height - 1 do
			for x := xfrom to xfrom + width - 1 do
			begin
				if toVideo then
					VideoWriteText(x, y,
					 buf[x+1][y+1].Color,
					 buf[x+1][y+1].Char)
				else
					buf[x+1][y+1] := MainBuffer[x+1][y+1];
			end;
	end;

begin
	VFuzzMode := false;
	VideoBorderColor := 0;
	VideoColumns := 80;
	if LastMode = 7 then begin
		VideoTextSegment := $B000;
		VideoMonochrome := true;
	end else begin
		VideoTextSegment := $B800;
		VideoMonochrome := false;
	end;
	VideoTextPointer := Ptr(VideoTextSegment, $0000);
	VideoCursorVisible := true;
end.
