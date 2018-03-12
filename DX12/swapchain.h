#pragma once

namespace dx12 {

struct FrameResource final {
	ComPtr<ID3D12Resource> renderTarget;
	ComPtr<ID3D12CommandAllocator> allocator;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
	ulong fenceValue;
	ulong number;
};

class SwapChain final {
	const DX12& dx12;
	ComPtr<IDXGISwapChain4> swapChain;
	ComPtr<ID3D12DescriptorHeap> rtvHeap;
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12Fence> fence;
	HANDLE event = nullptr;
	vector<FrameResource> frameResources;
	uint frameIndex;
	bool allowTearing = false;
public:
	SwapChain(const DX12& dx12) : dx12(dx12) {}
	~SwapChain() {
		if(swapChain) {
			swapChain->SetFullscreenState(false, nullptr);
		}
		if(event) CloseHandle(event);
	}
	void init(uint numBuffers = 2);
	void create();
	void waitForGPU();
	inline FrameResource& currentFrame() { return frameResources[frameIndex]; }
	void present();
};

} /// dx12