#include "Settings.h"
#include "precheader.h"

#include "Notepad2.h"
#include "Config.h"

extern "C" void ExpandEnvironmentStringsEx(LPWSTR lpSrc, DWORD dwSrc);
extern "C" int  Encoding_MapIniSetting(BOOL, int);
extern "C" void PathRelativeToApp(LPWSTR, LPWSTR, int, BOOL, BOOL, BOOL);

typedef struct _editfindreplace
{
    char szFind[512];
    char szReplace[512];
    char szFindUTF8[3 * 512];
    char szReplaceUTF8[3 * 512];
    UINT fuFlags;
    BOOL bTransformBS;
    BOOL bObsolete /* was bFindUp */;
    BOOL bFindClose;
    BOOL bReplaceClose;
    BOOL bNoFindWrap;
    HWND hwnd;
#ifdef BOOKMARK_EDITION
    BOOL bWildcardSearch;
#endif
    //HANDLE hMRUFind;
    //HANDLE hMRUReplace;

} EDITFINDREPLACE, * LPEDITFINDREPLACE, * LPCEDITFINDREPLACE;

typedef struct _wi
{
    int x;
    int y;
    int cx;
    int cy;
    int max;
} WININFO;
extern "C" WININFO wi;
extern "C" HWND  hwndMain;

extern "C"  T_Settings SETTINGS;
extern "C" WCHAR tchOpenWithDir[MAX_PATH];
extern "C" WCHAR tchFavoritesDir[MAX_PATH];
extern "C" EDITFINDREPLACE efrData;
extern "C" T_FLAG FLAG;

static auto fileNameSettings = L"Notepad2.ini";
static auto fileNameSettings_JSON = L"Notepad2x.json";

// encoding function
std::string to_utf8(std::wstring& wide_string)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(wide_string);
}

void Settings::load()
{
    //saveSettings(true);
    //findIniFile();
    //testIniFile();
    //createIniFile();
    loadSettings();
}

void Settings::loadFlags()
{
    using json = nlohmann::json;
    std::ifstream iFile(fileNameSettings_JSON);

    json jsonfile;;
    iFile >> jsonfile;
}

bool Settings::findIniFile()
{
    int bFound = 0;
    std::wstring tchTest;
    std::wstring tchModule;
    tchTest.resize(MAX_PATH);
    tchModule.resize(MAX_PATH);

    GetModuleFileName(NULL, tchModule.data(), MAX_PATH);

    if (!_iniFile.empty()) {
        if (lstrcmpi(_iniFile.c_str(), L"*?") == 0)
            return false;
        else {
            if (!checkIniFile(_iniFile, tchModule)) {
                ExpandEnvironmentStringsEx(_iniFile.data(), MAX_PATH);
                if (PathIsRelative(_iniFile.data())) {
                    lstrcpy(tchTest.data(), tchModule.data());
                    PathRemoveFileSpec(tchTest.data());
                    PathAppend(tchTest.data(), _iniFile.data());
                    lstrcpy(_iniFile.data(), tchTest.data());
                }
            }
        }
        return true;
    }

    lstrcpy(tchTest.data(), PathFindFileName(tchModule.data()));
    PathRenameExtension(tchTest.data(), L".ini");
    bFound = checkIniFile(tchTest, tchModule);

    if (!bFound) {
        lstrcpy(tchTest.data(), fileNameSettings);
        bFound = checkIniFile(tchTest, tchModule);
    }

    if (bFound) {
        // allow two redirections: administrator -> user -> custom
        if (checkIniFileRedirect(tchTest, tchModule))
            checkIniFileRedirect(tchTest, tchModule);
        _iniFile = tchTest;
    }
    else {
        lstrcpy(_iniFile.data(), tchModule.data());
        PathRenameExtension(_iniFile.data(), L".ini");
    }

    return true;
}

bool Settings::testIniFile()
{
    _iniFile2.resize(MAX_PATH);
    if (lstrcmpi(_iniFile.c_str(), L"*?") == 0) {
        lstrcpy(_iniFile2.data(), L"");
        lstrcpy(_iniFile.data(), L"");
        return false;
    }

    if (PathIsDirectory(_iniFile.data()) || *CharPrev(_iniFile.data(), &_iniFile[_iniFile.size()]) == L'\\') {
        std::wstring module;
        module.resize(MAX_PATH);
        GetModuleFileName(NULL, module.data(), module.size());
        PathAppend(_iniFile.data(), PathFindFileName(module.data()));
        PathRenameExtension(_iniFile.data(), L".ini");
        if (!PathFileExists(_iniFile.data())) {
            lstrcpy(PathFindFileName(_iniFile.data()), L"Notepad2.ini");
            if (!PathFileExists(_iniFile.data())) {
                lstrcpy(PathFindFileName(_iniFile.data()), PathFindFileName(module.data()));
                PathRenameExtension(_iniFile.data(), L".ini");
            }
        }
    }

    if (!PathFileExists(_iniFile.data()) || PathIsDirectory(_iniFile.data())) {
        lstrcpy(_iniFile2.data(), _iniFile.data());
        lstrcpy(_iniFile.data(), L"");
        return false;
    }
    else
        return true;
}

bool Settings::createIniFile()
{
    return createIniFileEx(_iniFile);
}

bool Settings::createIniFileEx(const std::wstring& file)
{
    if (file.size()) {
        WCHAR* pwchTail;

        if (pwchTail = StrRChrW(file.data(), NULL, L'\\')) {
            *pwchTail = 0;
            SHCreateDirectoryEx(NULL, file.data(), NULL);
            *pwchTail = L'\\';
        }

        HANDLE hFile = CreateFile(file.data(),
                GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            //dwLastIOError = GetLastError(); // TOD ??
        if (hFile != INVALID_HANDLE_VALUE) {
            if (GetFileSize(hFile, NULL) == 0) {
                DWORD dw;
                WriteFile(hFile, (LPCVOID)L"\xFEFF[Notepad2]\r\n", 26, &dw, NULL);
            }
            CloseHandle(hFile);
            return true;
        }
    }
    return false;
}

bool Settings::loadSettings()
{
    return false;
}

bool Settings::saveSettings(bool saveSettingsNow)
{
    createIniFile();

    if (!SETTINGS.SaveSettings && !saveSettingsNow) {
        //IniSetInt(L"Settings", L"SaveSettings", TEG_Settings.bSaveSettings);
        return true;
    }

    using json = nlohmann::json;

    json jsonfile;
    
    json settings;
    settings["Settings"]["SaveSettings"] = SETTINGS.SaveSettings;
    settings["Settings"]["SaveRecentFiles"] = SETTINGS.SaveRecentFiles;
    settings["Settings"]["SaveFindReplace"] = SETTINGS.SaveFindReplace;
    
    settings["Settings"]["CloseFind"] = efrData.bFindClose;
    settings["Settings"]["CloseReplace"] = efrData.bReplaceClose;
    settings["Settings"]["NoFindWrap"] = efrData.bNoFindWrap;

    {
        WCHAR wchTmp[MAX_PATH];
        PathRelativeToApp(tchOpenWithDir, wchTmp, MAX_PATH, FALSE, TRUE, FLAG.PortableMyDocs);
        auto ws = std::wstring(wchTmp);
        settings["Settings"]["OpenWithDir"] = to_utf8(ws);
    }
    {
        WCHAR wchTmp[MAX_PATH];
        PathRelativeToApp(tchFavoritesDir, wchTmp, MAX_PATH, FALSE, TRUE, FLAG.PortableMyDocs);
        auto ws = std::wstring(wchTmp);
        settings["Settings"]["Favorites"] = to_utf8(ws);
    }

    settings["Settings"]["PathNameFormat"] = SETTINGS.PathNameFormat;
    settings["Settings"]["WordWrap"] = SETTINGS.WordWrapG;
    settings["Settings"]["WordWrapMode"] = SETTINGS.WordWrapMode;
    settings["Settings"]["WordWrapIndent"] = SETTINGS.iWordWrapIndent;
    settings["Settings"]["WordWrapSymbols"] = SETTINGS.iWordWrapSymbols;
    settings["Settings"]["ShowWordWrapSymbols"] = SETTINGS.ShowWordWrapSymbols;
    settings["Settings"]["MatchBraces"] = SETTINGS.MatchBraces;
    settings["Settings"]["AutoCloseTags"] = SETTINGS.AutoCloseTags;
    settings["Settings"]["HighlightCurrentLine"] = SETTINGS.HiliteCurrentLine;
    settings["Settings"]["AutoIndent"] = SETTINGS.AutoIndent;
    settings["Settings"]["AutoCompleteWords"] = SETTINGS.AutoCompleteWords;
    settings["Settings"]["ShowIndentGuides"] = SETTINGS.ShowIndentGuides;
    settings["Settings"]["TabsAsSpaces"] = SETTINGS.TabsAsSpacesG;
    settings["Settings"]["TabIndents"] = SETTINGS.TabIndentsG;
    settings["Settings"]["BackspaceUnindents"] = SETTINGS.BackspaceUnindents;
    settings["Settings"]["TabWidth"] = SETTINGS.TabWidthG;
    settings["Settings"]["IndentWidth"] = SETTINGS.IndentWidthG;
    settings["Settings"]["MarkLongLines"] = SETTINGS.MarkLongLines;
    settings["Settings"]["LongLinesLimit"] = SETTINGS.LongLinesLimitG;
    settings["Settings"]["LongLineMode"] = SETTINGS.LongLineMode;
    settings["Settings"]["ShowSelectionMargin"] = SETTINGS.ShowSelectionMargin;
    settings["Settings"]["ShowLineNumbers"] = SETTINGS.ShowLineNumbers;
    settings["Settings"]["ShowCodeFolding"] = SETTINGS.ShowCodeFolding;
    settings["Settings"]["MarkOccurrences"] = SETTINGS.MarkOccurrences;
    settings["Settings"]["MarkOccurrencesMatchCase"] = SETTINGS.MarkOccurrencesMatchCase;
    settings["Settings"]["MarkOccurrencesMatchWholeWords"] = SETTINGS.MarkOccurrencesMatchWords;
    settings["Settings"]["ViewWhiteSpace"] = SETTINGS.ViewWhiteSpace;
    settings["Settings"]["ViewEOLs"] = SETTINGS.ViewEOLs;
    settings["Settings"]["DefaultEncoding"] = Encoding_MapIniSetting(FALSE, SETTINGS.DefaultEncoding);
    settings["Settings"]["SkipUnicodeDetection"] = SETTINGS.SkipUnicodeDetection;
    settings["Settings"]["LoadASCIIasUTF8"] = SETTINGS.LoadASCIIasUTF8;
    settings["Settings"]["LoadNFOasOEM"] = SETTINGS.LoadNFOasOEM;
    settings["Settings"]["NoEncodingTags"] = SETTINGS.NoEncodingTags;
    settings["Settings"]["DefaultEOLMode"] = SETTINGS.DefaultEOLMode;
    settings["Settings"]["FixLineEndings"] = SETTINGS.FixLineEndings;
    settings["Settings"]["FixTrailingBlanks"] = SETTINGS.AutoStripBlanks;
    settings["Settings"]["PrintHeader"] = SETTINGS.PrintHeader;
    settings["Settings"]["PrintFooter"] = SETTINGS.PrintFooter;
    settings["Settings"]["PrintColorMode"] = SETTINGS.PrintColor;
    settings["Settings"]["PrintZoom"] = SETTINGS.PrintZoom + 10;
    settings["Settings"]["PrintMargin"] = {
        {"left", SETTINGS.pagesetupMargin.left},
        {"top", SETTINGS.pagesetupMargin.top},
        {"right", SETTINGS.pagesetupMargin.right},
        {"bottom", SETTINGS.pagesetupMargin.bottom},
    };
    settings["Settings"]["SaveBeforeRunningTools"] = SETTINGS.SaveBeforeRunningTools;
    settings["Settings"]["FileWatchingMode"] = SETTINGS.FileWatchingMode;
    settings["Settings"]["ResetFileWatching"] = SETTINGS.ResetFileWatching;
    settings["Settings"]["EscFunction"] = SETTINGS.EscFunction;
    settings["Settings"]["AlwaysOnTop"] = SETTINGS.AlwaysOnTop;
    settings["Settings"]["MinimizeToTray"] = SETTINGS.MinimizeToTray;
    settings["Settings"]["TransparentMode"] = SETTINGS.TransparentMode;

    auto ws = std::wstring(SETTINGS.tchToolbarButtons);
    settings["Settings"]["ToolbarButtons"] = to_utf8(ws);

    settings["Settings"]["ShowToolbar"] = SETTINGS.ShowToolbar;
    settings["Settings"]["ShowStatusbar"] = SETTINGS.ShowStatusbar;
    settings["Settings"]["EncodingDlgSize"] = { {"x", SETTINGS.cxEncodingDlg }, {"y", SETTINGS.cyEncodingDlg } };
    settings["Settings"]["RecodeDlgSize"] = { {"x", SETTINGS.cxRecodeDlg }, {"y", SETTINGS.cyRecodeDlg } };
    settings["Settings"]["FileMRUDlgSize"] = { {"x", SETTINGS.cxFileMRUDlg }, {"y", SETTINGS.cyFileMRUDlg } };
    settings["Settings"]["OpenWithDlgSize"] = { {"x", SETTINGS.cxOpenWithDlg }, {"y", SETTINGS.cyOpenWithDlg } };
    settings["Settings"]["FavoritesDlgSize"] = { {"x", SETTINGS.cxFavoritesDlg }, {"y", SETTINGS.cyFavoritesDlg } };
    settings["Settings"]["FindReplaceDlgPos"] = { {"x", SETTINGS.xFindReplaceDlg }, {"y", SETTINGS.yFindReplaceDlg } };

    //jsonfile["Settings2"] = "bar";
    //jsonfile["Recent Files"] = "bar";
    //jsonfile["Recent Find"] = "bar";
    //jsonfile["Recent Replace"] = "bar";
    //jsonfile["Window"] = "bar";
    //jsonfile["Custom Colors"] = "bar";

    if (saveSettingsNow){
        WINDOWPLACEMENT wndpl;

        //GetWindowPlacement
        wndpl.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwndMain, &wndpl);

        wi.x = wndpl.rcNormalPosition.left;
        wi.y = wndpl.rcNormalPosition.top;
        wi.cx = wndpl.rcNormalPosition.right - wndpl.rcNormalPosition.left;
        wi.cy = wndpl.rcNormalPosition.bottom - wndpl.rcNormalPosition.top;
        wi.max = (IsZoomed(hwndMain) || (wndpl.flags & WPF_RESTORETOMAXIMIZED));
    }
        
    if(settings["Settings2"]["StickyWindowPosition"].is_null()){
        const int ResX = GetSystemMetrics(SM_CXSCREEN);
        const int ResY = GetSystemMetrics(SM_CYSCREEN);

        settings["Settings"]["Window"] = {
            {"SCREEN_X", ResX},
            {"SCREEN_Y", ResY},
            {"x", wi.x},
            {"y", wi.y},
            {"SizeX", wi.cx},
            {"SizeY", wi.cy},
            {"Maximized", (bool)wi.max}
        };
    }
    
    jsonfile = settings;

    //ScintillaStyles_Save
    // TODO

    std::ofstream file(fileNameSettings_JSON);
    file << std::setw(2) << jsonfile;

    return false;
}

bool Settings::checkIniFile(std::wstring& file, const std::wstring& module)
{
    std::wstring fileExpanded;
    std::wstring build;
    fileExpanded.resize(MAX_PATH);
    build.resize(MAX_PATH);

    ExpandEnvironmentStrings(file.c_str(), fileExpanded.data(), fileExpanded.size());

    if (PathIsRelative(fileExpanded.data())) {
        // program directory
        build = module;
        lstrcpy(PathFindFileName(build.c_str()), fileExpanded.data());
        if (PathFileExists(build.data())) {
            lstrcpy(file.data(), build.data());
            return true;
        }
        // %appdata%
        if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, build.data())) {
            PathAppend(build.data(), fileExpanded.data());
            if (PathFileExists(build.data())) {
                lstrcpy(file.data(), build.data());
                return true;
            }
        }
        // general
        if (SearchPath(NULL, fileExpanded.data(), NULL, MAX_PATH, build.data(), NULL)) {
            lstrcpy(file.data(), build.data());
            return true;
        }
    }

    else if (PathFileExists(fileExpanded.data())) {
        lstrcpy(file.data(), fileExpanded.data());
        return true;
    }

    return false;
}

bool Settings::checkIniFileRedirect(std::wstring& file, const std::wstring& module)
{
    std::wstring tch;
    tch.resize(MAX_PATH);
    if (GetPrivateProfileString(L"Notepad2", fileNameSettings, L"", tch.data(), tch.size(), file.data())) {
        if (checkIniFile(tch, module)) {
            lstrcpy(file.data(), tch.c_str());
            return true;
        }
        else {
            WCHAR tchFileExpanded[MAX_PATH];
            ExpandEnvironmentStrings(tch.data(), tchFileExpanded, MAX_PATH);
            if (PathIsRelative(tchFileExpanded)) {
                lstrcpy(file.data(), module.c_str());
                lstrcpy(PathFindFileName(file.data()), tchFileExpanded);
                return true;
            }
            else {
                file = tchFileExpanded;
                return true;
            }
        }
    }
    return false;
}
