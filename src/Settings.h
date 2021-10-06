#pragma once
#include <string>

class Settings
{
public:
	void load();
	void loadFlags();

private:
	bool findFile();
	bool testIniFile();
	bool createIniFile();
	bool createIniFileEx(std::wstring_view file);
	bool loadSettings();
	bool saveSettings(bool saveSettingsNow);
	bool checkFile(std::wstring& file, std::wstring_view module);
	bool checkFileRedirect(std::wstring& file, std::wstring_view module);

	std::wstring _file;
	std::wstring _iniFile2;
};

