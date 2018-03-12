#pragma once
/*
	Demonstrates the following:

	- Drawing a textured triangle using a vertex buffer.
	- Creating a root signature with 3 separate constant buffers.
	- Command list bundles.
*/
class TriangleTest : public BaseTest {
private:
	ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12PipelineState> pipelineState;
	ComPtr<ID3D12RootSignature> rootSignature;

	ComPtr<ID3D12GraphicsCommandList> bundle;
	ComPtr<ID3D12CommandAllocator> bundleAllocator;

	ComPtr<ID3D12DescriptorHeap> cbvSrvHeap;
	uint cbvSrvDescriptorSize;
	ComPtr<ID3D12Resource> gpuTexture;

	enum RootParameters : uint {
		CBV,
		SRV
	};
	enum DescriptorIndex : uint {
		CONSTANT_BUFFER0,
		CONSTANT_BUFFER1,
		CONSTANT_BUFFER2,
		TEXTURE0,
		TEXTURE1,
		TEXTURE2,
		length	// num elements in enum
	};
	struct Vertex final {
		XMFLOAT3 position;
		XMFLOAT4 color;
		XMFLOAT2 uv;
	};
	struct Constants final {
		float red;
	};
	struct Constants2 final {
		float green;
	};
	struct Constants3 final {
		float blue;
	};
	ConstantBuffer<Constants, DescriptorIndex::CONSTANT_BUFFER0> constantBuffer;
	ConstantBuffer<Constants2, DescriptorIndex::CONSTANT_BUFFER1> constantBuffer2;
	ConstantBuffer<Constants3, DescriptorIndex::CONSTANT_BUFFER2> constantBuffer3;

	TextureHeap textureHeap{"Lovely Textures"};
public:
	void init(HINSTANCE hInstance, int cmdShow) final override {
		params.title = L"DX12 Triangle Test";
		params.width = 1000;
		params.height = 600;
		params.windowMode = WindowMode::WINDOWED;
		params.vsync = false;
		params.numBackBuffers = 3;
		BaseTest::init(hInstance, cmdShow);
	}
	void setup() final override {
		Log::write("Application setup");
		this->cbvSrvDescriptorSize = dx12.device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		setupVertices();
		createHeaps();
		createConstantBuffers();
		uploadTextures();
		setupRootSignature();
		setupPipelineState();
		createBundle();
		Log::write("Finished Application setup");
	}
	void render(FrameResource& frame, ComPtr<ID3D12GraphicsCommandList> commandList) final override {
		updateConstantBuffers(frame);

		// Parent command list needs the heaps to be set
		ID3D12DescriptorHeap* ppHeaps[] = {cbvSrvHeap.Get()};
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		// Execute our bundle
		commandList->ExecuteBundle(bundle.Get());

		/*
		// The same thing without using the bundle
		commandList->SetPipelineState(pipelineState.Get());
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = {cbvSrvHeap.Get()};
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		auto handle = cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
		commandList->SetGraphicsRootDescriptorTable(0, handle);
		handle.ptr += 3*cbvSrvDescriptorSize;
		commandList->SetGraphicsRootDescriptorTable(1, handle);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->DrawInstanced(3, 1, 0, 0);
		*/
	}
private:
	void updateConstantBuffers(FrameResource& frame) {
		ulong number = frame.number/32;
		float dir    = (float)((number/256)&1);
		float f      = (number % 256) / 255.0f;
		float red    = abs(dir-f);
		float green  = abs(dir-f);
		float blue   = abs(dir-f);
		constantBuffer.data.red = red;
		constantBuffer2.data.green = green;
		constantBuffer3.data.blue = blue;

		constantBuffer.update();
		constantBuffer2.update();
		constantBuffer3.update();
	}
	/// Create an empty root signature
	void setupRootSignature() {
		Log::write("\tSetting up root signature");
		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if(FAILED(dx12.device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData)))) {
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}
		Log::format("\tUsing root signature version %.1f",
			featureData.HighestVersion==D3D_ROOT_SIGNATURE_VERSION_1_1 ? 1.1 : 1.0);

		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 3, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

		// Allow input layout and deny uneccessary access to certain pipeline stages.
		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, rootSignatureFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> errors;
		auto hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &errors);
		if(FAILED(hr)) {
			Log::format("D3DX12SerializeVersionedRootSignature failed %s", Win::HRESULTToString(hr).c_str());
			if(errors) {
				Log::format("%s", errors->GetBufferPointer());
			}
		}
		throwOnDXError(dx12.device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
	}
	void createHeaps() {
		// Flags indicate that this descriptor heap can be bound to the pipeline 
		// and that descriptors contained in it can be referenced by a root table.
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = DescriptorIndex::length;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		throwOnDXError(dx12.device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(cbvSrvHeap.GetAddressOf())));
	}
	void uploadTextures() {
		textureHeap.init(&dx12, cbvSrvHeap)
			.setBaseDir("../Resources/")
			.add("birds.png", DescriptorIndex::TEXTURE0)
			.add("bat.png", DescriptorIndex::TEXTURE1)
			.add("jimi.png", DescriptorIndex::TEXTURE2)
			.uploadAll();
	}
	void createBundle() {
		// Create bundle allocator
		throwOnDXError(dx12.device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(bundleAllocator.GetAddressOf())));

		// Create bundle
		throwOnDXError(dx12.device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, bundleAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(bundle.GetAddressOf())));
		
		// Set root signature and heap
		bundle->SetGraphicsRootSignature(rootSignature.Get());
		ID3D12DescriptorHeap* ppHeaps[] = {cbvSrvHeap.Get()};
		bundle->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		auto handle = cbvSrvHeap->GetGPUDescriptorHandleForHeapStart();
		bundle->SetGraphicsRootDescriptorTable(RootParameters::CBV, handle); 
		handle.ptr += (3*cbvSrvDescriptorSize);
		bundle->SetGraphicsRootDescriptorTable(RootParameters::SRV, handle);

		bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		bundle->IASetVertexBuffers(0, 1, &vertexBufferView);
		bundle->DrawInstanced(3, 1, 0, 0);

		throwOnDXError(bundle->Close());
	}
	void createConstantBuffers() {
		constantBuffer.data.red = 0;
		constantBuffer.init(dx12, cbvSrvHeap);

		constantBuffer2.data.green = 0;
		constantBuffer2.init(dx12, cbvSrvHeap);

		constantBuffer3.data.blue = 0;
		constantBuffer3.init(dx12, cbvSrvHeap);
	}
	void setupPipelineState() {
		Shader shader{L"../Resources/triangle.hlsl"};
		ComPtr<ID3DBlob> vertexShader = shader.compileVS({});
		ComPtr<ID3DBlob> pixelShader = shader.compilePS({});
		assert(vertexShader && "vertexShader is null");
		assert(pixelShader && "pixelShader is null");

		const auto FLOAT2 = DXGI_FORMAT_R32G32_FLOAT;
		const auto FLOAT3 = DXGI_FORMAT_R32G32B32_FLOAT;
		const auto FLOAT4 = DXGI_FORMAT_R32G32B32A32_FLOAT;

		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] = {
			{"POSITION", 0, FLOAT3, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR",    0, FLOAT4, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, FLOAT2, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = {inputElementDescs, _countof(inputElementDescs)};
		psoDesc.pRootSignature = rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE;
		psoDesc.DepthStencilState.StencilEnable = FALSE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.SampleDesc.Count = 1;
		throwOnDXError(dx12.device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.GetAddressOf())));
	}
	void setupVertices() {
		const float aspectRatio = (float)params.width/params.height;
		const Vertex vertices[] = {
			{{0.0f, 0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.5f, 0.0f}},   // red
			{{0.25f, -0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // green
			{{-0.25f, -0.25f * aspectRatio, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}} // blue
		};
		const uint vertexBufferSize = sizeof(vertices);

		// TODO - make this more efficient
		vertexBuffer = dx12.resources.createUploadBuffer(vertexBufferSize);

		dx12.resources.writeToBuffer(vertexBuffer, vertices, vertexBufferSize);

		// Initialize the vertex buffer view.
		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.StrideInBytes = sizeof(Vertex);
		vertexBufferView.SizeInBytes = vertexBufferSize;
	}
};