#pragma once
///	eg.
///	struct MyConstants {
///		XMFLOAT4 value;
///	};
///	ConstantBuffer<MyConstants> buffer;
///	buffer.data.value = XMFLOAT4(2);
///	buffer.setup(dx12, heap);
///
///	buffer.data.value = XMFLOAT4(3);
///	buffer.update();
namespace dx12 {

template<class T, unsigned index>
class ConstantBuffer final {
	ComPtr<ID3D12Resource> buffer;
	uint cbvDescriptorSize;
public:
	T data;
	ubyte* mapPtr;

	void init(const DX12& dx12, ComPtr<ID3D12DescriptorHeap> cbvHeap) {
		this->cbvDescriptorSize = dx12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		uint size = (sizeof(T) + 255) & ~255; // Align to 256 bytes

		buffer = dx12.resources.createUploadBuffer(size);

		/// Describe and create a constant buffer view.
		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = size;

		auto cbvHandle = cbvHeap->GetCPUDescriptorHandleForHeapStart();
		cbvHandle.ptr += (cbvDescriptorSize * index);
		dx12.device->CreateConstantBufferView(&cbvDesc, cbvHandle);

		/// Map constant buffer permanently and write initial data
		CD3DX12_RANGE readRange(0, 0);
		throwOnDXError(buffer->Map(0, &readRange, (void**)&mapPtr));
		update();
	}
	void update() {
		memcpy(mapPtr, &data, sizeof(T));
	}
};

} /// dx12