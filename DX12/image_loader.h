#pragma once

namespace dx12 {

struct PNG final {
	uint width, height, bytesPerPixel;
	ubyte* data = nullptr;

	ulong sizeBytes() const { return width*height*bytesPerPixel; }
	void free() {
		if(data) ::free(data);
		data = nullptr;
	}
};
struct DDS final {
	uint width, height, bytesPerPixel;
	ubyte* data;
	ComPtr<ID3D12Resource> texture;
};

class ImageLoader final {
public:
	/// Set bpp to 4 to force 4 bytes per pixel.
	/// The caller needs to free the malloced data.
	static PNG loadPNG(string filename, uint bpp = 0) {
		PNG png;
		int numBytes;
		png.data = stbi_load(filename.c_str(), (int*)&png.width, (int*)&png.height, &numBytes, bpp);
		png.bytesPerPixel = bpp==0 ? numBytes : bpp;
		return png;
	}
	static DDS loadDDS(const DX12& dx12, wstring filename);
};

} ///  dx12