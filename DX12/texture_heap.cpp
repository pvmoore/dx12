#include "_pch.h"
#include "_exported.h"

namespace dx12 {

using namespace core;

TextureHeap& TextureHeap::init(DX12* dx12, ComPtr<ID3D12DescriptorHeap> srvHeap) {
	this->dx12 = dx12;
	this->srvHeap = srvHeap;
	this->descriptorSize = dx12->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return *this;
}
TextureHeap& TextureHeap::add(string filename, uint descriptorIndex, Channels channels) {
	textures.push_back({filename,descriptorIndex,channels});
	return *this;
}
TextureHeap& TextureHeap::uploadAll() {
	Log::format("[%s] Uploading all textures", name.c_str());
	high_resolution_clock clock;
	
	ulong totalSize = 0;
	for(auto& info : textures) {
		string filename    = baseDir + info.filename;
		DXGI_FORMAT format = info.channels == Channels::RGBA ? DXGI_FORMAT_R8G8B8A8_UNORM : 
															   DXGI_FORMAT_R8_UNORM;

		auto png = ImageLoader::loadPNG(filename, info.channels);	
		if(png.data) {
			Log::format("\tTexture '%s' loaded", filename.c_str());
		} else {
			if(png.width==0 || png.width>16384) png.width = 64;
			if(png.height==0 || png.height>16384) png.height = 64;
			if(png.bytesPerPixel==0) png.bytesPerPixel = 4;
			png.data = (ubyte*)malloc(png.sizeBytes());
			Log::format("\tTexture '%s' FAILED! Creating as blank", filename.c_str());
		}
		Log::format("\t\t%ux%u %u channels (%u bytes)",
			png.width, png.height, png.bytesPerPixel,
			png.width*png.height*png.bytesPerPixel);

		info.png = png;

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.MipLevels = 1;
		textureDesc.Format = format;
		textureDesc.Width = png.width;
		textureDesc.Height = png.height;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Alignment = 0;

		info.desc = textureDesc;
		// Get the required size and alignment
		info.allocInfo = dx12->device->GetResourceAllocationInfo(0, 1, &textureDesc);
		Log::format("\t\tSize %llu align %llu", info.allocInfo.SizeInBytes, info.allocInfo.Alignment);
		// for now, assume alignment is 64k
		assert(info.allocInfo.Alignment==65536);
		totalSize += info.allocInfo.SizeInBytes;
	}
	Log::format("\tCreating heap of %llu bytes", totalSize);
	CD3DX12_HEAP_DESC heapDesc(totalSize,
							   D3D12_HEAP_TYPE_DEFAULT, 0,
							   D3D12_HEAP_FLAG_DENY_BUFFERS
							   | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES);

	throwOnDXError(dx12->device->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));

	ulong offset = 0;
	vector<D3D12_RESOURCE_BARRIER> barriers;
	// Place the textures into the heap
	for(auto& info : textures) {
		// Adjust offset to be properly aligned
		offset += info.allocInfo.Alignment-1;
		offset &= ~(info.allocInfo.Alignment-1);

		assert(offset%info.allocInfo.Alignment==0);
		info.offset = offset;

		Log::format("\tPlacing texture at offset %lld (%llu * 64K)", offset, offset/65536);
		throwOnDXError(dx12->device->CreatePlacedResource(
			heap.Get(),
			offset,
			&info.desc,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(info.resource.GetAddressOf())));

		offset += info.allocInfo.SizeInBytes;

		barriers.push_back(CD3DX12_RESOURCE_BARRIER::Aliasing(nullptr, info.resource.Get()));
	}

	auto copyCommandList = dx12->uploader.prepare();

	copyCommandList->ResourceBarrier((uint)barriers.size(), barriers.data());
	auto start = clock.now();
	vector<ComPtr<ID3D12Resource>> uploadBuffers;
	Log::write("\tUploading textures to GPU");
	for(auto& info : textures) {
		ulong uploadBufferSize = GetRequiredIntermediateSize(info.resource.Get(), 0, 1) + D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;

		ComPtr<ID3D12Resource> uploadBuffer =
			dx12->resources.createUploadBuffer(uploadBufferSize);
		/// Keep the handle so that it is not deleted before we do the upload
		uploadBuffers.push_back(uploadBuffer);

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = info.png.data;
		textureData.RowPitch = info.png.width * info.png.bytesPerPixel;
		textureData.SlicePitch = textureData.RowPitch * info.png.height;

		UpdateSubresources<1>(copyCommandList.Get(), info.resource.Get(), uploadBuffer.Get(), 0, 0, 1, &textureData);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = info.desc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = info.desc.MipLevels;

		auto cpuHandle = srvHeap->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += (info.descriptorIndex*descriptorSize);

		dx12->device->CreateShaderResourceView(info.resource.Get(), &srvDesc, cpuHandle);
	}
	copyCommandList->Close();
	dx12->uploader.upload();

	/// Free the memory now that everything is uploaded to the GPU
	for(auto& info : textures) {
		info.png.free();
	}
	auto elapsed = clock.now() - start;
	Log::format("[%s] finished. Took %.2f ms to upload", name.c_str(), elapsed.count()/1e6);
	return *this;
}

} /// dx12