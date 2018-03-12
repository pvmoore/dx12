#pragma once

namespace dx12 {

class TextureHeap final {
public:
	enum Channels : uint { R = 1, RGBA = 4 };
private:
	struct Info final {
		string filename;
		uint descriptorIndex;
		Channels channels;
		ulong offset;
		D3D12_RESOURCE_DESC desc;
		D3D12_RESOURCE_ALLOCATION_INFO allocInfo;
		PNG png;
		ComPtr<ID3D12Resource> resource;
	};
	DX12* dx12 = nullptr;
	string name;
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	uint descriptorSize = 0;
	string baseDir = "";
	ComPtr<ID3D12Heap> heap;
	vector<Info> textures;
public:
	TextureHeap(string name) : name(name) {}
	TextureHeap& setBaseDir(string bd) { baseDir = bd; return *this; }
	TextureHeap& init(DX12* dx12, ComPtr<ID3D12DescriptorHeap> srvHeap);
	TextureHeap& add(string filename, uint descriptorIndex, Channels channels=Channels::RGBA);
	TextureHeap& uploadAll();
};

} /// dx12