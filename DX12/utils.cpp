#include "_pch.h"
#include "_exported.h"
/**
	Gets the real resolution of the current display regardless of scaling.
*/
namespace dx12 {

using namespace core;

const Resolution getScreenResolution() {
	DEVMODE dm = {};
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(
		nullptr,
		ENUM_CURRENT_SETTINGS,
		&dm
	);
	return {dm.dmPelsWidth, dm.dmPelsHeight, dm.dmDisplayFrequency, dm.dmBitsPerPel};
}
//====================================================================================
void throwOnDXError(HRESULT hr, const char* msg) {
	if(FAILED(hr)) {
		string m;
		if(msg) m += msg;
		Log::format("FAILURE!! %s", m.c_str());
		throw std::runtime_error(String::format("An error occurred: %s", m.c_str()));
	}
}
D3D_FEATURE_LEVEL getMaxSupportedFeatureLevel(ComPtr<ID3D12Device2> device) {
	// Determine maximum supported feature level for this device
	static const D3D_FEATURE_LEVEL s_featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};
	D3D12_FEATURE_DATA_FEATURE_LEVELS featLevels = {
		_countof(s_featureLevels), s_featureLevels, D3D_FEATURE_LEVEL_11_0
	};
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	if(SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &featLevels, sizeof(featLevels)))) {
		featureLevel = featLevels.MaxSupportedFeatureLevel;
	}
	return featureLevel;
}

} /// dx12