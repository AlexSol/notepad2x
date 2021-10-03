#include "Application.h"
#include <commctrl.h>
#include <commdlg.h>
#include <shlwapi.h>

#include "scintilla.h"
#include "scilexer.h"
#include "resource.h"

static UINT16 uWinVer;
extern "C" UINT16 g_uWinVer = 0;

extern "C" HINSTANCE g_hInstance = Application::getInstance()->getInstanceWin();

extern "C"  BOOL fIsElevated = FALSE;
extern "C"  UINT msgTaskbarCreated = 0;
extern "C"  HMODULE hModUxTheme = NULL;

extern "C" WCHAR wchWndClass[16] = L"Notepad2";
extern "C" HWND      hDlgFindReplace = NULL;

extern "C" WCHAR     g_wchAppUserModelID[32] = L"";
extern "C" WCHAR     g_wchWorkingDirectory[MAX_PATH] = L"";

extern "C" LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
extern "C" void LoadSettings();
extern "C" void Encoding_InitDefaults();
extern "C" void ParseCommandLine();
extern "C" void LoadFlags();
extern "C" int  CreateIniFile();
extern "C" int FindIniFile();
extern "C" int TestIniFile();
extern "C" HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCWSTR AppID);
extern "C" void DisplayCmdLineHelp(HWND hwnd);
extern "C" BOOL RelaunchElevated();
extern "C" BOOL RelaunchMultiInst();
extern "C" BOOL ActivatePrevInst();
extern "C" HWND InitInstance(HINSTANCE, LPSTR, int);

//=============================================================================
//
// Flags
//
extern "C" int flagPasteBoard = 0;
extern "C" int flagDisplayHelp = 0;

Application::Application()
{
    _instance = GetModuleHandle(nullptr);
    g_hInstance = _instance;
}

Application::~Application()
{
    // Save Settings is done elsewhere

    Scintilla_ReleaseResources();
    UnregisterClass(wchWndClass, getInstanceWin());

    if (hModUxTheme)
        FreeLibrary(hModUxTheme);

    OleUninitialize();
}

int Application::exec()
{
    return init();
}

int Application::init()
{
    WCHAR wchWorkingDirectory[MAX_PATH];
    uWinVer = LOWORD(GetVersion());
    uWinVer = MAKEWORD(HIBYTE(uWinVer), LOBYTE(uWinVer));
    g_uWinVer = uWinVer;

    // Don't keep working directory locked
    GetCurrentDirectory(MAX_PATH, g_wchWorkingDirectory);
    GetModuleFileName(NULL, wchWorkingDirectory, MAX_PATH);
    PathRemoveFileSpec(wchWorkingDirectory);
    SetCurrentDirectory(wchWorkingDirectory);

    // Check if running with elevated privileges
    elevated();
    fIsElevated = IsElevated();

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    // Default Encodings (may already be used for command line parsing)
    Encoding_InitDefaults();

    // Command Line, Ini File and Flags
    ParseCommandLine();
    FindIniFile();
    TestIniFile();
    CreateIniFile();
    LoadFlags();

    // set AppUserModelID
    PrivateSetCurrentProcessExplicitAppUserModelID(g_wchAppUserModelID);

    // Command Line Help Dialog
    if (flagDisplayHelp) {
        DisplayCmdLineHelp(NULL);
        return -1;
    }

    // Adapt window class name
    if (IsElevated())
        StrCat(wchWndClass, L"U");
    if (flagPasteBoard)
        StrCat(wchWndClass, L"B");

    // Relaunch with elevated privileges
    if (RelaunchElevated())
        return -1;

    // Try to run multiple instances
    if (RelaunchMultiInst())
        return -1;

    // Try to activate another window
    if (ActivatePrevInst())
        return -1;

    // Init OLE and Common Controls
    OleInitialize(NULL);

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icex);

    msgTaskbarCreated = RegisterWindowMessage(L"TaskbarCreated");
    hModUxTheme = LoadLibrary(L"uxtheme.dll");

    Scintilla_RegisterClasses(getInstanceWin());

    // Load Settings
    LoadSettings();

    if (!InitApplication(getInstanceWin())) {
        return -1;
    }

    HWND hwnd;
    //if (!(hwnd = InitInstance(hInstance, lpCmdLine, nCmdShow)))
    if (!(hwnd = InitInstance(getInstanceWin(), (char*)"", 1))) {
        return -1;
    }

    HACCEL hAccMain = LoadAccelerators(getInstanceWin(), MAKEINTRESOURCE(IDR_MAINWND));
    HACCEL hAccFindReplace = LoadAccelerators(getInstanceWin(), MAKEINTRESOURCE(IDR_ACCFINDREPLACE));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (IsWindow(hDlgFindReplace) && (msg.hwnd == hDlgFindReplace || IsChild(hDlgFindReplace, msg.hwnd)))
            if (TranslateAccelerator(hDlgFindReplace, hAccFindReplace, &msg) || IsDialogMessage(hDlgFindReplace, &msg))
                continue;

        if (!TranslateAccelerator(hwnd, hAccMain, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return msg.wParam;
}

Application* Application::getInstance()
{
    static Application appInstance;
    return &appInstance;
}

HINSTANCE Application::getInstanceWin()
{
    return _instance;
}

bool Application::IsElevated()
{
    return _isElevated;
}

void Application::elevated()
{
    _isElevated = false;

    if (!isVista()) {
        return;
    }

    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        struct {
            DWORD TokenIsElevated;
        } /*TOKEN_ELEVATION*/te;
        DWORD dwReturnLength = 0;

        if (GetTokenInformation(hToken, TokenElevation, &te, sizeof(te), &dwReturnLength)) {
            if (dwReturnLength == sizeof(te)) {
                _isElevated = te.TokenIsElevated;
            }
        }
        CloseHandle(hToken);
    }
    return;
}

bool isVista()
{
    return uWinVer >= 0x0600;
}

//=============================================================================
//
//  InitApplication()
//
//

bool InitApplication(HINSTANCE hInstance)
{
    WNDCLASS   wc;

    wc.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAINWND));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAINWND);
    wc.lpszClassName = wchWndClass;

    return RegisterClass(&wc);
}