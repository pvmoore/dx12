#pragma once

namespace dx12 {

class Descriptors final {
	ComPtr<ID3D12Device2> device;
public:
	Descriptors(ComPtr<ID3D12Device2> device) : device(device) {}
private:
	ComPtr<ID3D12RootSignature> createEmptyRootSignature() {
		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		throwOnDXError(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error));
		if(FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) {
			//error.GetBufferPointer(), error.GetBufferSize();
			throw std::runtime_error("Failed to create root signature");
		}
		return rootSignature;
	}
	void createRootSignature() {
		/*D3D12_ROOT_SIGNATURE_DESC1 root;
		D3D12_ROOT_PARAMETER1 params[2];
		D3D12_STATIC_SAMPLER_DESC samplers[1];
		D3D12_ROOT_SIGNATURE_FLAGS flags;
		D3D12_DESCRIPTOR_RANGE_FLAGS rangeFlags;
		D3D12_ROOT_DESCRIPTOR_FLAGS f;
		ID3DBlob* signature;
*/
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 1;
		srvRange.BaseShaderRegister = 0;	/// start from t0
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		D3D12_ROOT_PARAMETER srvParam = {};
		srvParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		srvParam.DescriptorTable = {1, &srvRange}; /// one range
		srvParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN; /// only used in domain shader

		D3D12_ROOT_PARAMETER constBuf = {};
		constBuf.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; /// constant buffer
		constBuf.Descriptor = {0, 0}; /// first register (b0) in first register space
		constBuf.ShaderVisibility = D3D12_SHADER_VISIBILITY_DOMAIN; /// only used in domain shader

		D3D12_ROOT_PARAMETER hullConstants = {};
		hullConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		hullConstants.Constants = {0, 0, 2}; /// 2 constants in first register (b0) in first register space
		hullConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_HULL; /// only used in hull shader

		vector<D3D12_ROOT_PARAMETER> rootParams{srvParam, constBuf, hullConstants};

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags{
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | /// we're using vertex and index buffers
			D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
		};

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.NumParameters = (uint)(rootParams.size());
		rootSignatureDesc.pParameters = rootParams.data();
		rootSignatureDesc.NumStaticSamplers = 0; // samplers can be stored in root signature separately and consume no space
		rootSignatureDesc.pStaticSamplers = nullptr; // we're not using texturing
		rootSignatureDesc.Flags = rootSignatureFlags;

		ID3DBlob* signature;
		ID3DBlob* error;
		if(FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error))) {

		}
		//if(FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)))) {

		//}
	}
	/// Depth/Stencil View
	void createDSV() {
		/*
		D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
		dsvDescriptorHeapDesc.NumDescriptors = 1;
		dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		check(device->CreateDescriptorHeap(&dsvDescriptorHeapDesc, IID_PPV_ARGS(&dsvHeap)));

		//CD3DX12_HEAP_PROPERTIES depthHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_HEAP_PROPERTIES depthHeapProperties = {};

		D3D12_RESOURCE_DESC depthStencilDesc = {};
		//depthStencilDesc.Format = back buffer format
		//depthStencilDesc.Width = back buffer width
		//depthStencilDesc.Height = back buffer height
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.DepthOrArraySize = 1;
		depthStencilDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		//depthOptimizedClearValue.Format = depth buffer format
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		ID3D12Resource* depthStencil;
		device->CreateCommittedResource(
			&depthHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&depthStencil)
		));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = m_depthBufferFormat;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

		device->CreateDepthStencilView(depthStencil.Get(), &dsvDesc, m_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		*/

	}
};
} /// dx12
