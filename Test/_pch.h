#pragma once

#ifndef _ALLOW_RTCc_IN_STL
#define _ALLOW_RTCc_IN_STL
#endif

// Target Windows 10  
#define WINVER		 _WIN32_WINNT_WIN10
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <SDKDDKVer.h>

#define NOMINMAX	
#define NOMENUS
#define NODRAWTEXT
#define NOCTLMGR
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOTEXTMETRIC
//#define NOGDI
#define NOBITMAP
#define NOMCX	
#define NOSERVICE	
#define NOHELP	
#define WIN32_LEAN_AND_MEAN 
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

using std::unique_ptr;
using std::unordered_map;
using std::string;
using std::wstring;
using std::vector;
using std::chrono::high_resolution_clock;

/// Core
#include "../../Core/Core/core.h"
#ifdef _DEBUG
#pragma comment(lib, "../../Core/x64/Debug/Core.lib")
#else 
#pragma comment(lib, "../../Core/x64/Release/Core.lib")
#endif
