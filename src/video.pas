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
	const
		{ Credits go to Ben Russell (iamgreaser) for this table and
		  the logic behind WriteUnicodeAsUTF8. }
		{ Note, $ED now redirects to U+03D5 as identified by IBM;
		  see Wikipedia note. https://bit.ly/39t1732 Better still
		  would be U+1D719, but then the table would no longer be
		  smallint.}
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
uses Crt, Dos;
var
	VideoColumns: integer;
	VideoBorderColor: integer;
	VideoTextSegment: word;
	VideoTextPointer: pointer;
	VideoCursorVisible: boolean;

function utf8Len(codepoint: smallint): integer;
	begin
		if codepoint < $80 then utf8Len := 1
		else if codepoint < $0800 then utf8Len := 2
		else utf8Len := 3
	end;

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
			charPseudoEnd := x+offset+utf8Len(cp437ToUnicode[ord(C)]);
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
				WriteUnicodeAsUTF8(cp437ToUnicode[ord(C)]);

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
					Delay(100);
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
