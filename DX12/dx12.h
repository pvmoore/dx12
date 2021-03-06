#pragma once

namespace dx12 {

enum KeyMod : uint { NONE = 0, CTRL = 1, SHIFT = 2 };
enum MouseClick : uint { PRESS, RELEASE, DBLCLICK };
enum WindowMode : uint { WINDOWED, WINDOWED_FULLSCREEN}; // FULLSCREEN
inline KeyMod operator|(KeyMod a, KeyMod b) {
	return static_cast<KeyMod>(static_cast<uint>(a) | static_cast<uint>(b));
}

class InputEventHandler {
public:
	virtual void key(int vkCode, bool pressed) = 0;
	virtual void mouseMove(POINT pos, KeyMod mod) = 0;
	virtual void mouseButton(int button, POINT pos, KeyMod mod, MouseClick click) = 0;
	virtual void mouseWheel(int delta, KeyMod mod) = 0;
	virtual void render(FrameResource& frame, ComPtr<ID3D12GraphicsCommandList> commandList) = 0;
};

struct InitParams final {
	uint width = 400;
	uint height = 400;
	WindowMode windowMode = WindowMode::WINDOWED;
	bool vsync = false;
	const wchar_t* title = L"No title";
	uint numBackBuffers = 2;
};

class DX12 final {
	static DX12* self;
	const wchar_t WINDOW_CLASS[5] = L"DX12";
	ComPtr<ID3D12Debug> debugLayer;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissor;
public:
	HINSTANCE hInstance = nullptr;
	HWND hwnd = nullptr;
	InputEventHandler* eventHandler = nullptr;
	InitParams params;
	ComPtr<IDXGIFactory5> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device2> device;
	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	Uploader uploader{*this};
	SwapChain swapChain{*this};
	Resources resources;
	ulong frameNumber;

	DX12();
	~DX12();
	bool init(const HINSTANCE hInstance,
			  const InitParams params,
			  InputEventHandler* eventHandler);
	void run();
	Dimension getWindowSize() { return {params.width, params.height}; }
	void setTitle(const wchar_t* str) {
		SetWindowText(hwnd, str);
	}
	void setIcon(const wchar_t* filename) {
		HANDLE icon = LoadImage(hInstance, filename, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
		if(icon) {
			SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);
			SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)icon);
		}
	}
	void showWindow(int cmdShow = SW_SHOWDEFAULT) {
		ShowWindow(hwnd, cmdShow);
		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}
	void showCursor(bool show) {
		ShowCursor(show);
	}
private:
	void selectAdapter(ComPtr<IDXGIFactory5> factory);
	bool createWindow();
	bool createDevice();
	void createCommandQueues();
	void createCommandLists();
	void renderFrame();
	static LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

} ///  dx12
