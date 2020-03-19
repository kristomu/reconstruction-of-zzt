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
		TVideoWriteTextProc = procedure(x, y, color: byte; text: TVideoLine);
	const
		PORT_CGA_PALETTE = $03D9;
	var
		VideoMonochrome: boolean;
		cp437: array[0..255] of String = (' ', '☺', '☻', '♥', '♦', '♣', '♠', '•', '◘', '○', '◙', '♂', '♀', '♪', '♫', '☼', '►', '◄', '↕', '‼', '¶', '§', '▬', '↨', '↑', '↓', '→', '←', '∟', '↔', '▲', '▼', ' ', '!', '"', '#', '$', '%', '&', '''', '(', ')', '*', '+', ',', '-', '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '', 'Ç', 'ü', 'é', 'â', 'ä', 'à', 'å', 'ç', 'ê', 'ë', 'è', 'ï', 'î', 'ì', 'Ä', 'Å', 'É', 'æ', 'Æ', 'ô', 'ö', 'ò', 'û', 'ù', 'ÿ', 'Ö', 'Ü', '¢', '£', '¥', '₧', 'ƒ', 'á', 'í', 'ó', 'ú', 'ñ', 'Ñ', 'ª', 'º', '¿', '⌐', '¬', '½', '¼', '¡', '«', '»', '░', '▒', '▓', '│', '┤', '╡', '╢', '╖', '╕', '╣', '║', '╗', '╝', '╜', '╛', '┐', '└', '┴', '┬', '├', '─', '┼', '╞', '╟', '╚', '╔', '╩', '╦', '╠', '═', '╬', '╧', '╨', '╤', '╥', '╙', '╘', '╒', '╓', '╫', '╪', '┘', '┌', '█', '▄', '▌', '▐', '▀', 'α', 'ß', 'Γ', 'π', 'Σ', 'σ', 'µ', 'τ', 'Φ', 'Θ', 'Ω', 'δ', '∞', 'φ', 'ε', '∩', '≡', '±', '≥', '≤', '⌠', '⌡', '÷', '≈', '°', '∙', '·', '√', 'ⁿ', '²', '■', ' ');
	function VideoConfigure: boolean;
	procedure VideoWriteTextColor(x, y, color: byte; text: TVideoLine);
	procedure VideoWriteTextBW(x, y, color: byte; text: TVideoLine);
	procedure VideoWriteText(x, y, color: byte; text: TVideoLine);
	procedure VideoToggleEGAMode(EGA: Boolean);
	procedure VideoInstall(columns, borderColor: integer);
	procedure VideoWriteTextUTF8(x, y, color: byte; text: TVideoLine);
	procedure VideoUninstall;
	procedure VideoShowCursor;
	procedure VideoHideCursor;
	procedure VideoSetBorderColor(value: integer);
	procedure VideoMove(x, y, chars: integer; data: pointer; toVideo: boolean);

implementation
uses Crt, Dos;
var
	VideoColumns: integer;
	VideoBorderColor: integer;
	VideoTextSegment: word;
	VideoTextPointer: pointer;
	VideoCursorVisible: boolean;

procedure VideoWriteTextUTF8(x, y, color: byte; text: TVideoLine);
	var
		offset: Integer = 0;
		C: Char;

	begin
		GotoXY(x+1, y+1);
		if color > $7F then
			TextColor(color and $F + Blink)
		else
			TextColor(color and $F);
		TextBackground(color shr 4);
		{ Hack from https://stackoverflow.com/a/35140822 
		  A better solution will have to move away from Crt altogether.}
		for C in text do begin
			GotoXY(1, 1);
			GotoXY(x+offset+1, y+1);
			Write(cp437[ord(C)]);
			offset := offset + 1;
		end;
		{Write(text);}
	end;

procedure VideoWriteTextColor(x, y, color: byte; text: TVideoLine);
	begin
		VideoWriteTextUTF8(x, y, color, text);
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
		VideoWriteTextUTF8(x, y, color, text);
	end;

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
				repeat until KeyPressed;
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

{ This procedure is intended to quickly move text data from a spare buffer
  onto the main display buffer. Since Linux doesn't allow direct access to
  B800:0 (or has a concept of this in protected mode), it can't work. I've
  disabled the procedure for now; I'll have to find a way to make it work,
  later. Probably either by just redrawing whatever was obscured, or I may
  use something like ncurses windows or a manual double buffer. 

  The procedudure is intended to move the line from (x,y) to (x+chars, y)
  to video memory (active buffer) if toVideo is true, or from it if false. }
procedure VideoMove(x, y, chars: integer; data: pointer; toVideo: boolean);
	begin
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
