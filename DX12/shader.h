#pragma once

namespace dx12 {

/// TODO - make this a builder
class Shader final {
	const wchar_t* filename;
	vector<D3D_SHADER_MACRO> defines;
	vector<ID3DInclude> includes;
public:
	Shader(const wchar_t* filename) : filename(filename) {}

	/*ComPtr<ID3DBlob> read(const wchar_t* filename) {
		ComPtr<ID3DBlob> blob;
		if(FAILED(D3DReadFileToBlob(filename, &blob))) {
			log("Failed to load shader file");
		}
		return blob;
	}*/
	ComPtr<ID3DBlob> compileVS(vector<D3D_SHADER_MACRO> defines) {
		return compile("VSMain", "vs_5_0", defines);
	}
	ComPtr<ID3DBlob> compilePS(vector<D3D_SHADER_MACRO> defines) {
		return compile("PSMain", "ps_5_0", defines);
	}
	ComPtr<ID3DBlob> compileCS(vector<D3D_SHADER_MACRO> defines) {
		return compile("CSMain", "cs_5_0", defines);
	}
private:
	ComPtr<ID3DBlob> compile(const char* entry, 
							 const char* target, 
							 vector<D3D_SHADER_MACRO> defines)
	{
		defines.push_back({});
		ComPtr<ID3DBlob> blob;
		ComPtr<ID3DBlob> errors;
		auto hr = D3DCompileFromFile(
			filename,
			defines.data(), 
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			entry,
			target,
			getCompilerOptions(),
			0,
			blob.GetAddressOf(),
			errors.GetAddressOf()
		);
		if(FAILED(hr)) {
			string msg = "Shader compilation failed: " + core::Win::HRESULTToString(hr);
			if(errors) {
				msg += string((const char*)errors->GetBufferPointer(), errors->GetBufferSize());
			}
			throw std::runtime_error(msg.c_str());
		}
		return blob;
	}
	uint getCompilerOptions() const {
		uint compileOpts =
			D3DCOMPILE_ENABLE_STRICTNESS |
			D3DCOMPILE_PARTIAL_PRECISION |
			//D3DCOMPILE_AVOID_FLOW_CONTROL |
			//D3DCOMPILE_PREFER_FLOW_CONTROL |
			//D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR |
			D3DCOMPILE_OPTIMIZATION_LEVEL3 |
			D3DCOMPILE_WARNINGS_ARE_ERRORS |
			0;
#ifdef _DEBUG
		compileOpts |= D3DCOMPILE_DEBUG |
			D3DCOMPILE_SKIP_OPTIMIZATION |
			0;
#endif
		return compileOpts;
	}
};

} /// dx12