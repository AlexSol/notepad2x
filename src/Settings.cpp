#include "Settings.h"
#include "precheader.h"

#include "Notepad2.h"
//#include "Edit.h"

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

extern "C"  T_Settings TEG_Settings;
extern "C" int flagPortableMyDocs;
extern "C" WCHAR tchOpenWithDir[MAX_PATH];
extern "C" WCHAR tchFavoritesDir[MAX_PATH];
extern "C" EDITFINDREPLACE efrData;

static auto fileNameSettings = L"Notepad2.ini";
static auto fileNameSettings_JSON = L"Notepad2x.json";

void loadIniSection(const std::wstring& section, std::wstring& out, const std::wstring& fileName);

// encoding function
std::string to_utf8(std::wstring& wide_string)
{
    static std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8_conv;
    return utf8_conv.to_bytes(wide_string);
}

void Settings::load()
{
    saveSettings();
    //findIniFile();
    //testIniFile();
    //createIniFile();
    //loadSettings();
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
    std::wstring iniSection;
    iniSection.resize(32 * 1024);

    loadIniSection(L"Settings", iniSection, _iniFile);

    //bSaveSettings = IniSectionGetInt(pIniSection, L"SaveSettings", 1);
    //if (bSaveSettings) bSaveSettings = 1;

    return false;
}

bool Settings::saveSettings()
{
    using json = nlohmann::json;

    json jsonfile;
    
    json settings;
    settings["Settings"]["SaveSettings"] = TEG_Settings.bSaveSettings;
    settings["Settings"]["SaveRecentFiles"] = TEG_Settings.bSaveRecentFiles;
    settings["Settings"]["SaveFindReplace"] = TEG_Settings.bSaveFindReplace;
    
    settings["Settings"]["CloseFind"] = efrData.bFindClose;
    settings["Settings"]["CloseReplace"] = efrData.bReplaceClose;
    settings["Settings"]["NoFindWrap"] = efrData.bNoFindWrap;

    {
        WCHAR wchTmp[MAX_PATH];
        PathRelativeToApp(tchOpenWithDir, wchTmp, MAX_PATH, FALSE, TRUE, flagPortableMyDocs);
        auto ws = std::wstring(wchTmp);
        settings["Settings"]["OpenWithDir"] = to_utf8(ws);
    }
    {
        WCHAR wchTmp[MAX_PATH];
        PathRelativeToApp(tchFavoritesDir, wchTmp, MAX_PATH, FALSE, TRUE, flagPortableMyDocs);
        auto ws = std::wstring(wchTmp);
        settings["Settings"]["Favorites"] = to_utf8(ws);
    }

    settings["Settings"]["PathNameFormat"] = TEG_Settings.iPathNameFormat;
    settings["Settings"]["WordWrap"] = TEG_Settings.fWordWrapG;
    settings["Settings"]["WordWrapMode"] = TEG_Settings.iWordWrapMode;
    settings["Settings"]["WordWrapIndent"] = TEG_Settings.iWordWrapIndent;
    settings["Settings"]["WordWrapSymbols"] = TEG_Settings.iWordWrapSymbols;
    settings["Settings"]["ShowWordWrapSymbols"] = TEG_Settings.bShowWordWrapSymbols;
    settings["Settings"]["MatchBraces"] = TEG_Settings.bMatchBraces;
    settings["Settings"]["AutoCloseTags"] = TEG_Settings.bAutoCloseTags;
    settings["Settings"]["HighlightCurrentLine"] = TEG_Settings.bHiliteCurrentLine;
    settings["Settings"]["AutoIndent"] = TEG_Settings.bAutoIndent;
    settings["Settings"]["AutoCompleteWords"] = TEG_Settings.bAutoCompleteWords;
    settings["Settings"]["ShowIndentGuides"] = TEG_Settings.bShowIndentGuides;
    settings["Settings"]["TabsAsSpaces"] = TEG_Settings.bTabsAsSpacesG;
    settings["Settings"]["TabIndents"] = TEG_Settings.bTabIndentsG;
    settings["Settings"]["BackspaceUnindents"] = TEG_Settings.bBackspaceUnindents;
    settings["Settings"]["TabWidth"] = TEG_Settings.iTabWidthG;
    settings["Settings"]["IndentWidth"] = TEG_Settings.iIndentWidthG;
    settings["Settings"]["MarkLongLines"] = TEG_Settings.bMarkLongLines;
    settings["Settings"]["LongLinesLimit"] = TEG_Settings.iLongLinesLimitG;
    settings["Settings"]["LongLineMode"] = TEG_Settings.iLongLineMode;
    settings["Settings"]["ShowSelectionMargin"] = TEG_Settings.bShowSelectionMargin;
    settings["Settings"]["ShowLineNumbers"] = TEG_Settings.bShowLineNumbers;
    settings["Settings"]["ShowCodeFolding"] = TEG_Settings.bShowCodeFolding;
    settings["Settings"]["MarkOccurrences"] = TEG_Settings.iMarkOccurrences;
    settings["Settings"]["MarkOccurrencesMatchCase"] = TEG_Settings.bMarkOccurrencesMatchCase;
    settings["Settings"]["MarkOccurrencesMatchWholeWords"] = TEG_Settings.bMarkOccurrencesMatchWords;
    settings["Settings"]["ViewWhiteSpace"] = TEG_Settings.bViewWhiteSpace;
    settings["Settings"]["ViewEOLs"] = TEG_Settings.bViewEOLs;
    settings["Settings"]["DefaultEncoding"] = Encoding_MapIniSetting(FALSE, TEG_Settings.iDefaultEncoding);
    settings["Settings"]["SkipUnicodeDetection"] = TEG_Settings.bSkipUnicodeDetection;
    settings["Settings"]["LoadASCIIasUTF8"] = TEG_Settings.bLoadASCIIasUTF8;
    settings["Settings"]["LoadNFOasOEM"] = TEG_Settings.bLoadNFOasOEM;
    settings["Settings"]["NoEncodingTags"] = TEG_Settings.bNoEncodingTags;
    settings["Settings"]["DefaultEOLMode"] = TEG_Settings.iDefaultEOLMode;
    settings["Settings"]["FixLineEndings"] = TEG_Settings.bFixLineEndings;
    settings["Settings"]["FixTrailingBlanks"] = TEG_Settings.bAutoStripBlanks;
    settings["Settings"]["PrintHeader"] = TEG_Settings.iPrintHeader;
    settings["Settings"]["PrintFooter"] = TEG_Settings.iPrintFooter;
    settings["Settings"]["PrintColorMode"] = TEG_Settings.iPrintColor;
    settings["Settings"]["PrintZoom"] = TEG_Settings.iPrintZoom + 10;
    settings["Settings"]["PrintMargin"] = {
        {"left", TEG_Settings.pagesetupMargin.left},
        {"top", TEG_Settings.pagesetupMargin.top},
        {"right", TEG_Settings.pagesetupMargin.right},
        {"bottom", TEG_Settings.pagesetupMargin.bottom},
    };
    settings["Settings"]["SaveBeforeRunningTools"] = TEG_Settings.bSaveBeforeRunningTools;
    settings["Settings"]["FileWatchingMode"] = TEG_Settings.iFileWatchingMode;
    settings["Settings"]["ResetFileWatching"] = TEG_Settings.bResetFileWatching;
    settings["Settings"]["EscFunction"] = TEG_Settings.iEscFunction;
    settings["Settings"]["AlwaysOnTop"] = TEG_Settings.bAlwaysOnTop;
    settings["Settings"]["MinimizeToTray"] = TEG_Settings.bMinimizeToTray;
    settings["Settings"]["TransparentMode"] = TEG_Settings.bTransparentMode;

    auto ws = std::wstring(TEG_Settings.tchToolbarButtons);
    settings["Settings"]["ToolbarButtons"] = to_utf8(ws);

    settings["Settings"]["ShowToolbar"] = TEG_Settings.bShowToolbar;
    settings["Settings"]["ShowStatusbar"] = TEG_Settings.bShowStatusbar;
    settings["Settings"]["EncodingDlgSize"] = { {"x", TEG_Settings.cxEncodingDlg }, {"y", TEG_Settings.cyEncodingDlg } };
    settings["Settings"]["RecodeDlgSize"] = { {"x", TEG_Settings.cxRecodeDlg }, {"y", TEG_Settings.cyRecodeDlg } };
    settings["Settings"]["FileMRUDlgSize"] = { {"x", TEG_Settings.cxFileMRUDlg }, {"y", TEG_Settings.cyFileMRUDlg } };
    settings["Settings"]["OpenWithDlgSize"] = { {"x", TEG_Settings.cxOpenWithDlg }, {"y", TEG_Settings.cyOpenWithDlg } };
    settings["Settings"]["FavoritesDlgSize"] = { {"x", TEG_Settings.cxFavoritesDlg }, {"y", TEG_Settings.cyFavoritesDlg } };
    settings["Settings"]["FindReplaceDlgPos"] = { {"x", TEG_Settings.xFindReplaceDlg }, {"y", TEG_Settings.yFindReplaceDlg } };


    jsonfile = settings;

    //jsonfile["Settings2"] = "bar";
    //jsonfile["Recent Files"] = "bar";
    //jsonfile["Recent Find"] = "bar";
    //jsonfile["Recent Replace"] = "bar";
    //jsonfile["Window"] = "bar";
    //jsonfile["Custom Colors"] = "bar";

    std::ofstream file(fileNameSettings_JSON);
    file << std::setw(4) << jsonfile;

    return false;
}

bool Settings::saveSettings(bool saveSettingsNow)
{
    createIniFile();
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

void loadIniSection(const std::wstring& section, std::wstring& out, const std::wstring& fileName)
{
    out.resize(32 * 1024);
    GetPrivateProfileSection(section.c_str(), out.data(), out.size(), fileName.c_str());
}