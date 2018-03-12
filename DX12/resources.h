#pragma once

namespace dx12 {

class Resources final {
	ComPtr<ID3D12Device2> device;
public:
	void init(ComPtr<ID3D12Device2> device) {
		this->device = device;
	}

	ComPtr<ID3D12Resource> createGPUBuffer(ulong size, D3D12_RESOURCE_STATES states) {
		ComPtr<ID3D12Resource> buffer;
		//D3D12_CLEAR_VALUE cv;
		throwOnDXError(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size),
			states,
			nullptr, /// clear value
			IID_PPV_ARGS(buffer.GetAddressOf())));
		return buffer;
	}
	ComPtr<ID3D12Resource> createUploadBuffer(ulong size) const {
		ComPtr<ID3D12Resource> buffer;
		throwOnDXError(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(size),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(buffer.GetAddressOf())));
		return buffer;
	}
	void writeToBuffer(ComPtr<ID3D12Resource> buffer, const void* data, ulong size) {
		void* ptr;
		CD3DX12_RANGE readRange(0, 0);  /// No CPU read
		throwOnDXError(buffer->Map(0, &readRange, &ptr));
		memcpy(ptr, data, size);
		buffer->Unmap(0, nullptr);
	}
};

} /// dx12