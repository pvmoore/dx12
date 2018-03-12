#include "_pch.h"
#include "_exported.h"

namespace dx12 {

using namespace core;

void SwapChain::init( uint numBuffers) {
	this->frameResources.resize(numBuffers);
}
void SwapChain::create() {
	Log::write("Creating fence");
	throwOnDXError(dx12.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf())));
	event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

	Log::write("Creating swap chain");
	auto commandQueue = dx12.commandQueue;
	auto device = dx12.device;

	//auto screenWidth = (GetSystemMetrics(SM_CXSCREEN));
	//auto screenHeight = (GetSystemMetrics(SM_CYSCREEN));
	//uint numerator = 0, denominator = 0;

	//log("\tScreen = %ux%u", screenWidth, screenHeight);

	/*auto displayModes = helper.getDisplayModes();
	log("\tFound %d display modes", displayModes.size());
	for(auto& m : displayModes) {
		if(m.Height == screenHeight) {
			if(m.Width == screenWidth) {
				log("\tFound display mode (%dx%d) format: %u scaling: %u RefreshRate: %u/%u (%.2f Hz)",
					m.Width, m.Height,
					m.Format, m.Scaling,
					m.RefreshRate.Numerator,
					m.RefreshRate.Denominator,
					(double)m.RefreshRate.Numerator/m.RefreshRate.Denominator
				);
				numerator = m.RefreshRate.Numerator;
				denominator = m.RefreshRate.Denominator;
			}
		}
	}
	log("\tRefresh rate = %u/%u (%.2f Hz)", numerator, denominator, (double)numerator/denominator);
	
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Height = dx12->params.width;
		swapChainDesc.BufferDesc.Width = dx12->params.height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.OutputWindow = dx12->hwnd;
		swapChainDesc.Windowed = !dx12->params.fullScreen;
		if(dx12->params.vsync) {
			swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
		} else {
			swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
			swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		}
		// Turn multisampling off.
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		// Set the scan line ordering and scaling to unspecified.
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		// Don't set the advanced flags.
		swapChainDesc.Flags = 0;
		*/
	if(!dx12.params.vsync) {
		int buf = 0;
		if(FAILED(dx12.factory->CheckFeatureSupport(
			DXGI_FEATURE_PRESENT_ALLOW_TEARING,
			&buf, sizeof(buf)))) {
		}
		this->allowTearing = buf != 0;
		Log::format("\tAllow tearing = %s", allowTearing ? "true" : "false");
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = (uint)frameResources.size();
	swapChainDesc.Width = dx12.params.width;
	swapChainDesc.Height = dx12.params.height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = FALSE;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

	//DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
	//IDXGIOutput output;

	ComPtr<IDXGISwapChain1> swapChain1;
	throwOnDXError(dx12.factory->CreateSwapChainForHwnd(commandQueue.Get(), dx12.hwnd, &swapChainDesc, nullptr, nullptr, swapChain1.GetAddressOf()));
	// Upgrade to a IDXGISwapChain4
	swapChain1.As(&swapChain);

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = (uint)frameResources.size();
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	// Create the render target view heap for the back buffers.
	throwOnDXError(dx12.device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));

	// Get a handle to the starting memory location in the render target view 
	// heap to identify where the render target views will be located for the back buffers.
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHeap->GetCPUDescriptorHandleForHeapStart();

	// Get the size of the memory location for the render target view descriptors.
	uint rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// Create the frame resources
	Log::format("\tCreating %u back buffers of size (%ux%u)", (uint)frameResources.size(), dx12.params.width, dx12.params.height);
	for(auto i = 0; i<frameResources.size(); i++) {
		throwOnDXError(swapChain->GetBuffer(i, IID_PPV_ARGS(frameResources[i].renderTarget.GetAddressOf())));
		device->CreateRenderTargetView(frameResources[i].renderTarget.Get(), nullptr, rtvHandle);

		throwOnDXError(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(frameResources[i].allocator.GetAddressOf())));

		frameResources[i].rtvHandle = rtvHandle;

		frameResources[i].fenceValue = 1;

		rtvHandle.ptr += rtvDescriptorSize;
	}
	// Get the initial index to which buffer is the current back buffer.
	frameIndex = swapChain->GetCurrentBackBufferIndex();

	// Disallow transition to fullscreen
	dx12.factory->MakeWindowAssociation(dx12.hwnd, DXGI_MWA_NO_ALT_ENTER);
}
void SwapChain::present() {
	// Present the back buffer to the screen since rendering is complete.
	HRESULT result;
	if(dx12.params.vsync) {
		// Lock to screen refresh rate.
		result = swapChain->Present(1, 0);
	} else {
		// Present as fast as possible.
		result = swapChain->Present(0, allowTearing ? DXGI_PRESENT_ALLOW_TEARING : 0);
	}
	// If the device was reset we must completely reinitialize the renderer.
	if(result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET) {
		// todo
		Log::write("Present - device lost");
	}

	// Schedule a Signal command in the queue
	auto currentFenceValue = currentFrame().fenceValue;
	throwOnDXError(dx12.commandQueue->Signal(fence.Get(), currentFenceValue));

	// Move to next frame
	frameIndex = swapChain->GetCurrentBackBufferIndex();
	auto nextFenceValue = currentFrame().fenceValue;

	// If the next frame is not ready to be rendered yet, wait until it is ready
	if(fence->GetCompletedValue() < nextFenceValue) {
		throwOnDXError(fence->SetEventOnCompletion(nextFenceValue, event));
		WaitForSingleObjectEx(event, INFINITE, FALSE);
	}

	currentFrame().fenceValue = currentFenceValue+1;
}
void SwapChain::waitForGPU() {
	auto currentFenceValue = currentFrame().fenceValue;
	// Schedule a Signal command in the queue
	throwOnDXError(dx12.commandQueue->Signal(fence.Get(), currentFenceValue));

	// Wait until the fence has been processed
	throwOnDXError(fence->SetEventOnCompletion(currentFenceValue, event));
	WaitForSingleObjectEx(event, INFINITE, FALSE);

	// Increment the fence value for the current frame
	currentFrame().fenceValue++;
}

} // namespace dx12