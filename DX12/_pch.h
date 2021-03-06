// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#ifndef _ALLOW_RTCc_IN_STL
#define _ALLOW_RTCc_IN_STL
#endif

// Target Windows 10      
#define WINVER		 _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <SDKDDKVer.h>

#define NOMINMAX	// Use the C++ standard templated min/max
// DirectX apps don't need GDI
#define NOMENUS
#define NODRAWTEXT
#define NOCTLMGR
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOTEXTMETRIC
//#define NOGDI
#define NOBITMAP
#define NOMCX	// Include <mcx.h> if you need this
#define NOSERVICE	// Include <winsvc.h> if you need this
#define NOHELP	// WinHelp is deprecated
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cassert>

// std namespace files
#include <vector>
#include <chrono>
#include <string>
#include <unordered_map>
#include <exception>
#include <memory>

/// Core
#include "../../Core/Core/core.h"

#include "../External/DDSTextureLoader12.h"

using std::unique_ptr;
using std::unordered_map;
using std::string;
using std::wstring;
using std::vector;
using std::chrono::high_resolution_clock;
