#pragma once

//=============================================================================
//
// Flags
//
typedef struct {
    int NoReuseWindow;
    int ReuseWindow;
    int MultiFileArg;
    int SingleFileInstance;
    int StartAsTrayIcon;
    int AlwaysOnTop;
    int RelativeFileMRU;
    int PortableMyDocs;
    int NoFadeHidden;
    int ToolbarLook;
    int SimpleIndentGuides;
    int NoHTMLGuess;
    int NoCGIGuess;
    int NoFileVariables;
    int PosParam;
    int DefaultPos;
    int NewFromClipboard;
    int PasteBoard;
    int SetEncoding;
    int SetEOLMode;
    int JumpTo;
    int MatchText;
    int ChangeNotify;
    int LexerSpecified;
    int QuietCreate;
    int UseSystemMRU;
    int RelaunchElevated;
    int DisplayHelp;
} T_FLAG;
