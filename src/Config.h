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
    }T_FLAG;


typedef   struct {
        BOOL SaveSettings;
        BOOL SaveRecentFiles;
        BOOL SaveFindReplace;

        int     PathNameFormat;
        BOOL    WordWrapG;
        int     WordWrapMode;
        int     iWordWrapIndent;
        int     iWordWrapSymbols;
        BOOL    ShowWordWrapSymbols;
        BOOL    MatchBraces;
        BOOL    AutoCloseTags;
        BOOL    HiliteCurrentLine;
        BOOL    AutoIndent;
        BOOL    AutoCompleteWords;
        BOOL    ShowIndentGuides;
        BOOL    TabsAsSpacesG;
        BOOL    BackspaceUnindents;
        BOOL    TabIndentsG;
        int     TabWidthG;
        int     IndentWidthG;
        BOOL    MarkLongLines;
        int     LongLinesLimitG;
        int     LongLineMode;
        BOOL    ShowSelectionMargin;
        BOOL    ShowLineNumbers;
        BOOL    ShowCodeFolding;
        int     MarkOccurrences;
        BOOL    MarkOccurrencesMatchCase;
        BOOL    MarkOccurrencesMatchWords;
        BOOL    ViewWhiteSpace;
        BOOL    ViewEOLs;
        int     DefaultEncoding;
        BOOL    SkipUnicodeDetection;
        BOOL    LoadASCIIasUTF8;
        BOOL    LoadNFOasOEM;
        BOOL    NoEncodingTags;
        int     DefaultEOLMode;
        BOOL    FixLineEndings;
        BOOL    AutoStripBlanks;
        int     PrintHeader;
        int     PrintFooter;
        int     PrintColor;
        int     PrintZoom;
        RECT    pagesetupMargin;
        BOOL    SaveBeforeRunningTools;
        int     FileWatchingMode;
        BOOL    ResetFileWatching;
        int     EscFunction;
        BOOL    AlwaysOnTop;
        BOOL    MinimizeToTray;
        BOOL    TransparentMode;
        WCHAR   tchToolbarButtons[512];
        BOOL    ShowToolbar;
        BOOL    ShowStatusbar;

        int     cxEncodingDlg;
        int     cyEncodingDlg;
        int     cxRecodeDlg;
        int     cyRecodeDlg;
        int     cxFileMRUDlg;
        int     cyFileMRUDlg;
        int     cxOpenWithDlg;
        int     cyOpenWithDlg;
        int     cxFavoritesDlg;
        int     cyFavoritesDlg;
        int     xFindReplaceDlg;
        int     yFindReplaceDlg;
    }T_Settings;

//typedef struct T_Settings T_Settings;