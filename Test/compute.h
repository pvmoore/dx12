#pragma once

class ComputeTest : public BaseTest {
private:
	enum RootParameters : uint {
		UNKNOWN
	};
	enum DescriptorIndex : uint {
		CONSTANT_BUFFER,
		length	
	};
	struct Constants final {
		float value;
	};
	ComPtr<ID3D12PipelineState> computePipelineState;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;	// cbvSrvUav
	uint descriptorSize;

	ConstantBuffer<Constants, DescriptorIndex::CONSTANT_BUFFER> constantBuffer;
public:
	void init(HINSTANCE hInstance, int cmdShow) final override {
		params.title = L"DX12 Compute Test";
		params.width = 1000;
		params.height = 600;
		params.windowMode = WindowMode::WINDOWED;
		params.vsync = false;
		params.numBackBuffers = 3;
		BaseTest::init(hInstance, cmdShow);
	}
	void setup() final override {
		Log::write("Application setup");
		this->descriptorSize = dx12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = DescriptorIndex::length;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		throwOnDXError(dx12.device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(descriptorHeap.GetAddressOf())));
		
		constantBuffer.data.value = 0;
		constantBuffer.init(dx12, descriptorHeap);

		setupRootSignature();
		setupComputePipeline();

		Log::write("Finished Application setup");
	}
	void render(FrameResource& frame, ComPtr<ID3D12GraphicsCommandList> commandList) final override {

	}
private:
	void setupComputePipeline() {

	}
	void setupRootSignature() {

	}
};