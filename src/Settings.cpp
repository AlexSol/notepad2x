#include "Settings.h"
#include "precheader.h"

extern "C" void ExpandEnvironmentStringsEx(LPWSTR lpSrc, DWORD dwSrc);

static auto fileNameSettings = L"Notepad2.ini";

void loadIniSection(const std::wstring& section, std::wstring& out, const std::wstring& fileName);

void Settings::load()
{
    findIniFile();
    testIniFile();
    createIniFile();
    loadSettings();
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