#pragma once

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x501
#endif

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include "Shlobj.h"
#include "scintilla.h"
#include "scilexer.h"
#include "SciCall.h"
#include "resource.h"

#include <string>

#include <nlohmann/json.hpp>
#include <fstream>
#include <iomanip> 
#include <codecvt> // codecvt_utf8
#include <locale>  // wstring_convert

