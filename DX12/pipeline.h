#pragma once

namespace dx12 {

class Pipeline {
	ComPtr<ID3D12Device2> device;
	ID3D12RootSignature* rootSignature;
	ComPtr<ID3D12PipelineState> pipelineState;
	vector<D3D12_INPUT_ELEMENT_DESC> inputs;
	D3D12_RASTERIZER_DESC rasterizerDesc;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D12_BLEND_DESC blendDesc;
	D3D12_PRIMITIVE_TOPOLOGY_TYPE topology;
	ID3DBlob* vertexShaderBlob;
	ID3DBlob* hullShaderBlob;
	ID3DBlob* domainShaderBlob;
	ID3DBlob* geometryShaderBlob;
	ID3DBlob* pixelShaderBlob;
public:
	Pipeline(ComPtr<ID3D12Device2> device, ID3D12RootSignature* rootSignature)
		: device(device), 
		  rootSignature(rootSignature) 
	{
		setDefaults();
	}

	inline auto withTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE type) {
		this->topology = type;
		return this;
	}
	inline auto withInputElements(vector<D3D12_INPUT_ELEMENT_DESC> inputs) {
		this->inputs = inputs;
		return this;
	}
	inline auto withRasterizerDesc(D3D12_RASTERIZER_DESC rasterizerDesc) {
		this->rasterizerDesc = rasterizerDesc;
		return this;
	}
	inline auto withDepthStencilDesc(D3D12_DEPTH_STENCIL_DESC depthStencilDesc) {
		this->depthStencilDesc = depthStencilDesc;
		return this;
	}
	inline auto withBlendDesc(D3D12_BLEND_DESC blendDesc) {
		this->blendDesc = blendDesc;
		return this;
	}
	inline auto withVertexShader(ID3DBlob* blob) {
		this->vertexShaderBlob = blob;
		return this;
	}

	ComPtr<ID3D12PipelineState> create() {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};
		pipelineStateDesc.InputLayout = {inputs.data(), (uint)(inputs.size())};
		pipelineStateDesc.pRootSignature = rootSignature;
		if(vertexShaderBlob) {
			pipelineStateDesc.VS = {vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize()};
		}
		if(hullShaderBlob) {
			pipelineStateDesc.HS = {hullShaderBlob->GetBufferPointer(), hullShaderBlob->GetBufferSize()};
		}
		if(domainShaderBlob) {
			pipelineStateDesc.DS = {domainShaderBlob->GetBufferPointer(), domainShaderBlob->GetBufferSize()};
		}
		if(geometryShaderBlob) {
			pipelineStateDesc.GS = {geometryShaderBlob->GetBufferPointer(), geometryShaderBlob->GetBufferSize()};
		}
		if(pixelShaderBlob) {
			pipelineStateDesc.PS = {pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize()};
		}
		pipelineStateDesc.RasterizerState = rasterizerDesc;
		pipelineStateDesc.BlendState = blendDesc;
		pipelineStateDesc.DepthStencilState = depthStencilDesc;
		pipelineStateDesc.SampleMask = UINT_MAX;
		pipelineStateDesc.PrimitiveTopologyType = topology;
		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		pipelineStateDesc.SampleDesc.Count = 1;

		throwOnDXError(device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf())));
		return pipelineState;
	}
private:
	void setDefaults() {
		/// for debugging purposes
		inputs = {
			/// name,   index,  format,                  slot, byteoffset, slotclass, instance data step rate
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
		};

		rasterizerDesc = {};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
		rasterizerDesc.FrontCounterClockwise = FALSE;
		rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDesc.DepthClipEnable = FALSE;
		rasterizerDesc.MultisampleEnable = FALSE;
		rasterizerDesc.AntialiasedLineEnable = FALSE;
		rasterizerDesc.ForcedSampleCount = 0;
		rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		depthStencilDesc = {};
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		depthStencilDesc.StencilEnable = FALSE;
		depthStencilDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		depthStencilDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		static const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = {
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_STENCIL_OP_KEEP,
			D3D12_COMPARISON_FUNC_ALWAYS
		};
		depthStencilDesc.FrontFace = defaultStencilOp;
		depthStencilDesc.BackFace = defaultStencilOp;

		CD3DX12_BLEND_DESC(D3D12_DEFAULT);

		blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0] = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	}
};

} /// dx12