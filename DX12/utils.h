#pragma once

namespace dx12 {

//====================================================================================
struct Resolution final {
	uint width, height, frequency, bits;
};

const Resolution getScreenResolution();
//====================================================================================
void throwOnDXError(HRESULT hr, const char* msg = nullptr);
D3D_FEATURE_LEVEL getMaxSupportedFeatureLevel(ComPtr<ID3D12Device2> device);
//====================================================================================

} /// dx12