#include "_pch.h"
#include "_exported.h"

namespace dx12 {

using namespace core;

void Uploader::init() {
	Log::write("Setting up Uploader");
	Log::write("\tCreating command queue");
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	throwOnDXError(dx12.device->CreateCommandQueue(&desc, IID_PPV_ARGS(copyQueue.GetAddressOf())));

	Log::write("Creating command allocator");
	throwOnDXError(dx12.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS(copyAllocator.GetAddressOf())));

	Log::write("Creating command list");
	throwOnDXError(dx12.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COPY, copyAllocator.Get(), nullptr, IID_PPV_ARGS(copyCommandList.GetAddressOf())));
	throwOnDXError(copyCommandList->Close());

	Log::write("Creating fence");
	throwOnDXError(dx12.device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.GetAddressOf())));
	fenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
}


} /// dx12