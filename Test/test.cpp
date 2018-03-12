#include "_pch.h"

using namespace core;

#define _ALLOW_RTCc_IN_STL

#include "../DX12/_exported.h"

using namespace dx12;
using namespace DirectX;

#include "base_test.h"
#include "triangle.h"
#include "compute.h"

int APIENTRY wWinMain(HINSTANCE hInstance,
					  HINSTANCE hPrevInstance,
					  LPWSTR    lpCmdLine,
					  int       nCmdShow) {
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF);
#endif

#define TEST 1

	try{
#if TEST==1
		TriangleTest app;
#elif TEST==2

#endif
		app.init(hInstance, nCmdShow);
		app.run();
	}catch(std::exception& e) {
		wstring msg = WString::toWString(e.what());
		MessageBox(nullptr, msg.c_str(), L"Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;
	}
	return 0;
}
