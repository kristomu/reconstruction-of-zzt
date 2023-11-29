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
{$V-}
unit Game;

interface
	uses GameVars, TxtWind;
	const
		PROMPT_NUMERIC = 0;
		PROMPT_ALPHANUM = 1;
		PROMPT_ANY = 2;

	{ Sanity checks. }
	function ValidCoord(x, y:integer):boolean;
	function CoordInsideViewport(x, y:integer):boolean;

	procedure SidebarClearLine(y: integer);
	procedure SidebarClear;
	procedure GenerateTransitionTable;
	procedure AdvancePointer(var address: pointer; count: integer);
	procedure BoardClose(showTruncationNote: boolean);
	procedure BoardOpen(boardId: integer; worldIsDamaged: boolean);
	procedure BoardChange(boardId: integer);
	procedure BoardCreate;
	procedure WorldCreate;
	procedure TransitionDrawToFill(chr: char; color: integer);
	procedure BoardDrawTile(x, y: integer);
	procedure BoardDrawBorder;
	procedure TransitionDrawToBoard;
	procedure SidebarPromptCharacter(editable: boolean; x, y: integer; prompt: TString50; var value: byte);
	procedure SidebarPromptSlider(editable: boolean; x, y: integer; prompt: string; var value: byte);
	procedure SidebarPromptChoice(editable: boolean; y: integer; prompt, choiceStr: string; var result: byte);
	procedure SidebarPromptDirection(editable: boolean; y: integer; prompt: string; var deltaX, deltaY: integer);
	procedure PromptString(x, y, arrowColor, color, width: integer; mode: byte; var buffer: TString50);
	function SidebarPromptYesNo(message: string; defaultReturn: boolean): boolean;
	procedure SidebarPromptString(prompt: string; extension: TString50; var filename: string; promptMode: byte);
	procedure PauseOnError;
	function DisplayIOError: boolean;
	procedure DisplayTruncationNote;
	procedure DisplayCorruptionNote;
	procedure WorldUnload;
	function WorldLoad(filename, extension: TString50): boolean;
	procedure WorldSave(filename, extension: TString50);
	procedure GameWorldSave(prompt: TString50; var filename: TString50; extension: TString50);
	function GameWorldLoad(extension: TString50): boolean;
	procedure CopyStatDataToTextWindow(statId: integer; var state: TTextWindowState);
	procedure AddStat(tx, ty: integer; element: byte; color, tcycle: integer; template: TStat);
	procedure RemoveStat(statId: integer);
	function GetStatIdAt(x, y: integer): integer;
	function BoardPrepareTileForPlacement(x, y: integer): boolean;
	procedure MoveStat(statId: integer; newX, newY: integer);
	procedure PopupPromptString(question: string; var buffer: TString50);
	function Signum(val: integer): integer;
	function Difference(a, b: integer): integer;
	procedure DamageStat(attackerStatId: integer);
	procedure BoardDamageTile(x, y: integer);
	procedure BoardAttack(attackerStatId: integer; x, y: integer);
	function BoardShoot(element: byte; tx, ty, deltaX, deltaY: integer; source: integer): boolean;
	procedure CalcDirectionRnd(var deltaX, deltaY: integer);
	procedure CalcDirectionSeek(x, y: integer; var deltaX, deltaY: integer);
	procedure TransitionDrawBoardChange;
	procedure GameUpdateSidebar;
	procedure GameAboutScreen;
	procedure GamePlayLoop(boardChanged: boolean);
	procedure DisplayMessage(ticks: integer; message: string);
	procedure BoardEnter;
	procedure BoardPassageTeleport(x, y: integer);
	procedure GameDebugPrompt;
	procedure GameTitleLoop;
	procedure GamePrintRegisterMessage;
const
	ProgressAnimColors: array[0 .. 7] of byte = ($14, $1C, $15, $1D, $16, $1E, $17, $1F);
	ProgressAnimStrings: array[0 .. 7] of string[5] =
		('....|', '...*/', '..*.-', '.*..\', '*...|', '..../', '....-', '....\');
	ColorNames: array[1 .. 7] of string[8] =
		('Blue', 'Green', 'Cyan', 'Red', 'Purple', 'Yellow', 'White');
	{}
	DiagonalDeltaX: array[0 .. 7] of integer = (-1, 0, 1, 1, 1, 0, -1, -1);
	DiagonalDeltaY: array[0 .. 7] of integer = (1, 1, 1, 0, -1, -1, -1, 0);
	NeighborDeltaX: array[0 .. 3] of integer = (0, 0, -1, 1);
	NeighborDeltaY: array[0 .. 3] of integer = (-1, 1, 0, 0);
	{}
	TileBorder: TTile = (Element: E_NORMAL; Color: $0E);
	TileBoardEdge: TTile = (Element: E_BOARD_EDGE; Color: $00);
	StatTemplateDefault: TStat = (
		X: 0; Y: 0; StepX: 0; StepY: 0;
		Cycle: 0; P1: 0; P2: 0; P3: 0;
		Follower: -1; Leader: -1
	);
	LineChars: string[16] = #249#208#210#186#181#188#187#185#198#200#201#204#205#202#203#206;

implementation
uses Dos, Crt, Video, Sounds, Input, Elements, Editor, Oop, Minmax, Fileops;

function ValidCoord(x, y:integer):boolean;
	begin
		if (x < 0) or (y < 0) then begin
			ValidCoord := false;
			Exit;
		end;
		if (x > BOARD_WIDTH+1) or (y > BOARD_HEIGHT+1) then begin
			ValidCoord := false;
			Exit;
		end;
		ValidCoord := true;
	end;

function CoordInsideViewport(x, y:integer): boolean;
	begin
		if (x <= 0) or (y <= 0) then begin
			CoordInsideViewport := false;
			Exit;
		end;
		if (x > BOARD_WIDTH) or (y > BOARD_HEIGHT) then begin
			CoordInsideViewport := false;
			Exit;
		end;
		CoordInsideViewport := true;
	end;

procedure SidebarClearLine(y: integer);
	begin
		VideoWriteText(60, y, $11, #179'                   ');
	end;

procedure SidebarClear;
	var
		i: integer;
	begin
		for i := 3 to 24 do
			SidebarClearLine(i);
	end;

procedure GenerateTransitionTable;
	var
		ix, iy: integer;
		t: TCoord;
	begin
		TransitionTableSize := 0;
		for iy := 1 to BOARD_HEIGHT do
			for ix := 1 to BOARD_WIDTH do begin
				TransitionTableSize := TransitionTableSize + 1;
				TransitionTable[TransitionTableSize].X := ix;
				TransitionTable[TransitionTableSize].Y := iy;
			end;

		{ shuffle }
		for ix := 1 to TransitionTableSize do begin
			iy := Random(TransitionTableSize) + 1;
			t := TransitionTable[iy];
			TransitionTable[iy] := TransitionTable[ix];
			TransitionTable[ix] := t;
		end;
	end;

procedure AdvancePointer(var address: pointer; count: integer);
	begin
		address := address + count;
	end;

procedure BoardClose(showTruncationNote: boolean);
	var
		ix, iy: integer;
		ptr: pointer;
		ptrStart: pointer;
		rle: TRleTile;
		cleanupNeeded: boolean;
	begin
		ptr := IoTmpBuf;
		ptrStart := IoTmpBuf;
		cleanupNeeded := false;

		Move(Board.Name, ptr^, SizeOf(Board.Name));
		AdvancePointer(ptr, SizeOf(Board.Name));

		ix := 1;
		iy := 1;
		rle.Count := 1;
		rle.Tile := Board.Tiles[ix][iy];
		repeat
			ix := ix + 1;
			if ix > BOARD_WIDTH then begin
				ix := 1;
				iy := iy + 1;
			end;
			if (Board.Tiles[ix][iy].Color = rle.Tile.Color) and
				(Board.Tiles[ix][iy].Element = rle.Tile.Element) and
				(rle.Count < 255) and (iy <= BOARD_HEIGHT) then
			begin
				rle.Count := rle.Count + 1;
			end else begin
				Move(rle, ptr^, SizeOf(rle));
				AdvancePointer(ptr, SizeOf(rle));
				rle.Tile := Board.Tiles[ix][iy];
				rle.Count := 1;
			end;
		until iy > BOARD_HEIGHT;

		Move(Board.Info, ptr^, SizeOf(Board.Info));
		AdvancePointer(ptr, SizeOf(Board.Info));

		Move(Board.StatCount, ptr^, SizeOf(Board.StatCount));
		AdvancePointer(ptr, SizeOf(Board.StatCount));

		for ix := 0 to Board.StatCount do begin
			with Board.Stats[ix] do begin
				if DataLen > 0 then begin
					for iy := 1 to (ix - 1) do begin
						{ IMP: Make all bound objects link to the same one. }
						if Board.Stats[iy].Data = Data then begin
							DataLen := -iy;
							Data := nil;
							Break;
						end;
					end;
				end;
				Move(Board.Stats[ix], ptr^, SizeOf(TStat));
				AdvancePointer(ptr, SizeOf(TStat));
				if DataLen > 0 then begin
					Move(Data^, ptr^, DataLen);
					FreeMem(Data, DataLen);
					AdvancePointer(ptr, DataLen);
				end;
			end;
		end;

		{ For some reason, using @IoTmpBuf instead of ptrStart causes a range check error. }
		World.BoardLen[World.Info.CurrentBoard] := ptr - ptrStart;

		{ If we're using too much space, truncate the size, feed the
		  whole thing back through BoardOpen to fix the inevitable
		  corruption, then run BoardClose again.
		  This smart-ass solution should allow us to keep all the smarts of
		  board parsing in BoardOpen and nowt have to duplicate any logic. }
		{ Such a situation should *only* happen if RLE is too large (see
		  RLEFLOW.ZZT), because AddStat should reject adding stats when
		  there's no room. }
		if World.BoardLen[World.Info.CurrentBoard] > MAX_BOARD_LEN then begin
			World.BoardLen[World.Info.CurrentBoard] := MAX_BOARD_LEN;
			cleanupNeeded := true;
		end;

		{ LEAKFIX: Needs to be ReAllocMem instead of Get/Free because first time
		  around the memory isn't allocated yet.}
		ReAllocMem(World.BoardData[World.Info.CurrentBoard], World.BoardLen[World.Info.CurrentBoard]);
		Move(IoTmpBuf^, World.BoardData[World.Info.CurrentBoard]^, World.BoardLen[World.Info.CurrentBoard]);

		if cleanupNeeded then begin
			BoardOpen(World.Info.CurrentBoard, false);
			BoardClose(false);
			if showTruncationNote then DisplayTruncationNote;
		end;
	end;

{ Clean up stats by processing DataLen reference chains, clamping out-of-
  bounds stat values, and placing a player on the board if there is none
  already. }
procedure AdjustBoardStats;
	var
		ix, iy: integer;

	begin
		{ SANITY: Process referential DataLen variables. This must be
		  done after the former loop because otherwise it could be
		  using incorrect data. }

		for ix := 0 to Board.StatCount do begin
			with Board.Stats[ix] do begin
				if DataLen < 0 then begin
					{ Well-behaved reference chains do nothing in
					  DOS ZZT, so cycles should do nothing too. If
					  we're pointing at another reference or out of
					  bounds, do nothing. }
					{ Furthermore, if we're the player, do nothing.
					  Due to the way that BoardClose works, letting the
					  player refer to a later object's data can't be
					  allowed. Strictly speaking, referring to a later
					  object is not allowed in general, but as long as
					  the object doing the referring is not the player,
					  we can pretend (in BoardClose) that the later object
					  refers to the earlier's data instead. This is not
					  possible with the player, because the reference
					  DataLen would then be -0, which is just 0.}
					if (ix = 0) or (-DataLen > Board.StatCount) or
					   (Board.Stats[-DataLen].DataLen < 0) then
						DataLen := 0;

					if DataLen < 0 then begin
						Data := Board.Stats[-DataLen].Data;
						DataLen := Board.Stats[-DataLen].DataLen;
					end;

					{ If it's part of a chain, break the chain. }
					{ Can we do this?? }
					if DataLen < 0 then
						DataLen := 0;
				end;
			end;
		end;

		{ SANITY: Positive Leader and Follower values must be indices
		  to stats. If they're too large, they're corrupt: set them to
		  zero.
		  Furthermore, there's no need for StepX and StepY to be out of
		  range of the board area, and clamping these values helps
		  avoid a ton of over/underflow problems whose fixes would
		  otherwise clutter up the code... }
		for ix := 0 to Board.StatCount do begin
			with Board.Stats[ix] do begin
				if Follower > Board.StatCount then Follower := 0;
				if Leader > Board.StatCount then Leader := 0;

				if StepX < -BOARD_WIDTH then StepX := -BOARD_WIDTH;
				if StepX > BOARD_WIDTH then StepX := BOARD_WIDTH;

				if StepY < -BOARD_HEIGHT then StepY := -BOARD_HEIGHT;
				if StepY > BOARD_HEIGHT then StepY := BOARD_HEIGHT;
			end;
		end;

		{ SANITY: If there's neither a player nor a monitor at the position
		  indicated by stats 0, place a player there to keep the invariant
		  that one should always exist on every board. }
		with Board.Stats[0] do begin
			if (Board.Tiles[X][Y].Element <> E_PLAYER) and
			   (Board.Tiles[X][Y].Element <> E_MONITOR) then
			   Board.Tiles[X][Y].Element := E_PLAYER;
		end;
	end;

{ Set worldIsDamaged to true if the BoardOpen is from a world load and
the world metadata is wrong; this will make the corruption notification
show up regardless of whether the board itself is damaged. }
procedure BoardOpen(boardId: integer; worldIsDamaged: boolean);
	var
		ptr: ^byte;
		i, ix, iy: integer;
		rle: TRleTile;
		bytesRead: integer = 0;
		boardIsDamaged : boolean;
	begin
		if boardId > World.BoardCount then
			boardId := World.Info.CurrentBoard;

		ptr := World.BoardData[boardId];

		{ Create a default yellow border board, because we might need
		  to abort before the board is fully specced. }
		BoardCreate;
		boardIsDamaged := worldIsDamaged;

		{ Check that the sanity check on board titles have been executed. }

		{SANITY: Reconstruct the title. We need at least a size of
		 two bytes for the title: a size designation and the first
		 letter of the title. If we don't even have that, let the
		 title be blank.}
		if World.BoardLen[boardId] < SizeOf(Board.Name) then begin
			World.Info.CurrentBoard := boardId;

			if World.BoardLen[boardId] > 1 then begin
				ptr^ := Min(ptr^, World.BoardLen[boardId]-1);
				Move(ptr^, Board.Name, ptr^);
			end else begin
				Board.Name := '';
			end;

			{ This board is damaged. }
			DisplayCorruptionNote;
			Exit;
		end;

		{ SANITY: Range check on board name length. }
		ptr^ := Min(ptr^, SizeOf(Board.Name));

		Move(ptr^, Board.Name, SizeOf(Board.Name));
		AdvancePointer(ptr, SizeOf(Board.Name));
		bytesRead := bytesRead + SizeOf(Board.Name);

		ix := 1;
		iy := 1;
		rle.Count := 0;
		repeat
			{ ZZT used to have a "feature" where an RLE count of 0 would
		      mean 256 repetitions of the tile. However, because it never
		      writes those RLE counts itself, it would get desynchronized
		      on a board close and crash. Therefore, we must simply ignore
		      such rle count 0 bytes, even though that is not what DOS ZZT
		      does. DOS ZZT would instead write past the bounds of the
		      scratch space when cleaning up, which means authors can't use
		      any RLE count 0 pairs without risking a glitch or crash
		      anyway.

		      BoardClose and BoardOpen may still desynchronize, but what
		      BoardClose outputs will never be longer than what BoardOpen
		      inputs, which is okay.

		      If you absolutely need this functionality, you would have to
		      increment an auxiliary counter by one every time you get an
		      RLE count 0 byte and then allocate that much extra scratch
		      space. But keeping two counts like that is a pain, so I don't.}
			if rle.Count <= 0 then begin
				{ Not enough space? Get outta here. }
				if bytesRead + SizeOf(rle) > World.BoardLen[boardId] then Break;
				Move(ptr^, rle, SizeOf(rle));
				AdvancePointer(ptr, SizeOf(rle));
				bytesRead := bytesRead + SizeOf(rle);
				if rle.Count = 0 then begin
					boardIsDamaged := true;
					Continue;
				end;
			end;

			{ SANITY: If the element is unknown, replace it with a normal. }

			if rle.Tile.Element > MAX_ELEMENT then begin
				rle.Tile.Element := E_NORMAL;
				boardIsDamaged := true;
			end;

			Board.Tiles[ix][iy] := rle.Tile;
			ix := ix + 1;
			if ix > BOARD_WIDTH then begin
				ix := 1;
				iy := iy + 1;
			end;

			rle.Count := rle.Count - 1;

		until (iy > BOARD_HEIGHT) or (bytesRead >= World.BoardLen[boardId]);

		{ SANITY: If reading board info and the stats count byte would
		  get us out of bounds, we have a board that's truncated too early.
		  Do the best we can, then show the damaged board note and exit. }
		if (SizeOf(Board.Info) + SizeOf(Board.StatCount) + bytesRead) >= World.BoardLen[boardId] then begin
			World.Info.CurrentBoard := boardId;
			AdjustBoardStats;
			DisplayCorruptionNote;
			Exit;
		end;

		Move(ptr^, Board.Info, SizeOf(Board.Info));
		AdvancePointer(ptr, SizeOf(Board.Info));
		bytesRead := bytesRead + SizeOf(Board.Info);

		{ Clamp out-of-bounds Board.Info variables. They'll cause problems
		  in the editor otherwise. }
		for i := 0 to 3 do
			{ This behavior is from elements.pas, BoardEdgeTouch. }
			if Board.Info.NeighborBoards[i] > World.BoardCount then
				Board.Info.NeighborBoards[i] := boardId;

		if not ValidCoord(Board.Info.StartPlayerX, Board.Info.StartPlayerY) then begin
			Board.Info.StartPlayerX := 1;
			Board.Info.StartPlayerY := 1;
			boardIsDamaged := true;
		end;

		Move(ptr^, Board.StatCount, SizeOf(Board.StatCount));
		AdvancePointer(ptr, SizeOf(Board.StatCount));
		bytesRead := bytesRead + SizeOf(Board.StatCount);

		Board.StatCount := Max(0, Min(Board.StatCount, MAX_STAT));

		for ix := 0 to Board.StatCount do
			with Board.Stats[ix] do begin
				{ SANITY: Handle too few stats items for the stats count. }
				if (bytesRead + SizeOf(TStat)) > World.BoardLen[boardId] then begin
					Board.StatCount := Max(ix - 1, 0);
					World.Info.CurrentBoard := boardId;
					boardIsDamaged := true;
					Break;
				end;

				Move(ptr^, Board.Stats[ix], SizeOf(TStat));
				AdvancePointer(ptr, SizeOf(TStat));
				bytesRead := bytesRead + SizeOf(TStat);

				{ SANITY: If the element underneath is unknown, replace it
				  with a normal. }
				if Under.Element > MAX_ELEMENT then begin
					Under.Element := E_NORMAL;
					boardIsDamaged := true;
				end;

				{ SANITY: Handle objects that are out of bounds. }
				if not ValidCoord(X, Y) then begin
					X := Min(Max(X, 0), BOARD_WIDTH+1);
					Y := Min(Max(Y, 0), BOARD_HEIGHT+1);
					boardIsDamaged := true;
				end;

				{ SANITY: (0,0) is not available: it's used by one-line
				  messages. So if the stat is at (0,0) or another
				  unavailable position, put it into (1,1). TODO? Make
				  a note of which are thus placed, and place them on
				  empty spots on the board instead if possible... }
				{ The compromise to the Postelic position is probably to
				  be generous, but show a warning message that the board
				  was corrupted and attempted fixed. }
				if (X = 0) and (Y = 0) then begin
					X := 1;
					Y := 1;
					boardIsDamaged := true;
				end;

				{ SANITY: If DataLen is much too large, truncate. We'll
			      then stop processing more objects next round around
			      the loop. }
				if bytesRead + DataLen > World.BoardLen[boardId] then begin
					DataLen := World.BoardLen[boardId] - bytesRead;
					boardIsDamaged := true;
				end;

				if DataLen > 0 then
					{ SANITY: If DataLen is too long, truncate it. }
					if DataLen > World.BoardLen[boardId]-bytesRead then begin
						DataLen := World.BoardLen[boardId]-bytesRead;
						boardIsDamaged := true;
					end;

				{ Only allocate if data length is still positive... }
				if DataLen > 0 then begin
					GetMem(Data, DataLen);
					Move(ptr^, Data^, DataLen);
					AdvancePointer(ptr, DataLen);
					bytesRead := bytesRead + DataLen;
				end;

				{ Otherwise, clear Data to avoid potential leaks later. }
				if DataLen = 0 then Data := nil;
			end;

		AdjustBoardStats;
		World.Info.CurrentBoard := boardId;

		if boardIsDamaged then
			DisplayCorruptionNote;
	end;

procedure BoardChange(boardId: integer);
	begin
		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element := E_PLAYER;
		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color := ElementDefs[E_PLAYER].Color;
		if boardId <> World.Info.CurrentBoard then begin
			BoardClose(true);
			BoardOpen(boardId, false);
		end;
	end;

procedure BoardCreate;
	var
		ix, iy, i: integer;
	begin
		Board.Name := '';
		Board.Info.Message := '';
		Board.Info.MaxShots := 255;
		Board.Info.IsDark := false;
		Board.Info.ReenterWhenZapped := false;
		Board.Info.TimeLimitSec := 0;
		for i := 0 to 3 do
			Board.Info.NeighborBoards[i] := 0;

		for ix := 0 to BOARD_WIDTH+1 do begin
			Board.Tiles[ix][0] := TileBoardEdge;
			Board.Tiles[ix][BOARD_HEIGHT+1] := TileBoardEdge;
		end;
		for iy := 0 to BOARD_HEIGHT+1 do begin
			Board.Tiles[0][iy] := TileBoardEdge;
			Board.Tiles[BOARD_WIDTH+1][iy] := TileBoardEdge;
		end;

		for ix := 1 to BOARD_WIDTH do
			for iy := 1 to BOARD_HEIGHT do begin
				Board.Tiles[ix][iy].Element := E_EMPTY;
				Board.Tiles[ix][iy].Color := 0;
			end;

		for ix := 1 to BOARD_WIDTH do begin
			Board.Tiles[ix][1] := TileBorder;
			Board.Tiles[ix][BOARD_HEIGHT] := TileBorder;
		end;
		for iy := 1 to BOARD_HEIGHT do begin
			Board.Tiles[1][iy] := TileBorder;
			Board.Tiles[BOARD_WIDTH][iy] := TileBorder;
		end;

		Board.Tiles[BOARD_WIDTH div 2][BOARD_HEIGHT div 2].Element := E_PLAYER;
		Board.Tiles[BOARD_WIDTH div 2][BOARD_HEIGHT div 2].Color := ElementDefs[E_PLAYER].Color;
		Board.StatCount := 0;
		Board.Stats[0].X := BOARD_WIDTH div 2;
		Board.Stats[0].Y := BOARD_HEIGHT div 2;
		Board.Stats[0].Cycle := 1;
		Board.Stats[0].Under.Element := E_EMPTY;
		Board.Stats[0].Under.Color := 0;
		Board.Stats[0].Data := nil;
		Board.Stats[0].DataLen := 0;
	end;

procedure WorldCreate;
	var
		i: integer;
	begin
		InitElementsGame;
		World.BoardCount := 0;
		World.BoardLen[0] := 0;
		InitEditorStatSettings;
		ResetMessageNotShownFlags;
		BoardCreate;
		World.Info.IsSave := false;
		World.Info.CurrentBoard := 0;
		World.Info.Ammo := 0;
		World.Info.Gems := 0;
		World.Info.Health := 100;
		World.Info.EnergizerTicks := 0;
		World.Info.Torches := 0;
		World.Info.TorchTicks := 0;
		World.Info.Score := 0;
		World.Info.BoardTimeSec := 0;
		World.Info.BoardTimeHsec := 0;
		for i := 1 to 7 do
			World.Info.Keys[i] := false;
		for i := 1 to MAX_FLAG do
			World.Info.Flags[i] := '';
		BoardChange(0);
		Board.Name := 'Title screen';
		LoadedGameFileName := '';
		World.Info.Name := '';
	end;

procedure TransitionDrawToFill(chr: char; color: integer);
	var
		i: integer;
	begin
		for i := 1 to TransitionTableSize do
			VideoWriteText(TransitionTable[i].X - 1, TransitionTable[i].Y - 1, color, chr);
	end;

procedure BoardDrawTile(x, y: integer);
	var
		ch: byte;
	begin
		with Board.Tiles[x][y] do begin
			if not Board.Info.IsDark
				or (ElementDefs[Board.Tiles[x][y].Element].VisibleInDark)
				or (
					(World.Info.TorchTicks > 0)
					and ((Sqr(Board.Stats[0].X - x) + Sqr(Board.Stats[0].Y - y) * 2) < TORCH_DIST_SQR)
				) or ForceDarknessOff then
			begin
				if Element = E_EMPTY then
					VideoWriteText(x - 1, y - 1, $0F, ' ')
				else if (Element < E_TEXT_MIN) and ElementDefs[Element].HasDrawProc then begin
					ElementDefs[Element].DrawProc(x, y, ch);
					VideoWriteText(x - 1, y - 1, Color, Chr(ch));
				end else if Element < E_TEXT_MIN then
					VideoWriteText(x - 1, y - 1, Color, ElementDefs[Element].Character)
				else begin
					{ Text drawing }
					if Element = E_TEXT_WHITE then
						VideoWriteText(x - 1, y - 1, $0F, Chr(Board.Tiles[x][y].Color))
					else if VideoMonochrome then
						VideoWriteText(x - 1, y - 1, ((Element - E_TEXT_MIN) + 1) * 16, Chr(Board.Tiles[x][y].Color))
					else
						VideoWriteText(x - 1, y - 1, (((Element - E_TEXT_MIN) + 1) * 16) + $F, Chr(Board.Tiles[x][y].Color));
				end
			end else begin
				{ Darkness }
				VideoWriteText(x - 1, y - 1, $07, #176);
			end;
		end;
	end;

procedure BoardDrawBorder;
	var
		ix, iy: integer;
	begin
		for ix := 1 to BOARD_WIDTH do begin
			BoardDrawTile(ix, 1);
			BoardDrawTile(ix, BOARD_HEIGHT);
		end;

		for iy := 1 to BOARD_HEIGHT do begin
			BoardDrawTile(1, iy);
			BoardDrawTile(BOARD_WIDTH, iy);
		end;
	end;

procedure TransitionDrawToBoard;
	var
		i: integer;
	begin
		BoardDrawBorder;

		for i := 1 to TransitionTableSize do
			with TransitionTable[i] do
				BoardDrawTile(X, Y);
	end;

procedure SidebarPromptCharacter(editable: boolean; x, y: integer; prompt: TString50; var value: byte);
	var
		i, newValue: integer;
	begin
		SidebarClearLine(y);
		VideoWriteText(x, y, Integer(editable) + $1E, prompt);
		SidebarClearLine(y + 1);
		VideoWriteText(x + 5, y + 1, $9F, #31);
		SidebarClearLine(y + 2);

		repeat
			for i := (value - 4) to (value + 4) do
				VideoWriteText(((x + i) - value) + 5, y + 2, $1E, Chr((i + $100) mod $100));

			if editable then begin
				Delay(25);
				InputUpdate;
				if InputKeyPressed = KEY_TAB then
					InputDeltaX := 9;

				newValue := value + InputDeltaX;
				if value <> newValue then begin
					value := (newValue + $100) mod $100;
					SidebarClearLine(y + 2);
				end;
			end;
		until (InputKeyPressed = KEY_ENTER) or (InputKeyPressed = KEY_ESCAPE) or not editable or InputShiftPressed;

		VideoWriteText(x + 5, y + 1, $1F, #31);
	end;

procedure SidebarPromptSlider(editable: boolean; x, y: integer; prompt: string; var value: byte);
	var
		newValue: integer;
		startChar, endChar: char;
	begin
		if prompt[Length(prompt) - 2] = ';' then begin
			startChar := prompt[Length(prompt) - 1];
			endChar := prompt[Length(prompt)];
			prompt := Copy(prompt, 1, Length(prompt) - 3);
		end else begin
			startChar := '1';
			endChar := '9';
		end;

		SidebarClearLine(y);
		VideoWriteText(x, y, Integer(editable) + $1E, prompt);
		SidebarClearLine(y + 1);
		SidebarClearLine(y + 2);
		VideoWriteText(x, y + 2, $1e, startChar + '....:....' + endChar);

		repeat
			if editable then begin
				if InputJoystickMoved then
					Delay(45);
				VideoWriteText(x + value + 1, y + 1, $9F, #31);

				InputUpdate;
				if (InputKeyPressed >= '1') and (InputKeyPressed <= '9') then begin
					value := Ord(InputKeyPressed) - 49;
					SidebarClearLine(y + 1);
				end else begin
					newValue := value + InputDeltaX;
					if (value <> newValue) and (newValue >= 0) and (newValue <= 8) then begin
						value := newValue;
						SidebarClearLine(y + 1);
					end;
				end;
			end;
		until (InputKeyPressed = KEY_ENTER) or (InputKeyPressed = KEY_ESCAPE) or not editable or InputShiftPressed;

		VideoWriteText(x + value + 1, y + 1, $1F, #31);
	end;

procedure SidebarPromptChoice(editable: boolean; y: integer; prompt, choiceStr: string; var result: byte);
	var
		i, j, choiceCount: integer;
		newResult: integer;
	begin
		SidebarClearLine(y);
		SidebarClearLine(y + 1);
		SidebarClearLine(y + 2);
		VideoWriteText(63, y, Integer(editable) + $1E, prompt);
		VideoWriteText(63, y + 2, $1E, choiceStr);

		choiceCount := 1;
		for i := 1 to Length(choiceStr) do
			if choiceStr[i] = ' ' then
				choiceCount := choiceCount + 1;

		repeat
			j := 0;
			i := 1;
			while (j < result) and (i < Length(choiceStr)) do begin
				if choiceStr[i] = ' ' then
					j := j + 1;
				i := i + 1;
			end;

			if editable then begin
				VideoWriteText(62 + i, y + 1, $9F, #31);
				Delay(35);
				InputUpdate;

				newResult := result + InputDeltaX;
				if (result <> newResult) and (newResult >= 0) and (newResult <= (choiceCount - 1)) then begin
					result := newResult;
					SidebarClearLine(y + 1);
				end;
			end;
		until (InputKeyPressed = KEY_ENTER) or (InputKeyPressed = KEY_ESCAPE) or not editable or InputShiftPressed;

		VideoWriteText(62 + i, y + 1, $1F, #31);
	end;

procedure SidebarPromptDirection(editable: boolean; y: integer; prompt: string; var deltaX, deltaY: integer);
	var
		choice: byte;
	begin
		if deltaY = -1 then
			choice := 0
		else if deltaY = 1 then
			choice := 1
		else if deltaX = -1 then
			choice := 2
		else
			choice := 3;
		SidebarPromptChoice(editable, y, prompt, #24' '#25' '#27' '#26, choice);
		deltaX := NeighborDeltaX[choice];
		deltaY := NeighborDeltaY[choice];
	end;

procedure PromptString(x, y, arrowColor, color, width: integer; mode: byte; var buffer: TString50);
	var
		i: integer;
		oldBuffer: string;
		firstKeyPress: boolean;
	begin
		oldBuffer := buffer;
		firstKeyPress := true;

		repeat
			for i := 0 to (width - 1) do begin
				VideoWriteText(x + i, y, color, ' ');
				VideoWriteText(x + i, y - 1, arrowColor, ' ');
			end;
			VideoWriteText(x + width, y - 1, arrowColor, ' ');
			VideoWriteText(x + Length(buffer), y - 1, (arrowColor div $10) * 16 + $0F, #31);
			VideoWriteText(x, y, color, buffer);

			InputReadWaitKey;

			if (Length(buffer) < width) and (InputKeyPressed >= #32) and (not InputSpecialKeyPressed) then begin
				if firstKeyPress then
					buffer := '';
				case mode of
					PROMPT_NUMERIC: begin
						if (InputKeyPressed in ['0' .. '9']) then begin
							buffer := buffer + InputKeyPressed;
						end;
					end;
					PROMPT_ANY: begin
						buffer := buffer + InputKeyPressed;
					end;
					PROMPT_ALPHANUM: begin
						if (UpCase(InputKeyPressed) in ['A' .. 'Z'])
							or (InputKeyPressed in ['0' .. '9'])
							or (InputKeyPressed = '-') then
						begin
							buffer := buffer + UpCase(InputKeyPressed);
						end;
					end;
				end;
			end else if (InputKeyPressed = KEY_LEFT) or (InputKeyPressed = KEY_BACKSPACE) then begin
				buffer := Copy(buffer, 1, Length(buffer) - 1);
			{IMP: Clear the whole line if Home is pressed.}
			end else if (InputKeyPressed = KEY_HOME) then begin
				buffer := '';
			end;

			firstKeyPress := false;
		until (InputKeyPressed = KEY_ENTER) or (InputKeyPressed = KEY_ESCAPE);
		if InputKeyPressed = KEY_ESCAPE then begin
			buffer := oldBuffer;
		end;
	end;

function SidebarPromptYesNo(message: string; defaultReturn: boolean): boolean;
	begin
		SidebarClearLine(3);
		SidebarClearLine(4);
		SidebarClearLine(5);
		VideoWriteText(63, 5, $1F, message);
		VideoWriteText(63 + Length(message), 5, $9E, '_');

		repeat
			InputReadWaitKey;
		until UpCase(InputKeyPressed) in [KEY_ESCAPE, 'N', 'Y'];
		if UpCase(InputKeyPressed) = 'Y' then
			defaultReturn := true
		else
			defaultReturn := false;

		SidebarClearLine(5);
		SidebarPromptYesNo := defaultReturn;
	end;

procedure SidebarPromptString(prompt: string; extension: TString50; var filename: string; promptMode: byte);
	begin
		SidebarClearLine(3);
		SidebarClearLine(4);
		SidebarClearLine(5);
		VideoWriteText(75 - Length(prompt), 3, $1F, prompt);
		VideoWriteText(63, 5, $0F, '        ' + extension);

		PromptString(63, 5, $1E, $0F, 8, promptMode, filename);

		SidebarClearLine(3);
		SidebarClearLine(4);
		SidebarClearLine(5);
	end;

procedure PauseOnError;
	begin
		SoundQueue(1, SoundParse('s004x114x9'));
		Delay(2000);
	end;

function DisplayIOError: boolean;
	var
		ioResVal: word;
		errorNumStr: TString50;
		textWindow: TTextWindowState;
	begin
		ioResVal := IOResult;
		if ioResVal = 0 then begin
			DisplayIOError := false;
			exit;
		end;

		DisplayIOError := true;

		{IMP: Use explanations instead of numeric error codes if possible.}
		textWindow.Title := 'Error: ' + ErrorString(ioResVal);
		TextWindowInitState(textWindow);
		TextWindowAppend(textWindow, '$DOS Error: ');
		TextWindowAppend(textWindow, '');
		TextWindowAppend(textWindow, 'This may be caused by missing');
		TextWindowAppend(textWindow, 'ZZT files or a bad disk.  If');
		TextWindowAppend(textWindow, 'you are trying to save a game,');
		TextWindowAppend(textWindow, 'your disk may be full -- try');
		TextWindowAppend(textWindow, 'using a blank, formatted disk');
		TextWindowAppend(textWindow, 'for saving the game!');

		TextWindowDrawOpen(textWindow);
		TextWindowSelect(textWindow, false, false);
		TextWindowDrawClose(textWindow);
		TextWindowFree(textWindow);
	end;

procedure DisplayTruncationNote;
	var
		textWindow: TTextWindowState;
	begin
		textWindow.Title := 'Warning: Potential data loss';
		TextWindowInitState(textWindow);
		TextWindowAppend(textWindow, '$Warning:');
		TextWindowAppend(textWindow, '');
		TextWindowAppend(textWindow, 'A board that was just saved was too large');
		TextWindowAppend(textWindow, 'and some data had to be cut. This might');
		TextWindowAppend(textWindow, 'lead to data loss. If you haven''t saved');
		TextWindowAppend(textWindow, 'yet, do so under another name and make');
		TextWindowAppend(textWindow, 'the board smaller!');
		TextWindowAppend(textWindow, '');
		TextWindowAppend(textWindow, 'If you''re just playing, tell the author');
		TextWindowAppend(textWindow, 'of the world that you''re playing.');

		TextWindowDrawOpen(textWindow);
		TextWindowSelect(textWindow, false, false);
		TextWindowDrawClose(textWindow);
		TextWindowFree(textWindow);
	end;

procedure DisplayCorruptionNote;
	var
		textWindow: TTextWindowState;
	begin
		textWindow.Title := 'Warning: Corruption detected';
		TextWindowInitState(textWindow);
		TextWindowAppend(textWindow, '$Warning:');
		TextWindowAppend(textWindow, '');
		TextWindowAppend(textWindow, 'The file or board that was just loaded');
		TextWindowAppend(textWindow, 'contained some damaged information.');
		TextWindowAppend(textWindow, 'This might be caused by a bad file');
		TextWindowAppend(textWindow, 'or disk corruption. ZZT has tried');
		TextWindowAppend(textWindow, 'to undo the damage, but some data');
		TextWindowAppend(textWindow, 'might be lost.');

		TextWindowDrawOpen(textWindow);
		TextWindowSelect(textWindow, false, false);
		TextWindowDrawClose(textWindow);
		TextWindowFree(textWindow);
	end;

procedure WorldUnload;
	var
		i: integer;
	begin
		{ no need to show any notices if the world's to be unloaded. }
		BoardClose(false);
		{ Reallocating to 0 is better than freeing because it's then
	      possible to reallocate back to a higher level later, if
	      required. That BoardLen is 0 also makes it obvious that the
	      boards have been unloaded. }
		for i := 0 to World.BoardCount do begin
			World.BoardLen[i] := 0;
			ReAllocMem(World.BoardData[i], World.BoardLen[i]);
		end;
	end;

function WorldLoad(filename, extension: TString50): boolean;
	var
		f: file;
		ptr: pointer;
		boardId: integer;
		loadProgress: integer;
		actuallyRead: word;
		firstZero: integer;
		i: integer;
		worldIsDamaged: boolean;
	procedure SidebarAnimateLoading;
		begin
			VideoWriteText(69, 5, ProgressAnimColors[loadProgress], ProgressAnimStrings[loadProgress]);
			loadProgress := (loadProgress + 1) mod 8;
		end;
	begin
		WorldLoad := false;
		worldIsDamaged := false;
		loadProgress := 0;

		SidebarClearLine(4);
		SidebarClearLine(5);
		SidebarClearLine(5);
		VideoWriteText(62, 5, $1F, 'Loading.....');

		{ filenames must be C strings, which means that they are terminated
		  at the first 00, and thus we must remove everything from the first
		  00 out. Not doing this can cause Assign to assign stdin to
		  the file handle, with predictable results. }

		firstZero := Length(filename)+1;
		for i := Length(filename) downto 1 do
			if filename[i] = #0 then
				firstZero := i;

		SetLength(filename, firstZero-1);

		if filename + extension = '' then Exit;

		Assign(f, filename + extension);
		OpenForRead(f, 1);

		if not DisplayIOError then begin
			WorldUnload;
			BlockRead(f, IoTmpBuf^, WORLD_FILE_HEADER_SIZE);

			if not DisplayIOError then begin
				ptr := IoTmpBuf;
				Move(ptr^, World.BoardCount, SizeOf(World.BoardCount));
				AdvancePointer(ptr, SizeOf(World.BoardCount));

				if World.BoardCount < 0 then begin
					if World.BoardCount <> -1 then begin
						VideoWriteText(63, 5, $1E, 'You need a newer');
						VideoWriteText(63, 6, $1E, ' version of ZZT!');
						exit;
					end else begin
						Move(ptr^, World.BoardCount, SizeOf(World.BoardCount));
						AdvancePointer(ptr, SizeOf(World.BoardCount));
					end;
				end;

				Move(ptr^, World.Info, SizeOf(World.Info));
				AdvancePointer(ptr, SizeOf(World.Info));

				{ If the board count is negative, set it to zero. This should
				  also signal that the world is corrupt. Another option
				  would be to make all the fields unsigned, but who needs
				  worlds with >32k boards anyway? Besides, they'd crash
				  DOS ZZT. }
				if World.BoardCount < 0 then begin
					World.BoardCount := 0;
					worldIsDamaged := true;
				end;

				{ If there are too many boards, ditto. (That's a more serious
				  problem, as it may cut off boards outright.) }
				if World.BoardCount > MAX_BOARD then begin
					World.BoardCount := MAX_BOARD;
					worldIsDamaged := true;
				end;

				{ Don't accept CurrentBoard values that are too large or
				  small. }
				if (World.Info.CurrentBoard < 0) or (World.Info.CurrentBoard > World.BoardCount) then begin
					World.Info.CurrentBoard := Max(0, Min(World.BoardCount,
						World.Info.CurrentBoard));
					worldIsDamaged := true;
				end;

				for boardId := 0 to World.BoardCount do begin
					SidebarAnimateLoading;

					if boardId > World.BoardCount then continue;

					BlockRead(f, World.BoardLen[boardId], 2);

					{ Sanity check. Abort at this position so that any
					  boards before the corrupted one can still be
					  recovered.}
					if (DisplayIOError) or (World.BoardLen[boardId] < 0) then begin
						World.BoardLen[boardId] := 0;
						worldIsDamaged := true;
						if boardId = 0 then begin
							WorldUnload;
							Exit;
						end;
						World.BoardCount := boardId - 1;
						{ No more boards to be had, so break. }
						Break;
					end else begin
						{ If it's the last board, get everything we can.
						  This recovers the last Super Lock-corrupted board.
						  actuallyRead below will adjust the board length back
						  if we're dealing with an ordinary world. }
						if boardId = World.BoardCount then
							World.BoardLen[boardId] := MAX_BOARD_LEN;

						GetMem(World.BoardData[boardId], World.BoardLen[boardId]);
						BlockRead(f, World.BoardData[boardId]^, World.BoardLen[boardId],
							actuallyRead);
						{ SANITY: If reading the whole board would lead to an
  	  					  overflow down the line, pretend we only read the
						  MAX_BOARD_LEN first. }
						if actuallyRead > MAX_BOARD_LEN then begin
							actuallyRead := Min(actuallyRead, MAX_BOARD_LEN);
							worldIsDamaged := true;
					    end;

						{ SANITY: reallocate and update board len if
						  there's a mismatch between how much we were told
						  we could read, and how much we actually read.
						  This also cuts down very large boards that we can't
						  represent in memory anyway (size > 20k). }
						{ If you want to be extra stingy with memory, just
						  move this logic up to BlockRead and only read up to
						  MAX_BOARD_LEN, then seek the rest of the way. But
						  I can't be bothered. }
						if actuallyRead <> World.BoardLen[boardId] then begin
							World.BoardData[boardId] := ReAllocMem(World.BoardData[boardId],
								actuallyRead);
							World.BoardLen[boardId] := actuallyRead;
						end;
					end;
				end;

				Close(f);

				{ More sanity checks. If the current board number is
				  negative or too high, set it to zero. }
				if (World.Info.CurrentBoard < 0) or
					(World.Info.CurrentBoard > Min(MAX_BOARD, World.BoardCount)) then
					World.Info.CurrentBoard := 0;

				BoardOpen(World.Info.CurrentBoard, worldIsDamaged);
				LoadedGameFileName := filename;
				WorldLoad := true;

				HighScoresLoad;

				SidebarClearLine(5);
			end;
		end;
	end;

procedure WorldSave(filename, extension: TString50);
	var
		f: file;
		i: integer;
		unk1: integer;
		ptr: pointer;
		version: integer;
	label OnError;
	begin
		BoardClose(true);
		VideoWriteText(63, 5, $1F, 'Saving...');

		Assign(f, filename + extension);
		OpenForWrite(f, 1);

		if not DisplayIOError then begin
			ptr := IoTmpBuf;
			FillChar(IoTmpBuf^, WORLD_FILE_HEADER_SIZE, 0);
			version := -1;
			Move(version, ptr^, SizeOf(version));
			AdvancePointer(ptr, SizeOf(version));

			Move(World.BoardCount, ptr^, SizeOf(World.BoardCount));
			AdvancePointer(ptr, SizeOf(World.BoardCount));

			Move(World.Info, ptr^, SizeOf(World.Info));
			AdvancePointer(ptr, SizeOf(World.Info));

			BlockWrite(f, IoTmpBuf^, WORLD_FILE_HEADER_SIZE);
			if DisplayIOError then goto OnError;

			for i := 0 to World.BoardCount do begin
				BlockWrite(f, World.BoardLen[i], 2);
				if DisplayIOError then goto OnError;

				BlockWrite(f, World.BoardData[i]^, World.BoardLen[i]);
				if DisplayIOError then goto OnError;
			end;

			Close(f);
		end;

		BoardOpen(World.Info.CurrentBoard, false);
		SidebarClearLine(5);
		exit;

	OnError:
		Close(f);
		Erase(f);
		BoardOpen(World.Info.CurrentBoard, false);
		SidebarClearLine(5);
	end;

procedure GameWorldSave(prompt: TString50; var filename: TString50; extension: TString50);
	var
		newFilename: TString50;
	begin
		newFilename := filename;
		SidebarPromptString(prompt, extension, newFilename, PROMPT_ALPHANUM);
		if (InputKeyPressed <> KEY_ESCAPE) and (Length(newFilename) <> 0) then begin
			filename := newFilename;
			if extension = '.ZZT' then
				World.Info.Name := filename;
			WorldSave(filename, extension);
		end;
	end;

function GameWorldLoad(extension: TString50): boolean;
	var
		textWindow: TTextWindowState;
		fileSearchRec: SearchRec;
		entryName: string;
		i: integer;
	begin
		TextWindowInitState(textWindow);
		if extension = '.ZZT' then
			textWindow.Title := 'ZZT Worlds'
		else
			textWindow.Title := 'Saved Games';
		GameWorldLoad := false;
		textWindow.Selectable := true;

		FindFirst('*' + extension, AnyFile, fileSearchRec);
		while DosError = 0 do begin
			entryName := Copy(fileSearchRec.Name, 1, Length(fileSearchRec.name) - 4);

			for i := 1 to WorldFileDescCount do
				if entryName = WorldFileDescKeys[i] then
					entryName := WorldFileDescValues[i];

			TextWindowAppend(textWindow, entryName);
			FindNext(fileSearchRec);
		end;
		TextWindowSort(textWindow); { Sort the file names. }
		TextWindowAppend(textWindow, 'Exit');

		TextWindowDrawOpen(textWindow);
		TextWindowSelect(textWindow, false, false);
		TextWindowDrawClose(textWindow);

		if (textWindow.LinePos < textWindow.LineCount) and not TextWindowRejected then begin
			entryName := textWindow.Lines[textWindow.LinePos]^;
			if Pos(' ', entryName) <> 0 then
				entryName := Copy(entryName, 1, Pos(' ', entryName) - 1);

			GameWorldLoad := WorldLoad(entryName, extension);
			TransitionDrawToFill(#219, $44);
		end;

		TextWindowFree(textWindow);
	end;

procedure CopyStatDataToTextWindow(statId: integer; var state: TTextWindowState);
	var
		dataStr: string;
		dataPtr: pointer;
		dataChr: char;
		i: integer;
	begin
		with Board.Stats[statId] do begin
			TextWindowInitState(state);
			dataStr := '';
			dataPtr := Data;

			{ IMP: Fix off-by-one: Don't start counting
			  from 0 when copying data. }
			for i := 1 to DataLen do begin
				Move(dataPtr^, dataChr, 1);
				if dataChr = KEY_ENTER then begin
					TextWindowAppend(state, dataStr);
					dataStr := '';
				end else begin
					dataStr := dataStr + dataChr;
				end;
				AdvancePointer(dataPtr, 1);
			end;
		end;
	end;

procedure AddStat(tx, ty: integer; element: byte; color, tcycle: integer; template: TStat);
	begin
		{ First of all: check if we have space. If not, no can do! }
		if (template.Data = nil) and
			(World.BoardLen[World.Info.CurrentBoard] + SizeOf(TStat) > MAX_BOARD_LEN) then
			Exit;
		if (template.Data <> nil) and
			(World.BoardLen[World.Info.CurrentBoard] + SizeOf(TStat) + template.DataLen > MAX_BOARD_LEN) then
			Exit;

		{ Can't put anything on top of the player. }
		if (tx = Board.Stats[0].X) and (ty = Board.Stats[0].Y) then Exit;

		if Board.StatCount < MAX_STAT then begin
			Board.StatCount := Board.StatCount + 1;
			Board.Stats[Board.StatCount] := template;
			World.BoardLen[World.Info.CurrentBoard] := World.BoardLen[World.Info.CurrentBoard] + SizeOf(TStat);
			with Board.Stats[Board.StatCount] do begin
				X := tx;
				Y := ty;
				Cycle := tcycle;
				Under := Board.Tiles[tx][ty];
				DataPos := 0;
			end;

			if (template.Data <> nil) and (template.DataLen > 0) then begin
				GetMem(Board.Stats[Board.StatCount].Data, template.DataLen);
				Move(template.Data^, Board.Stats[Board.StatCount].Data^, template.DataLen);
				World.BoardLen[World.Info.CurrentBoard] := World.BoardLen[World.Info.CurrentBoard] + template.DataLen;
			end;

			if ElementDefs[Board.Tiles[tx][ty].Element].PlaceableOnTop then
				Board.Tiles[tx][ty].Color := (color and $0F) + (Board.Tiles[tx][ty].Color and $70)
			else
				Board.Tiles[tx][ty].Color := color;
			Board.Tiles[tx][ty].Element := element;

			if CoordInsideViewport(tx, ty) then
				BoardDrawTile(tx, ty);
		end;
	end;

procedure RemoveStat(statId: integer);
	var
		i: integer;
	label StatDataInUse;
	begin
		with Board.Stats[statId] do begin
			if DataLen <> 0 then begin
				for i := 1 to Board.StatCount do begin
					if (Board.Stats[i].Data = Data) and (i <> statId) then
						goto StatDataInUse;
				end;
				FreeMem(Data, DataLen);
			end;

		StatDataInUse:
			if statId < CurrentStatTicked then
				CurrentStatTicked := CurrentStatTicked - 1;

			Board.Tiles[X][Y] := Under;
			if Y > 0 then
				BoardDrawTile(X, Y);

			for i := 1 to Board.StatCount do begin
				if Board.Stats[i].Follower >= statId then begin
					if Board.Stats[i].Follower = statId then
						Board.Stats[i].Follower := -1
					else
						Board.Stats[i].Follower := Board.Stats[i].Follower - 1;
				end;

				if Board.Stats[i].Leader >= statId then begin
					if Board.Stats[i].Leader = statId then
						Board.Stats[i].Leader := -1
					else
						Board.Stats[i].Leader := Board.Stats[i].Leader - 1;
				end;
			end;

			for i := (statId + 1) to Board.StatCount do
				Board.Stats[i - 1] := Board.Stats[i];
			Board.StatCount := Board.StatCount - 1;
		end;
	end;

function GetStatIdAt(x, y: integer): integer;
	var
		i: integer;
	begin
		i := -1;
		repeat
			i := i + 1;
		until ((Board.Stats[i].X = x) and (Board.Stats[i].Y = y)) or (i > Board.StatCount);

		if i > Board.StatCount then
			GetStatIdAt := -1
		else
			GetStatIdAt := i;
	end;

function BoardPrepareTileForPlacement(x, y: integer): boolean;
	var
		statId: integer;
		result: boolean;
	begin
		statId := GetStatIdAt(x, y);
		if statId > 0 then begin
			RemoveStat(statId);
			result := true;
		end else if statId < 0 then begin
			if not ElementDefs[Board.Tiles[x][y].Element].PlaceableOnTop then
				Board.Tiles[x][y].Element := E_EMPTY;
			result := true;
		end else begin { statId = 0 (player) cannot be modified }
			result := false;
		end;
		BoardDrawTile(x, y);
		BoardPrepareTileForPlacement := result;
	end;

procedure MoveStat(statId: integer; newX, newY: integer);
	var
		iUnder: TTile;
		ix, iy: integer;
		oldX, oldY: integer;
		oldBgColor: integer;
	begin
		with Board.Stats[statId] do begin
			oldBgColor := Board.Tiles[newX][newY].Color and $F0;

			iUnder := Board.Stats[statId].Under;
			Board.Stats[statId].Under := Board.Tiles[newX][newY];

			if Board.Tiles[X][Y].Element = E_PLAYER then
				Board.Tiles[newX][newY].Color := Board.Tiles[X][Y].Color
			else if Board.Tiles[newX][newY].Element = E_EMPTY then
				Board.Tiles[newX][newY].Color := Board.Tiles[X][Y].Color and $0F
			else
				Board.Tiles[newX][newY].Color := (Board.Tiles[X][Y].Color and $0F) + (Board.Tiles[newX][newY].Color and $70);

			Board.Tiles[newX][newY].Element := Board.Tiles[X][Y].Element;
			Board.Tiles[X][Y] := iUnder;

			oldX := X;
			oldY := Y;
			X := newX;
			Y := newY;

			BoardDrawTile(X, Y);
			BoardDrawTile(oldX, oldY);

			if (statId = 0) and Board.Info.IsDark and (World.Info.TorchTicks > 0) then begin
				if (Sqr(oldX-X) + Sqr(oldY-Y)) = 1 then begin
					for ix := (X - TORCH_DX - 3) to (X + TORCH_DX + 3) do
						if (ix >= 1) and (ix <= BOARD_WIDTH) then
							for iy := (Y - TORCH_DY - 3) to (Y + TORCH_DY + 3) do
								if (iy >= 1) and (iy <= BOARD_HEIGHT) then
									if (((Sqr(ix-oldX))+(Sqr(iy-oldY)*2)) < TORCH_DIST_SQR) xor
										(((Sqr(ix-newX))+(Sqr(iy-newY)*2)) < TORCH_DIST_SQR) then
										BoardDrawTile(ix, iy);
				end else begin
					DrawPlayerSurroundings(oldX, oldY, 0);
					DrawPlayerSurroundings(X, Y, 0);
				end;
			end;

		end;
	end;

procedure PopupPromptString(question: string; var buffer: TString50);
	var
		x, y: integer;
	begin
		VideoWriteText(3, 18, $4F, TextWindowStrTop);
		VideoWriteText(3, 19, $4F, TextWindowStrText);
		VideoWriteText(3, 20, $4F, TextWindowStrSep);
		VideoWriteText(3, 21, $4F, TextWindowStrText);
		VideoWriteText(3, 22, $4F, TextWindowStrText);
		VideoWriteText(3, 23, $4F, TextWindowStrBottom);
		VideoWriteText(4 + (TextWindowWidth - Length(question)) div 2, 19, $4F, question);
		buffer := '';
		PromptString(10, 22, $4F, $4E, TextWindowWidth - 16, PROMPT_ANY, buffer);
		for y := 18 to 23 do
			for x := 3 to (TextWindowWidth + 3) do
				BoardDrawTile(x + 1, y + 1);
	end;

function Signum(val: integer): integer;
	begin
		if val > 0 then
			Signum := 1
		else if val < 0 then
			Signum := -1
		else
			Signum := 0;
	end;

function Difference(a, b: integer): integer;
	begin
		if (a - b) >= 0 then
			Difference := a - b
		else
			Difference := b - a;
	end;

procedure GameUpdateSidebar;
	var
		numStr: string[8];
		i: integer;
	begin
		if GameStateElement = E_PLAYER then begin
			if Board.Info.TimeLimitSec > 0 then begin
				VideoWriteText(64, 6, $1E, '   Time:');
				Str(Board.Info.TimeLimitSec - World.Info.BoardTimeSec, numStr);
				VideoWriteText(72, 6, $1E, numStr + ' ');
			end else begin
				SidebarClearLine(6);
			end;

			if World.Info.Health < 0 then
				World.Info.Health := 0;

			Str(World.Info.Health, numStr);
			VideoWriteText(72, 7, $1E, numStr + ' ');
			Str(World.Info.Ammo, numStr);
			VideoWriteText(72, 8, $1E, numStr + '  ');
			Str(World.Info.Torches, numStr);
			VideoWriteText(72, 9, $1E, numStr + ' ');
			Str(World.Info.Gems, numStr);
			VideoWriteText(72, 10, $1E, numStr + ' ');
			Str(World.Info.Score, numStr);
			VideoWriteText(72, 11, $1E, numStr + ' ');

			if World.Info.TorchTicks = 0 then
				VideoWriteText(75, 9, $16, '    ')
			else begin
				for i := 2 to 5 do begin
					if i <= ((World.Info.TorchTicks * 5) div TORCH_DURATION) then
						VideoWriteText(73 + i, 9, $16, #177)
					else
						VideoWriteText(73 + i, 9, $16, #176);
				end;
			end;

			for i := 1 to 7 do begin
				if World.Info.Keys[i] then
					VideoWriteText(71 + i, 12, $18 + i, ElementDefs[E_KEY].Character)
				else
					VideoWriteText(71 + i, 12, $1F, ' ');
			end;

			if SoundEnabled then
				VideoWriteText(65, 15, $1F, ' Be quiet')
			else
				VideoWriteText(65, 15, $1F, ' Be noisy');

			if DebugEnabled then begin
				{ TODO: Replace with some interesting stat
				  on Linux.}
				numStr := 'lots';
				VideoWriteText(69, 4, $1E, 'm' + numStr + ' ');
			end;
		end;
	end;

procedure DisplayMessage(ticks: integer; message: string);
	begin
		if GetStatIdAt(0, 0) <> -1 then begin
			RemoveStat(GetStatIdAt(0, 0));
			BoardDrawBorder;
		end;

		if Length(message) <> 0 then begin
			AddStat(0, 0, E_MESSAGE_TIMER, 0, 1, StatTemplateDefault);
			{IMP: P2 is a byte, so it can hold a max value of 255.}
			Board.Stats[Board.StatCount].P2 := Min(255, Time div (TickTimeDuration + 1));
			Board.Info.Message := message;
		end;
	end;

procedure DamageStat(attackerStatId: integer);
	var
		oldX, oldY: integer;
	begin
		with Board.Stats[attackerStatId] do begin
			if attackerStatId = 0 then begin
				if World.Info.Health > 0 then begin
					World.Info.Health := World.Info.Health - 10;

					GameUpdateSidebar;
					DisplayMessage(100, 'Ouch!');

					Board.Tiles[X][Y].Color := $70 + (ElementDefs[4].Color mod $10);

					if World.Info.Health > 0 then begin
						World.Info.BoardTimeSec := 0;
						if Board.Info.ReenterWhenZapped then begin
							SoundQueue(4, #32#1#35#1#39#1#48#1#16#1);

							{ Move player to start }
							Board.Tiles[X][Y].Element := E_EMPTY;
							BoardDrawTile(X, Y);
							oldX := X;
							oldY := Y;
							X := Board.Info.StartPlayerX;
							Y := Board.Info.StartPlayerY;
							DrawPlayerSurroundings(oldX, oldY, 0);
							DrawPlayerSurroundings(X, Y, 0);

							GamePaused := true;
						end;
						SoundQueue(4, #16#1#32#1#19#1#35#1);
					end else begin
						SoundQueue(5, #32#3#35#3#39#3#48#3#39#3#42#3#50#3#55#3#53#3#56#3#64#3#69#3#16#10);
					end;
				end;
			end else begin
				case Board.Tiles[X][Y].Element of
					E_BULLET: SoundQueue(3, #32#1);
					E_OBJECT: begin end;
				else
					SoundQueue(3, #64#1#16#1#80#1#48#1)
				end;
				RemoveStat(attackerStatId);
			end;
		end;
	end;

procedure BoardDamageTile(x, y: integer);
	var
		statId: integer;
	begin
		statId := GetStatIdAt(x, y);
		if statId <> -1 then begin
			DamageStat(statId);
		end else begin
			Board.Tiles[x][y].Element := E_EMPTY;
			BoardDrawTile(x, y);
		end;
	end;

procedure BoardAttack(attackerStatId: integer; x, y: integer);
	begin
		if (attackerStatId = 0) and (World.Info.EnergizerTicks > 0) then begin
			World.Info.Score := ElementDefs[Board.Tiles[x][y].Element].ScoreValue + World.Info.Score;
			GameUpdateSidebar;
		end else begin
			DamageStat(attackerStatId);
		end;

		if (attackerStatId > 0) and (attackerStatId <= CurrentStatTicked) then
			CurrentStatTicked := CurrentStatTicked - 1;

		if (Board.Tiles[x][y].Element = E_PLAYER) and (World.Info.EnergizerTicks > 0) then begin
			World.Info.Score := ElementDefs[Board.Tiles[Board.Stats[attackerStatId].X][Board.Stats[attackerStatId].Y].Element]
				.ScoreValue + World.Info.Score;
			GameUpdateSidebar;
		end else begin
			BoardDamageTile(x, y);
			SoundQueue(2, #16#1);
		end;
	end;

function BoardShoot(element: byte; tx, ty, deltaX, deltaY: integer; source: integer): boolean;
	begin
		if ElementDefs[Board.Tiles[tx + deltaX][ty + deltaY].Element].Walkable
			or (Board.Tiles[tx + deltaX][ty + deltaY].Element = E_WATER) then
		begin
			AddStat(tx + deltaX, ty + deltaY, element, ElementDefs[element].Color, 1, StatTemplateDefault);
			with Board.Stats[Board.StatCount] do begin
				P1 := source;
				StepX := deltaX;
				StepY := deltaY;
				P2 := 100;
			end;
			BoardShoot := true;
		end else if (Board.Tiles[tx + deltaX][ty + deltaY].Element = E_BREAKABLE)
			or (
				ElementDefs[Board.Tiles[tx + deltaX][ty + deltaY].Element].Destructible
				and ((Board.Tiles[tx + deltaX][ty + deltaY].Element = E_PLAYER) = Boolean(source))
				and (World.Info.EnergizerTicks <= 0)
			) then
		begin
			BoardDamageTile(tx + deltaX, ty + deltaY);
			SoundQueue(2, #16#1);
			BoardShoot := true;
		end else begin
			BoardShoot := false;
		end;
	end;

procedure CalcDirectionRnd(var deltaX, deltaY: integer);
	begin
		deltaX := Random(3) - 1;

		if deltaX = 0 then
			deltaY := Random(2) * 2 - 1
		else
			deltaY := 0;
	end;

procedure CalcDirectionSeek(x, y: integer; var deltaX, deltaY: integer);
	begin
		deltaX := 0;
		deltaY := 0;

		if (Random(2) < 1) or (Board.Stats[0].Y = y) then
			deltaX := Signum(Board.Stats[0].X - x);

		if deltaX = 0 then
			deltaY := Signum(Board.Stats[0].Y - y);

		if World.Info.EnergizerTicks > 0 then begin
			deltaX := -deltaX;
			deltaY := -deltaY;
		end;
	end;

procedure TransitionDrawBoardChange;
	begin
		TransitionDrawToFill(#219, $05);
		TransitionDrawToBoard;
	end;

procedure BoardEnter;
	begin
		Board.Info.StartPlayerX := Board.Stats[0].X;
		Board.Info.StartPlayerY := Board.Stats[0].Y;

		if Board.Info.IsDark and MessageHintTorchNotShown then begin
			DisplayMessage(200, 'Room is dark - you need to light a torch!');
			MessageHintTorchNotShown := false;
		end;

		World.Info.BoardTimeSec := 0;
		GameUpdateSidebar;
	end;

procedure BoardPassageTeleport(x, y: integer);
	var
		oldBoard: integer;
		col: byte;
		ix, iy: integer;
		newX, newY: integer;
	begin
		col := Board.Tiles[x][y].Color;

		oldBoard := World.Info.CurrentBoard;
		BoardChange(Board.Stats[GetStatIdAt(x, y)].P3);

		newX := 0;
		for ix := 1 to BOARD_WIDTH do
			for iy := 1 to BOARD_HEIGHT do
				if (Board.Tiles[ix][iy].Element = E_PASSAGE) and (Board.Tiles[ix][iy].Color = col) then begin
					newX := ix;
					newY := iy;
				end;

		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element := E_EMPTY;
		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color := 0;
		if newX <> 0 then begin
			Board.Stats[0].X := newX;
			Board.Stats[0].Y := newY;
		end;

		GamePaused := true;
		SoundQueue(4, #48#1#52#1#55#1#49#1#53#1#56#1#50#1#54#1#57#1#51#1#55#1#58#1#52#1#56#1#64#1);
		TransitionDrawBoardChange;
		BoardEnter;
	end;

procedure GameDebugPrompt;
	var
		input: TString50;
		i: integer;
		toggle: boolean;
	begin
		input := '';
		SidebarClearLine(4);
		SidebarClearLine(5);

		PromptString(63, 5, $1E, $0F, 11, PROMPT_ANY, input);
		for i := 1 to Length(input) do
			input[i] := UpCase(input[i]);

		toggle := true;
		if (input[1] = '+') or (input[1] = '-') then begin
			if input[1] = '-' then
				toggle := false;
			input := Copy(input, 2, Length(input) - 1);

			if toggle = true then
				WorldSetFlag(input)
			else
				WorldClearFlag(input);
		end;

		DebugEnabled := WorldGetFlagPosition('DEBUG') >= 0;

		if input = 'HEALTH' then
			World.Info.Health := World.Info.Health + 50
		else if input = 'AMMO' then
			World.Info.Ammo := World.Info.Ammo + 5
		else if input = 'KEYS' then
			for i := 1 to 7 do World.Info.Keys[i] := true
		else if input = 'TORCHES' then
			World.Info.Torches := World.Info.Torches + 3
		else if input = 'TIME' then
			World.Info.BoardTimeSec := World.Info.BoardTimeSec - 30
		else if input = 'GEMS' then
			World.Info.Gems := World.Info.Gems + 5
		else if input = 'DARK' then begin
			Board.Info.IsDark := toggle;
			TransitionDrawToBoard;
		end else if input = 'ZAP' then begin
			for i := 0 to 3 do begin
				BoardDamageTile(Board.Stats[0].X + NeighborDeltaX[i], Board.Stats[0].Y + NeighborDeltaY[i]);
				Board.Tiles[Board.Stats[0].X + NeighborDeltaX[i]][Board.Stats[0].Y + NeighborDeltaY[i]].Element := E_EMPTY;
				BoardDrawTile(Board.Stats[0].X + NeighborDeltaX[i], Board.Stats[0].Y + NeighborDeltaY[i]);
			end;
		end;

		SoundQueue(10, #39#4);
		SidebarClearLine(4);
		SidebarClearLine(5);
		GameUpdateSidebar;
	end;

procedure GameAboutScreen;
	begin
		TextWindowDisplayFile('ABOUT.HLP', 'About ZZT...');
	end;

procedure GamePlayLoop(boardChanged: boolean);
	var
		exitLoop: boolean;
		pauseBlink: boolean;
	procedure GameDrawSidebar;
		begin
			SidebarClear;
			SidebarClearLine(0);
			SidebarClearLine(1);
			SidebarClearLine(2);
			VideoWriteText(61, 0, $1F, '    - - - - -      ');
			VideoWriteText(62, 1, $70, '      ZZT      ');
			VideoWriteText(61, 2, $1F, '    - - - - -      ');
			if GameStateElement = E_PLAYER then begin
				VideoWriteText(64, 7, $1E, ' Health:');
				VideoWriteText(64, 8, $1E, '   Ammo:');
				VideoWriteText(64, 9, $1E, 'Torches:');
				VideoWriteText(64, 10, $1E, '   Gems:');
				VideoWriteText(64, 11, $1E, '  Score:');
				VideoWriteText(64, 12, $1E, '   Keys:');
				VideoWriteText(62, 7, $1F, ElementDefs[E_PLAYER].Character);
				VideoWriteText(62, 8, $1B, ElementDefs[E_AMMO].Character);
				VideoWriteText(62, 9, $16, ElementDefs[E_TORCH].Character);
				VideoWriteText(62, 10, $1B, ElementDefs[E_GEM].Character);
				VideoWriteText(62, 12, $1F, ElementDefs[E_KEY].Character);
				VideoWriteText(62, 14, $70, ' T ');
				VideoWriteText(65, 14, $1F, ' Torch');
				VideoWriteText(62, 15, $30, ' B ');
				VideoWriteText(62, 16, $70, ' H ');
				VideoWriteText(65, 16, $1F, ' Help');
				VideoWriteText(67, 18, $30, ' '#24#25#26#27' ');
				VideoWriteText(72, 18, $1F, ' Move');
				VideoWriteText(61, 19, $70, ' Shift '#24#25#26#27' ');
				VideoWriteText(72, 19, $1F, ' Shoot');
				VideoWriteText(62, 21, $70, ' S ');
				VideoWriteText(65, 21, $1F, ' Save game');
				VideoWriteText(62, 22, $30, ' P ');
				VideoWriteText(65, 22, $1F, ' Pause');
				VideoWriteText(62, 23, $70, ' Q ');
				VideoWriteText(65, 23, $1F, ' Quit');
			end else if GameStateElement = E_MONITOR then begin
				SidebarPromptSlider(false, 66, 21, 'Game speed:;FS', TickSpeed);
				VideoWriteText(62, 21, $70, ' S ');
				VideoWriteText(62, 7, $30, ' W ');
				VideoWriteText(65, 7, $1E, ' World:');

				if Length(World.Info.Name) <> 0 then
					VideoWriteText(69, 8, $1F, World.Info.Name)
				else
					VideoWriteText(69, 8, $1F, 'Untitled');

				VideoWriteText(62, 11, $70, ' P ');
				VideoWriteText(65, 11, $1F, ' Play');
				VideoWriteText(62, 12, $30, ' R ');
				VideoWriteText(65, 12, $1E, ' Restore game');
				VideoWriteText(62, 13, $70, ' Q ');
				VideoWriteText(65, 13, $1E, ' Quit');
				VideoWriteText(62, 16, $30, ' A ');
				VideoWriteText(65, 16, $1F, ' About ZZT!');
				VideoWriteText(62, 17, $70, ' H ');
				VideoWriteText(65, 17, $1E, ' High Scores');

				if EditorEnabled then begin
					VideoWriteText(62, 18, $30, ' E ');
					VideoWriteText(65, 18, $1E, ' Board Editor');
				end;
			end;
		end;
	begin
		GameDrawSidebar;
		GameUpdateSidebar;

		if JustStarted then begin
			GameAboutScreen;
			if Length(StartupWorldFileName) <> 0 then begin
				SidebarClearLine(8);
				VideoWriteText(69, 8, $1F, StartupWorldFileName);
				if not WorldLoad(StartupWorldFileName, '.ZZT') then WorldCreate;
			end;
			ReturnBoardId := World.Info.CurrentBoard;
			BoardChange(0);
			JustStarted := false;
		end;

		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element := GameStateElement;
		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color := ElementDefs[GameStateElement].Color;

		if GameStateElement = E_MONITOR then begin
			DisplayMessage(0, '');
			VideoWriteText(62, 5, $1B, 'Pick a command:');
		end;

		if boardChanged then
			TransitionDrawBoardChange;

		TickTimeDuration := TickSpeed * 2;
		GamePlayExitRequested := false;
		exitLoop := false;

		CurrentTick := Random(100);
		CurrentStatTicked := Board.StatCount + 1;

		pauseBlink := true;

		repeat
			if GamePaused then begin
				if SoundHasTimeElapsed(TickTimeCounter, 25) then
					pauseBlink := not pauseBlink;

				if pauseBlink then begin
					VideoWriteText(Board.Stats[0].X - 1, Board.Stats[0].Y - 1,
						ElementDefs[E_PLAYER].Color, ElementDefs[E_PLAYER].Character);
				end else begin
					if Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = E_PLAYER then
						VideoWriteText(Board.Stats[0].X - 1, Board.Stats[0].Y - 1, $0F, ' ')
					else
						BoardDrawTile(Board.Stats[0].X, Board.Stats[0].Y);
				end;

				VideoWriteText(64, 5, $1F, 'Pausing...');
				InputUpdate;

				if InputKeyPressed = KEY_ESCAPE then
					GamePromptEndPlay;

				if (InputDeltaX <> 0) or (InputDeltaY <> 0) then begin
					ElementDefs[Board.Tiles[Board.Stats[0].X + InputDeltaX][Board.Stats[0].Y + InputDeltaY].Element].TouchProc(
						Board.Stats[0].X + InputDeltaX, Board.Stats[0].Y + InputDeltaY, 0, InputDeltaX, InputDeltaY);
				end;

				if ((InputDeltaX <> 0) or (InputDeltaY <> 0))
					and ElementDefs[Board.Tiles[Board.Stats[0].X + InputDeltaX][Board.Stats[0].Y + InputDeltaY].Element].Walkable
				then begin
					{ Move player }
					if Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element = E_PLAYER then
						MoveStat(0, Board.Stats[0].X + InputDeltaX, Board.Stats[0].Y + InputDeltaY)
					else begin
						BoardDrawTile(Board.Stats[0].X, Board.Stats[0].Y);
						Board.Stats[0].X := Board.Stats[0].X + InputDeltaX;
						Board.Stats[0].Y := Board.Stats[0].Y + InputDeltaY;
						Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element := E_PLAYER;
						Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color := ElementDefs[E_PLAYER].Color;
						BoardDrawTile(Board.Stats[0].X, Board.Stats[0].Y);
						DrawPlayerSurroundings(Board.Stats[0].X, Board.Stats[0].Y, 0);
						DrawPlayerSurroundings(Board.Stats[0].X - InputDeltaX, Board.Stats[0].Y - InputDeltaY, 0);
					end;

					{ Unpause }
					GamePaused := false;
					SidebarClearLine(5);
					CurrentTick := Random(100);
					CurrentStatTicked := Board.StatCount + 1;
					World.Info.IsSave := true;
				end;

			end else begin { not GamePaused }
				if CurrentStatTicked <= Board.StatCount then begin
					with Board.Stats[CurrentStatTicked] do begin
						if (Cycle <> 0) and ((CurrentTick mod Cycle) = (CurrentStatTicked mod Cycle)) then
							ElementDefs[Board.Tiles[X][Y].Element].TickProc(CurrentStatTicked);

						CurrentStatTicked := CurrentStatTicked + 1;
					end;
				end;
			end;

			if (CurrentStatTicked > Board.StatCount) and not GamePlayExitRequested then begin
				{ all stats ticked }
				if SoundHasTimeElapsed(TickTimeCounter, TickTimeDuration) then begin
					{ next cycle }
					CurrentTick := CurrentTick + 1;
					if CurrentTick > 420 then
						CurrentTick := 1;
					CurrentStatTicked := 0;

					InputUpdate;
				end;
			end;
		until (exitLoop or GamePlayExitRequested) and GamePlayExitRequested;

		SoundClearQueue;

		if GameStateElement = E_PLAYER then begin
			if World.Info.Health <= 0 then begin
				HighScoresAdd(World.Info.Score);
			end;
		end else if GameStateElement = E_MONITOR then begin
			SidebarClearLine(5);
		end;

		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Element := E_PLAYER;
		Board.Tiles[Board.Stats[0].X][Board.Stats[0].Y].Color := ElementDefs[E_PLAYER].Color;

		SoundBlockQueueing := false;
	end;

procedure GameTitleLoop;
	var
		boardChanged: boolean;
		startPlay: boolean;
	begin
		GameTitleExitRequested := false;
		JustStarted := true;
		ReturnBoardId := 0;
		boardChanged := true;
		repeat
			BoardChange(0);
			repeat
				GameStateElement := E_MONITOR;
				startPlay := false;
				GamePaused := false;
				GamePlayLoop(boardChanged);
				boardChanged := false;

				case UpCase(InputKeyPressed) of
					'W': begin
						if GameWorldLoad('.ZZT') then begin
							ReturnBoardId := World.Info.CurrentBoard;
							boardChanged := true;
						end;
					end;
					'P': begin
						if World.Info.IsSave and not DebugEnabled then begin
							startPlay := WorldLoad(World.Info.Name, '.ZZT');
							ReturnBoardId := World.Info.CurrentBoard;
						end else begin
							startPlay := true;
						end;
						if startPlay then begin
							BoardChange(ReturnBoardId);
							BoardEnter;
						end;
					end;
					'A': begin
						GameAboutScreen;
					end;
					'E': if EditorEnabled then begin
						EditorLoop;
						ReturnBoardId := World.Info.CurrentBoard;
						boardChanged := true;
					end;
					'S': begin
						SidebarPromptSlider(true, 66, 21, 'Game speed:;FS', TickSpeed);
						InputKeyPressed := #0;
					end;
					'R': begin
						if GameWorldLoad('.SAV') then begin
							ReturnBoardId := World.Info.CurrentBoard;
							BoardChange(ReturnBoardId);
							startPlay := true;
						end;
					end;
					'H': begin
						HighScoresLoad;
						HighScoresDisplay(1);
					end;
					'|': begin
						GameDebugPrompt;
					end;
					KEY_ESCAPE, 'Q': begin
						GameTitleExitRequested := SidebarPromptYesNo('Quit ZZT? ', true);
					end;
				end;

				if startPlay then begin
					GameStateElement := E_PLAYER;
					GamePaused := true;
					GamePlayLoop(true);
					boardChanged := true;
				end;
			until boardChanged or GameTitleExitRequested;
		until GameTitleExitRequested;
	end;

procedure GamePrintRegisterMessage;
	var
		s: string;
		f: file;
		i: integer;
		ix, iy: integer;
		color: integer;
		isReading: boolean;
		strPtr: pointer;
	begin
		SetCBreak(false);
		s := 'END' + Chr(49 + Random(4)) + '.MSG';
		iy := 0;
		color := $0F;

		for i := 1 to ResourceDataHeader.EntryCount do begin
			if ResourceDataHeader.Name[i] = s then begin
				Assign(f, ResourceDataFileName);
				OpenForRead(f, 1);
				Seek(f, ResourceDataHeader.FileOffset[i]);

				isReading := true;
				while (IOResult = 0) and isReading do begin
					BlockRead(f, s, 1);
					strPtr := @s;
					AdvancePointer(strPtr, 1);
					if Length(s) = 0 then begin
						Dec(color);
					end else begin
						BlockRead(f, strPtr^, Length(s));
						if s <> '@' then
							VideoWriteText(0, iy, color, s)
						else
							isReading := false;
					end;
					Inc(iy);
				end;

				Close(f);
				VideoWriteText(28, 24, $1F, 'Press any key to exit...');
				TextColor(LightGray);

				repeat until KeyPressed;
				InputKeyPressed := ReadKey;

				VideoWriteText(28, 24, $00, '                        ');
				GotoXY(1, 23);
			end;
		end;
	end;

begin
end.
