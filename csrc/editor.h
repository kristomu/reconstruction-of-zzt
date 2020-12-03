#ifndef __editor_h__
#define __editor_h__

#include "gamevars.h"
#include "txtwind.h"

        void EditorLoop();
        void HighScoresLoad();
        void HighScoresSave();
        void HighScoresDisplay(integer linePos);
        void EditorOpenEditTextWindow(TTextWindowState& state);
        void EditorEditHelpFile();
        void HighScoresAdd(integer score);
        TString50 EditorGetBoardName(integer boardId, boolean titleScreenIsNone);
        integer EditorSelectBoard(string title, integer currentBoard, boolean titleScreenIsNone);

#endif
