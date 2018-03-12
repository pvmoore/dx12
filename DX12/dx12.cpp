#include "_pch.h"
#include "_exported.h"

namespace dx12 {

using namespace core;

DX12* DX12::self = nullptr;

DX12::DX12() {
	self = this;
}
DX12::~DX12() {
	Log::write("Deleting DX12");
	if(hwnd && hInstance) {
		ChangeDisplaySettings(nullptr, 0);
		DestroyWindow(hwnd);
		UnregisterClass(WINDOW_CLASS, hInstance);
	}
}
void DX12::run() {
	Log::write("Entering run loop");
	MSG msg;
	high_resolution_clock clock;
	double totalSeconds = 0;
	uint prevSecond = 0;
	uint prev5Seconds = 0;
	while(true) {
		auto start = clock.now();
		if(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if(msg.message == WM_QUIT) break;
			continue;
		}

		renderFrame();

		frameNumber++;
		auto end = clock.now();
		auto elapsed = end-start;
		totalSeconds += elapsed.count() * 1e-9;

		if((uint)totalSeconds > prevSecond) {
			// Every second
			prevSecond = (uint)totalSeconds;
			double ms  = elapsed.count() * 1e-6;
			double fps = 1000.0 / ms;
			Log::format("Frame [%llu] Elapsed: %.3fms, FPS: %.3f", frameNumber, ms, fps);
			// Every 5 seconds
			if((uint)totalSeconds>prev5Seconds+5) {
				prev5Seconds = (uint)totalSeconds;
				DXGI_QUERY_VIDEO_MEMORY_INFO memoryInfo;
				adapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &memoryInfo);
				Log::write("\tSystem mem usage .. ?");
				Log::format("\tGPU mem usage ..... %llu MB (of %llu MB budget)",
					memoryInfo.CurrentUsage/(1024*1024),
					memoryInfo.Budget/(1024*1024));
				// Add fps to window title
				if(params.windowMode==WindowMode::WINDOWED) {
					wstring s = params.title + wstring(L" : ") +
						std::to_wstring((uint)fps) + wstring(L" FPS");
					setTitle(s.c_str());
				}
			}
		}
	}
	Log::write("Exiting run loop");
	swapChain.waitForGPU();
}
void DX12::renderFrame() {
	FrameResource& frame = swapChain.currentFrame();
	auto allocator = frame.allocator;
	frame.number = frameNumber;

	// Reset (re-use) the memory associated with command allocator.
	throwOnDXError(allocator->Reset());
	// Reset the command list.
	throwOnDXError(commandList->Reset(allocator.Get(), nullptr));

	// Transition from present to render.
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = frame.renderTarget.Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	commandList->ResourceBarrier(1, &barrier);

	// Set the back buffer as the render target.
	// (Sets CPU descriptor handles for the render targets and depth stencil)
	commandList->OMSetRenderTargets(1, &frame.rtvHandle, FALSE, nullptr);

	// Set the color to clear the window to.
	float color[] = {0.1f, 0.1f, 0.4f, 1.0f};
	commandList->ClearRenderTargetView(frame.rtvHandle, color, 0, nullptr);
	//commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Set the viewport and scissor rect.
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	/// Let the application do some drawing
	eventHandler->render(frame, commandList);

	// Transition from render to present.
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);

	// Close the list of commands.
	throwOnDXError(commandList->Close());

	// Execute the list(s) of commands.
	ID3D12CommandList* ppCommandLists[1] = {commandList.Get()};
	commandQueue->ExecuteCommandLists(1, ppCommandLists);

	/// Present the back buffer to the screen
	swapChain.present();
}
bool DX12::init(const HINSTANCE hInstance,
				const InitParams params,
				InputEventHandler* eventHandler) {
	Log::write("Initialising DX12");
	this->hInstance = hInstance;
	this->params = params;
	this->eventHandler = eventHandler;
	this->swapChain.init(params.numBackBuffers);

	createWindow();
	createDevice();
	createCommandQueues();
	swapChain.create();
	createCommandLists();

	// Set viewport values to entire screen
	auto size = getWindowSize();
	viewport.TopLeftX = viewport.TopLeftY = 0.0f;
	viewport.Width = (float)size.width;
	viewport.Height = (float)size.height;
	viewport.MinDepth = D3D12_MIN_DEPTH;
	viewport.MaxDepth = D3D12_MAX_DEPTH;

	// Set scissor rect values to entire screen
	scissor.left = scissor.top = 0;
	scissor.right = size.width;
	scissor.bottom = size.height;

	Log::write("DX12 Ready");
	Log::write("======================================================");
	return true;
}
bool DX12::createWindow() {
	Log::write("Creating window");
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = WINDOW_CLASS;
	wcex.hIconSm = NULL;
	wcex.hIcon = NULL;
	RegisterClassExW(&wcex);

	// Disable auto scaling of the window (Windows 10 only)
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	uint dpi = GetDpiForSystem();
	Log::format("\tDesktop DPI = %u (%u%% scaling)", dpi, (dpi*100)/96);

	auto screenWidth = GetSystemMetrics(SM_CXSCREEN);
	auto screenHeight = GetSystemMetrics(SM_CYSCREEN);
	Log::format("\tScreen size is %ux%u", screenWidth, screenHeight);

	DWORD dwExStyle;				
	DWORD dwStyle;				
	RECT windowRect = {};					

	if(params.windowMode==WindowMode::WINDOWED_FULLSCREEN) {
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
		// Make a small window initially and resize it later
		windowRect.right = 100;
		windowRect.bottom = 100;
	} else {
		windowRect.left = (screenWidth - params.width) / 2;
		windowRect.top = (screenHeight - params.height) / 4;
		windowRect.right = windowRect.left + params.width;
		windowRect.bottom = windowRect.top + params.height;
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			
		dwStyle = WS_OVERLAPPEDWINDOW;
		Log::format("\tWindow rect: (%d,%d) - (%d,%d)", windowRect.left, windowRect.top, windowRect.right, windowRect.bottom);
	}

	hwnd = CreateWindowEx(
		dwExStyle,
		WINDOW_CLASS,
		params.title,
		dwStyle | WS_CLIPSIBLINGS |	WS_CLIPCHILDREN,
		windowRect.left, 
		windowRect.top,
		windowRect.right-windowRect.left,	
		windowRect.bottom-windowRect.top,
		HWND_DESKTOP,
		nullptr,
		hInstance,
		nullptr);

	if(!hwnd) {
		return false;
	}

	if(params.windowMode==WindowMode::WINDOWED_FULLSCREEN) {
		// Adjust window to fit nearest monitor
		HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFOEX monitorInfo = {};
		monitorInfo.cbSize = sizeof(MONITORINFOEX);
		GetMonitorInfo(hMonitor, &monitorInfo);

		SetWindowPos(hwnd, HWND_TOPMOST,
					 monitorInfo.rcMonitor.left,
					 monitorInfo.rcMonitor.top,
					 monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
					 monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
					 SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_HIDEWINDOW);
		Log::format("\tWindow rect: (%d,%d) - (%d,%d)", monitorInfo.rcMonitor.left, monitorInfo.rcMonitor.top, monitorInfo.rcMonitor.right, monitorInfo.rcMonitor.bottom);
		// Update these for creating the back buffers later
		params.width = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
		params.height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
	}

	char title[128] = {};
	wcstombs_s(nullptr, title, 128, params.title, 128);
	Log::format("\tCreated %ux%d window '%s'", params.width, params.height, title);
	UpdateWindow(hwnd);
	return true;
}
void DX12::selectAdapter(ComPtr<IDXGIFactory5> factory) {
	Log::write("\tEnumerating adapters");
	ComPtr<IDXGIAdapter1> adapter1;
	char str[128] = {};
	ulong maxMemory = 0;
	uint selected = 0;
	for(uint i = 0; factory->EnumAdapters1(i, adapter1.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND; i++) {
		Log::write("\tAdapter %d {", i);
		DXGI_ADAPTER_DESC1 desc;
		adapter1->GetDesc1(&desc);
		bool isSoftware = desc.Flags&DXGI_ADAPTER_FLAG_SOFTWARE;
		wcstombs_s(nullptr, str, 128, desc.Description, 128);
		Log::format("\t\tVendorId: %u", desc.VendorId);
		Log::format("\t\tDeviceId: %u", desc.DeviceId);
		Log::format("\t\tSubSysId: %u", desc.SubSysId);
		Log::format("\t\tRevision: %u", desc.Revision);
		Log::format("\t\tDedicatedVideoMemory: %lld", desc.DedicatedVideoMemory);
		Log::format("\t\tDedicatedSystemMemory: %lld", desc.DedicatedSystemMemory);
		Log::format("\t\tSharedSystemMemory   : %lld", desc.SharedSystemMemory);
		Log::format("\t\tDescription: %s", str);
		Log::format("\t\tType: %s", isSoftware ? "SOFTWARE" : "HARDWARE");
		Log::write("\t}");
		if(!isSoftware && desc.DedicatedVideoMemory > maxMemory) {
			maxMemory = desc.DedicatedSystemMemory;
			adapter1.As(&adapter);
			selected = i;
		}
	}
	Log::format("\tSelected adapter %u", selected);
}
bool DX12::createDevice() {
#ifdef _DEBUG
	Log::write("Enabling debug layer");
	if(SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.GetAddressOf())))) {
		debugLayer->EnableDebugLayer();
	}
#endif
	Log::write("Creating DXGI factory");
	uint factoryFlags = 0;
#ifdef _DEBUG
	factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	throwOnDXError(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(factory.GetAddressOf())));

	Log::write("Selecting adapter");
	selectAdapter(factory);

	Log::write("Creating device");
	auto result = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
	if(FAILED(result)) {
		throw std::runtime_error("Could not create a DirectX 11.0 device.  The default video card does not support DirectX 11.0");
	}
#ifdef _DEBUG
	ComPtr<ID3D12InfoQueue> infoQueue;
	if(SUCCEEDED(device.As(&infoQueue))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		//D3D12_MESSAGE_SEVERITY Severities[] = {
		//	D3D12_MESSAGE_SEVERITY_INFO
		//};
		// Suppress individual messages by their ID
		//D3D12_MESSAGE_ID DenyIds[] = {
		//	D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
		//	D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
		//	D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
		//};

		//D3D12_INFO_QUEUE_FILTER NewFilter = {};
		//NewFilter.DenyList.NumCategories = _countof(Categories);
		//NewFilter.DenyList.pCategoryList = Categories;
		//NewFilter.DenyList.NumSeverities = _countof(Severities);
		//NewFilter.DenyList.pSeverityList = Severities;
		//NewFilter.DenyList.NumIDs = _countof(DenyIds);
		//NewFilter.DenyList.pIDList = DenyIds;

		//check(infoQueue->PushStorageFilter(&NewFilter));
	}
#endif
	// All of these need the device
	this->resources.init(device);
	this->uploader.init();

	{
		auto maxFeatureLevel = getMaxSupportedFeatureLevel(device);
		const char* str;
		switch(maxFeatureLevel) {
			case D3D_FEATURE_LEVEL_11_0: str = "11.0"; break;
			case D3D_FEATURE_LEVEL_11_1: str = "11.1"; break;
			case D3D_FEATURE_LEVEL_12_0: str = "12.0"; break;
			case D3D_FEATURE_LEVEL_12_1: str = "12.1"; break;
			default: str = "UNKNOWN"; break;
		}
		Log::format("\tMax supported feature level: %s", str);
	}
	return true;
}
void DX12::createCommandQueues() {
	Log::write("Creating direct command queue");
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	throwOnDXError(device->CreateCommandQueue(&desc, IID_PPV_ARGS(commandQueue.GetAddressOf())));
}
void DX12::createCommandLists() {
	Log::write("Creating graphics command list");
	throwOnDXError(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, swapChain.currentFrame().allocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));
	throwOnDXError(commandList->Close());
}
/// static 
LRESULT DX12::windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	const auto getKeyMod = [=]()->KeyMod {
		return ((wParam&MK_CONTROL) ? KeyMod::CTRL : KeyMod::NONE) |
			((wParam&MK_SHIFT) ? KeyMod::SHIFT : KeyMod::NONE);
	};
	const auto handleMouseButton = [=](int button, MouseClick click) {
		if(self->eventHandler) {
			POINT pos = {LOWORD(lParam), HIWORD(lParam)};
			self->eventHandler->mouseButton(button, pos, getKeyMod(), click);
		}
	};
	const auto handleKey = [=](bool pressed) {
		if(self->eventHandler) {
			const bool isRepeat = lParam&0x40000000;
			if(pressed && isRepeat) return;
			self->eventHandler->key((int)wParam, pressed);
		}
	};
	switch(message) {
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_KEYDOWN:
			if(wParam==VK_ESCAPE) PostQuitMessage(0);
			handleKey(true);
			break;
		case WM_KEYUP:
			handleKey(false);
			break;
		case WM_MOUSEMOVE:
			if(self->eventHandler) {
				self->eventHandler->mouseMove({LOWORD(lParam), HIWORD(lParam)}, getKeyMod());
			}
			break;
		case WM_MOUSEWHEEL:
			if(self->eventHandler) {
				self->eventHandler->mouseWheel((short)HIWORD(wParam), getKeyMod());
			}
			break;
		case WM_LBUTTONDBLCLK:
			handleMouseButton(0, MouseClick::DBLCLICK);
			break;
		case WM_MBUTTONDBLCLK:
			handleMouseButton(1, MouseClick::DBLCLICK);
			break;
		case WM_RBUTTONDBLCLK:
			handleMouseButton(2, MouseClick::DBLCLICK);
			break;
		case WM_LBUTTONDOWN:
			handleMouseButton(0, MouseClick::PRESS);
			break;
		case WM_LBUTTONUP:
			handleMouseButton(0, MouseClick::RELEASE);
			break;
		case WM_MBUTTONDOWN:
			handleMouseButton(1, MouseClick::PRESS);
			break;
		case WM_MBUTTONUP:
			handleMouseButton(1, MouseClick::RELEASE);
			break;
		case WM_RBUTTONDOWN:
			handleMouseButton(2, MouseClick::PRESS);
			break;
		case WM_RBUTTONUP:
			handleMouseButton(2, MouseClick::RELEASE);
			break;
			//case WM_POWERBROADCAST:
		case WM_ACTIVATEAPP:
			if(wParam) {
				// activate
			} else {
				// deactivate
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

} /// dx12