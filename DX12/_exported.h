#pragma once

#if NDEBUG
#define _RELEASE
#else
#undef _RELEASE
#endif

#include <d3d12.h>	
#include <dxgi1_6.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "../External/stb_image.h"
#include "../External/d3dx12.h"	

// Global typedefs
typedef signed char sbyte;
typedef unsigned char ubyte;
typedef unsigned int uint;
typedef unsigned long long ulong;
typedef signed long long slong;

namespace dx12 {
struct Dimension final {
	uint width, height;
};
}

using Microsoft::WRL::ComPtr;

#include "utils.h"
#include "uploader.h"
#include "image_loader.h"
#include "constant_buffer.h"
#include "texture_heap.h"
#include "resources.h"
#include "shader.h"
#include "pipeline.h"
#include "swapchain.h"
#include "descriptors.h"
#include "dx12.h"
