/******************************************************************************
*
*
* Notepad2
*
* Notepad2.h
*   Global definitions and declarations
*
* See Readme.txt for more information about this source code.
* Please send me your comments to this work.
*
* See License.txt for details about distribution and modification.
*
*                                              (c) Florian Balmer 1996-2011
*                                                  florian.balmer@gmail.com
*                                               http://www.flos-freeware.ch
*
*
******************************************************************************/



//==== Main Window ============================================================
#define WC_NOTEPAD2 L"Notepad2"


//==== Data Type for WM_COPYDATA ==============================================
#define DATA_NOTEPAD2_PARAMS 0xFB10
typedef struct np2params {

  int   flagFileSpecified;
  int   ChangeNotify;
  int   LexerSpecified;
  int   iInitialLexer;
  int   QuietCreate;
  int   JumpTo;
  int   iInitialLine;
  int   iInitialColumn;
  int   iSrcEncoding;
  int   SetEncoding;
  int   SetEOLMode;
  int   flagTitleExcerpt;
  WCHAR wchData;

} NP2PARAMS, *LPNP2PARAMS;


typedef struct {
    BOOL bSaveSettings;
    BOOL bSaveRecentFiles;
    BOOL bSaveFindReplace;

    int     iPathNameFormat;
    BOOL    fWordWrapG;
    int     iWordWrapMode;
    int     iWordWrapIndent;
    int     iWordWrapSymbols;
    BOOL    bShowWordWrapSymbols;
    BOOL    bMatchBraces;
    BOOL    bAutoCloseTags;
    BOOL    bHiliteCurrentLine;
    BOOL    bAutoIndent;
    BOOL    bAutoCompleteWords;
    BOOL    bShowIndentGuides;
    BOOL    bTabsAsSpacesG;
    BOOL    bBackspaceUnindents;
    BOOL    bTabIndentsG;
    int     iTabWidthG;
    int     iIndentWidthG;
    BOOL    bMarkLongLines;
    int     iLongLinesLimitG;
    int     iLongLineMode;
    BOOL    bShowSelectionMargin;
    BOOL    bShowLineNumbers;
    BOOL    bShowCodeFolding;
    int     iMarkOccurrences;
    BOOL    bMarkOccurrencesMatchCase;
    BOOL    bMarkOccurrencesMatchWords;
    BOOL    bViewWhiteSpace;
    BOOL    bViewEOLs;
    int     iDefaultEncoding;
    BOOL    bSkipUnicodeDetection;
    BOOL    bLoadASCIIasUTF8;
    BOOL    bLoadNFOasOEM;
    BOOL    bNoEncodingTags;
    int     iDefaultEOLMode;
    BOOL    bFixLineEndings;
    BOOL    bAutoStripBlanks;
    int     iPrintHeader;
    int     iPrintFooter;
    int     iPrintColor;
    int     iPrintZoom;
    RECT    pagesetupMargin;
    BOOL    bSaveBeforeRunningTools;
    int     iFileWatchingMode;
    BOOL    bResetFileWatching;
    int     iEscFunction;
    BOOL    bAlwaysOnTop;
    BOOL    bMinimizeToTray;
    BOOL    bTransparentMode;
    WCHAR   tchToolbarButtons[512];
    BOOL    bShowToolbar;
    BOOL    bShowStatusbar;

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
} T_Settings;



//==== Toolbar Style ==========================================================
#define WS_TOOLBAR (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | \
                    TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_ALTDRAG | \
                    TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | \
                    CCS_ADJUSTABLE)


//==== ReBar Style ============================================================
#define WS_REBAR (WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | RBS_VARHEIGHT | \
                  RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN)


//==== Ids ====================================================================
#define IDC_STATUSBAR    0xFB00
#define IDC_TOOLBAR      0xFB01
#define IDC_REBAR        0xFB02
#define IDC_EDIT         0xFB03
#define IDC_EDITFRAME    0xFB04
#define IDC_FILENAME     0xFB05
#define IDC_REUSELOCK    0xFB06


//==== Statusbar ==============================================================
#define STATUS_DOCPOS    0
#define STATUS_DOCSIZE   1
#define STATUS_CODEPAGE  2
#define STATUS_EOLMODE   3
#define STATUS_OVRMODE   4
#define STATUS_LEXER     5
#define STATUS_HELP    255


//==== Change Notifications ===================================================
#define ID_WATCHTIMER 0xA000
#define WM_CHANGENOTIFY WM_USER+1
//#define WM_CHANGENOTIFYCLEAR WM_USER+2


//==== Callback Message from System Tray ======================================
#define WM_TRAYMESSAGE WM_USER


//==== Paste Board Timer ======================================================
#define ID_PASTEBOARDTIMER 0xA001


//==== Reuse Window Lock Timeout ==============================================
#define REUSEWINDOWLOCKTIMEOUT 1000


//==== Function Declarations ==================================================
HWND InitInstance(HINSTANCE,LPSTR,int);
BOOL ActivatePrevInst();
BOOL RelaunchMultiInst();
BOOL RelaunchElevated();
void SnapToDefaultPos(HWND);
void ShowNotifyIcon(HWND,BOOL);
void SetNotifyIconTitle(HWND);
void InstallFileWatching(LPCWSTR);
void CALLBACK WatchTimerProc(HWND,UINT,UINT_PTR,DWORD);
void CALLBACK PasteBoardTimer(HWND,UINT,UINT_PTR,DWORD);


void LoadSettings();
void SaveSettings(BOOL);
void ParseCommandLine();
void LoadFlags();
int  CheckIniFile(LPWSTR,LPCWSTR);
int  CheckIniFileRedirect(LPWSTR,LPCWSTR);
int  FindIniFile();
int  TestIniFile();
int  CreateIniFile();
int  CreateIniFileEx(LPCWSTR);


void UpdateStatusbar();
void UpdateToolbar();
void UpdateLineNumberWidth();


BOOL FileIO(BOOL,LPCWSTR,BOOL,int*,int*,BOOL*,BOOL*,BOOL*,BOOL);
BOOL FileLoad(BOOL,BOOL,BOOL,BOOL,LPCWSTR);
BOOL FileSave(BOOL,BOOL,BOOL,BOOL);
BOOL OpenFileDlg(HWND,LPWSTR,int,LPCWSTR);
BOOL SaveFileDlg(HWND,LPWSTR,int,LPCWSTR);


LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT MsgCreate(HWND,WPARAM,LPARAM);
void    CreateBars(HWND,HINSTANCE);
void    MsgThemeChanged(HWND,WPARAM,LPARAM);
void    MsgSize(HWND,WPARAM,LPARAM);
void    MsgInitMenu(HWND,WPARAM,LPARAM);
LRESULT MsgCommand(HWND,WPARAM,LPARAM);
LRESULT MsgNotify(HWND,WPARAM,LPARAM);



///   End of Notepad2.h   \\\
