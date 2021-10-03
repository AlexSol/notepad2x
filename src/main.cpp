#include "precheader.h"
#include "scintilla.h"
#include "scilexer.h"
#include "SciCall.h"
#include "resource.h"

#include "Application.h"

//HINSTANCE g_hInstance;
//UINT16    g_uWinVer;
//WCHAR     g_wchWorkingDirectory[MAX_PATH] = L"";
//HMODULE   hModUxTheme = NULL;
//BOOL      fIsElevated = FALSE;
//WCHAR     g_wchAppUserModelID[32] = L"";
//UINT      msgTaskbarCreated = 0;
//HWND      hDlgFindReplace = NULL;
//
//int flagPasteBoard = 0;
//int flagDisplayHelp = 0;
//
//void Encoding_InitDefaults();
//void ParseCommandLine();
//int FindIniFile();
//int TestIniFile();
//int CreateIniFile();
//void LoadFlags();
//void DisplayCmdLineHelp(HWND hwnd);
//BOOL RelaunchElevated();
//BOOL RelaunchMultiInst();
//BOOL ActivatePrevInst();
//void LoadSettings();
//BOOL InitApplication(HINSTANCE hInstance);
//HWND InitInstance(HINSTANCE hInstance, LPSTR pszCmdLine, int nCmdShow);
//HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID);
//
//static WCHAR wchWndClass[16] = L"Notepad2";

extern "C" int WINAPI runNotepad2(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{

    runNotepad2(hInstance, hPrevInst, lpCmdLine, nCmdShow);

    Application::getInstance()->init();
    Application::getInstance()->run();

    return 1;
}