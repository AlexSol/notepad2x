#pragma once
#include <string>

class Settings
{
public:
	void load();

private:
	bool findIniFile();
	bool testIniFile();
	bool createIniFile();
	bool createIniFileEx(const std::wstring& file);
	bool loadSettings();
	bool saveSettings();
	bool saveSettings(bool saveSettingsNow);
	bool checkIniFile(std::wstring& file, const std::wstring& module);
	bool checkIniFileRedirect(std::wstring& file, const std::wstring& module);

	std::wstring _iniFile;
	std::wstring _iniFile2;
};

