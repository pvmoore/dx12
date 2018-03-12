#pragma once
///	Upload textures and buffers to the GPU.
namespace dx12 {
class DX12;

class Uploader final {
	const DX12& dx12;
	ComPtr<ID3D12CommandQueue> copyQueue;
	ComPtr<ID3D12CommandAllocator> copyAllocator;
	ComPtr<ID3D12GraphicsCommandList> copyCommandList;
	ComPtr<ID3D12Fence> fence;
	ulong fenceValue = 1;
	HANDLE fenceEvent = nullptr;
public:
	Uploader(const DX12& dx12) : dx12(dx12) {}
	~Uploader() { if(fenceEvent) CloseHandle(fenceEvent);  }

	void init();

	ComPtr<ID3D12GraphicsCommandList> prepare() {
		throwOnDXError(copyAllocator->Reset());
		throwOnDXError(copyCommandList->Reset(copyAllocator.Get(), nullptr));
		return copyCommandList;
	}
	void upload() {
		ID3D12CommandList* ppCommandLists[] = {copyCommandList.Get()};
		copyQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

		/// Wait for the upload to finish
		auto currentFenceValue = fenceValue;
		throwOnDXError(copyQueue->Signal(fence.Get(), currentFenceValue));
		throwOnDXError(fence->SetEventOnCompletion(currentFenceValue, fenceEvent));
		WaitForSingleObjectEx(fenceEvent, INFINITE, FALSE);
		fenceValue++;
	}
};

} /// dx12