#include "Settings.h"
#include "precheader.h"


#include "Config.h"
#include "utility.h"

extern "C" {
#include "Notepad2.h"
}

extern "C" int  Encoding_MapIniSetting(BOOL, int);
extern "C" void PathRelativeToApp(LPWSTR, LPWSTR, int, BOOL, BOOL, BOOL);
extern "C" void GetWinInfo2(HWND hwnd, PTR_WININFO  wi);

std::wstring ExpandEnvironment(const std::wstring& path);

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

extern "C" WININFO wi;
extern "C" HWND  hwndMain;

extern "C"  T_Settings SETTINGS;
extern "C" WCHAR tchOpenWithDir[MAX_PATH];
extern "C" WCHAR tchFavoritesDir[MAX_PATH];
extern "C" EDITFINDREPLACE efrData;
extern "C" T_FLAG FLAG;

static auto fileNameSettings = L"Notepad2.ini";
static auto fileNameSettings_JSON = L"Notepad2x.json";

extern "C" WCHAR     g_wchAppUserModelID[32];

// encoding function
std::string to_utf8(std::wstring& wide_string)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(wide_string);
}

void Settings::load()
{
    saveSettings(true);
    findFile();
    testIniFile();
    createIniFile();
    
    //loadSettings();
}

void Settings::loadFlags()
{
    using json = nlohmann::json;
    std::ifstream iFile(fileNameSettings_JSON);

    json jsonfile;;
    iFile >> jsonfile;
    auto Settings2 = jsonfile["Settings2"];

    if (!FLAG.ReuseWindow && !FLAG.NoReuseWindow) {
        if (Settings2["ReuseWindow"].is_null()) {
            FLAG.NoReuseWindow = 1;
        }
        if (Settings2["SingleFileInstance"].is_null() == false) {
            FLAG.SingleFileInstance = 1;
        }
    }

    if (FLAG.MultiFileArg == 0) {
        if (Settings2["SingleFileInstance"].is_null() == false) {
            FLAG.MultiFileArg = 2;
        }
    }

    if (Settings2["RelativeFileMRU"].is_null() == false) { //default 1
        FLAG.RelativeFileMRU = 1;
    }

    if (Settings2["PortableMyDocs"].is_null() == false || FLAG.RelativeFileMRU) {
        FLAG.PortableMyDocs = 1;
    }

    if (Settings2["NoFadeHidden"].is_null() == false) {
        FLAG.NoFadeHidden = 1;
    }

    //if (auto node = Settings2["ToolbarLook"]; node.is_null() == false) {
    //    FLAG.ToolbarLook = 2;
    //}
    FLAG.ToolbarLook = 2;

    if (Settings2["SimpleIndentGuides"].is_null() == false) {
        FLAG.SimpleIndentGuides = 1;
    }

    if (Settings2["NoHTMLGuess"].is_null() == false) {
        FLAG.NoHTMLGuess = 1;
    }

    if (Settings2["NoCGIGuess"].is_null() == false) {
        FLAG.NoCGIGuess = 1;
    }

    if (Settings2["NoFileVariables"].is_null() == false) {
        FLAG.NoFileVariables = 1;
    }

    if (lstrlen(g_wchAppUserModelID) == 0) {
        if (auto node = Settings2["ShellAppUserModelID"]; node.is_null()) {
            lstrcpyn(g_wchAppUserModelID, L"(default)", lstrlen(L"(default)") + 1);
        }
        else{
           // lstrcpyn(g_wchAppUserModelID, node.get<std::string>().c_str(), node.get<std::string>().length());
        }
    }

    if (FLAG.UseSystemMRU == 0) {
        if (Settings2["ShellUseSystemMRU"].is_null() == false) {
            FLAG.UseSystemMRU = 2;
        }
    }
}

bool Settings::findFile()
{
    int bFound = 0;
    std::wstring tchTest;
    std::wstring tchModule;
    tchTest.resize(MAX_PATH);
    tchModule.resize(MAX_PATH);

    GetModuleFileName(NULL, tchModule.data(), MAX_PATH);

    if (!_file.empty()) {
        if (lstrcmpi(_file.c_str(), L"*?") == 0)
            return false;
        else {
            if (!checkFile(_file, tchModule)) {
                _file = ExpandEnvironment(_file);
                if (PathIsRelative(_file.data())) {
                    lstrcpy(tchTest.data(), tchModule.data());
                    PathRemoveFileSpec(tchTest.data());
                    PathAppend(tchTest.data(), _file.data());
                    lstrcpy(_file.data(), tchTest.data());
                }
            }
        }
        return true;
    }

    lstrcpy(tchTest.data(), PathFindFileName(tchModule.data()));
    PathRenameExtension(tchTest.data(), L".ini");
    bFound = checkFile(tchTest, tchModule);

    if (!bFound) {
        lstrcpy(tchTest.data(), fileNameSettings);
        bFound = checkFile(tchTest, tchModule);
    }

    if (bFound) {
        // allow two redirections: administrator -> user -> custom
        if (checkFileRedirect(tchTest, tchModule))
            checkFileRedirect(tchTest, tchModule);
        _file = tchTest;
    }
    else {
        lstrcpy(_file.data(), tchModule.data());
        PathRenameExtension(_file.data(), L".ini");
    }

    return true;
}

bool Settings::testIniFile()
{
    _iniFile2.resize(MAX_PATH);
    if (lstrcmpi(_file.c_str(), L"*?") == 0) {
        lstrcpy(_iniFile2.data(), L"");
        lstrcpy(_file.data(), L"");
        return false;
    }

    if (PathIsDirectory(_file.data()) || *CharPrev(_file.data(), &_file[_file.size()]) == L'\\') {
        std::wstring module;
        module.resize(MAX_PATH);
        GetModuleFileName(NULL, module.data(), module.size());
        PathAppend(_file.data(), PathFindFileName(module.data()));
        PathRenameExtension(_file.data(), L".ini");
        if (!PathFileExists(_file.data())) {
            lstrcpy(PathFindFileName(_file.data()), L"Notepad2.ini");
            if (!PathFileExists(_file.data())) {
                lstrcpy(PathFindFileName(_file.data()), PathFindFileName(module.data()));
                PathRenameExtension(_file.data(), L".ini");
            }
        }
    }

    if (!PathFileExists(_file.data()) || PathIsDirectory(_file.data())) {
        lstrcpy(_iniFile2.data(), _file.data());
        lstrcpy(_file.data(), L"");
        return false;
    }
    else
        return true;
}

bool Settings::createIniFile()
{
    return createIniFileEx(_file);
}

bool Settings::createIniFileEx(std::wstring_view file)
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
        GetWinInfo2(hwndMain, &wi);
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

bool Settings::checkFile(std::wstring& file, std::wstring_view module)
{
    namespace fs = std::filesystem;

    std::wstring build;
    build.resize(MAX_PATH);

    std::wstring fileExpanded = ExpandEnvironment(file);

    if (PathIsRelative(fileExpanded.data())) {
        // program directory
        build = module;
        lstrcpy(PathFindFileName(build.c_str()), fileExpanded.data());
        if (fs::exists(build)) {
            file = build;
            return true;
        }
        // %appdata%
        if (S_OK == SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, build.data())) {
            PathAppend(build.data(), fileExpanded.data());
            if (fs::exists(build.data())) {
                file = build;
                return true;
            }
        }
        // general
        if (SearchPath(NULL, fileExpanded.data(), NULL, MAX_PATH, build.data(), NULL)) {
            file = build;
            return true;
        }
    }else if (fs::exists(fileExpanded)) {
        file = fileExpanded;
        return true;
    }

    return false;
}

bool Settings::checkFileRedirect(std::wstring& file, std::wstring_view module)
{
    std::wstring tch;
    tch.resize(MAX_PATH);
    if (GetPrivateProfileString(L"Notepad2", fileNameSettings, L"", tch.data(), tch.size(), file.data())) {
        if (checkFile(tch, module)) {
            file = tch;
            return true;
        }
        else {
            std::wstring tchFileExpanded = ExpandEnvironment(tch);
            if (PathIsRelative(tchFileExpanded.c_str())) {
                file = module;
                lstrcpy(PathFindFileName(file.data()), tchFileExpanded.c_str());
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

std::wstring ExpandEnvironment(const std::wstring& path)
{
    WCHAR szBuf[312];
    if (::ExpandEnvironmentStrings(path.c_str(), szBuf, 312)) {
        return std::wstring{ szBuf };
    }
}