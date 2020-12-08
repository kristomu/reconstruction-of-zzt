#ifndef __game_h__
#define __game_h__

#include "gamevars.h"
#include "txtwind.h"

const integer PROMPT_NUMERIC = 0;
const integer PROMPT_ALPHANUM = 1;
const integer PROMPT_ANY = 2;

/* Sanity checks. */
boolean ValidCoord(integer x, integer y);
boolean CoordInsideViewport(integer x, integer y);

void SidebarClearLine(integer y);
void SidebarClear();
void GenerateTransitionTable();
void AdvancePointer(pointer& address, integer count);
void BoardClose(boolean showTruncationNote);
void BoardOpen(integer boardId, boolean worldIsDamaged);
void BoardChange(integer boardId);
void BoardCreate();
void WorldCreate();
void TransitionDrawToFill(char chr_, integer color);
void BoardDrawTile(integer x, integer y);
void BoardDrawBorder();
void TransitionDrawToBoard();
void SidebarPromptCharacter(boolean editable, integer x, integer y,
                            TString50 prompt, byte& value);
void SidebarPromptSlider(boolean editable, integer x, integer y,
                         string prompt, byte& value);
void SidebarPromptChoice(boolean editable, integer y, string prompt,
                         string choiceStr, byte& result);
void SidebarPromptDirection(boolean editable, integer y, string prompt,
                            integer& deltaX, integer& deltaY);
void PromptString(integer x, integer y, integer arrowColor, integer color,
                  integer width, byte mode, TString50& buffer);
boolean SidebarPromptYesNo(string message, boolean defaultReturn);
void SidebarPromptString(string prompt, TString50 extension,
                         string& filename, byte promptMode);
void PauseOnError();
boolean DisplayIOError();
void DisplayTruncationNote();
void DisplayCorruptionNote();
void WorldUnload();
boolean WorldLoad(TString50 filename, TString50 extension);
void WorldSave(TString50 filename, TString50 extension);
void GameWorldSave(TString50 prompt, TString50& filename,
                   TString50 extension);
boolean GameWorldLoad(TString50 extension);
void CopyStatDataToTextWindow(integer statId, TTextWindowState& state);
void AddStat(integer tx, integer ty, byte element, integer color,
             integer tcycle, TStat template_);
void RemoveStat(integer statId);
integer GetStatIdAt(integer x, integer y);
boolean BoardPrepareTileForPlacement(integer x, integer y);
void MoveStat(integer statId, integer newX, integer newY);
void PopupPromptString(string question, TString50& buffer);
integer Signum(integer val);
integer Difference(integer a, integer b);
void DamageStat(integer attackerStatId);
void BoardDamageTile(integer x, integer y);
void BoardAttack(integer attackerStatId, integer x, integer y);
boolean BoardShoot(byte element, integer tx, integer ty, integer deltaX,
                   integer deltaY, integer source);
void CalcDirectionRnd(integer& deltaX, integer& deltaY);
void CalcDirectionSeek(integer x, integer y, integer& deltaX,
                       integer& deltaY);
void TransitionDrawBoardChange();
void GameUpdateSidebar();
void GameAboutScreen();
void GamePlayLoop(boolean boardChanged);
void DisplayMessage(integer time, string message);
void BoardEnter();
void BoardPassageTeleport(integer x, integer y);
void GameDebugPrompt();
void GameTitleLoop();
void GamePrintRegisterMessage();
const array<0, 7,byte> ProgressAnimColors = {{0x14, 0x1c, 0x15, 0x1d, 0x16, 0x1e, 0x17, 0x1f}};
const array<0, 7,asciiz> ProgressAnimStrings =
{{"....|", "...*/", "..*.-", ".*..\\", "*...|", "..../", "....-", "....\\"}};
const array<1, 7,asciiz> ColorNames =
{{"Blue", "Green", "Cyan", "Red", "Purple", "Yellow", "White"}};
/**/
const array<0, 7,integer> DiagonalDeltaX = {{-1, 0, 1, 1, 1, 0, -1, -1}};
const array<0, 7,integer> DiagonalDeltaY = {{1, 1, 1, 0, -1, -1, -1, 0}};
const array<0, 3,integer> NeighborDeltaX = {{0, 0, -1, 1}};
const array<0, 3,integer> NeighborDeltaY = {{-1, 1, 0, 0}};
/**/
const TTile TileBorder = {E_NORMAL, 0xe};
const TTile TileBoardEdge = {E_BOARD_EDGE, 0};
const TStat StatTemplateDefault = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	-1, -1
};
const varying_string<16> LineChars =
    "\371\320\322\272\265\274\273\271\306\310\311\314\315\312\313\316";

#endif
