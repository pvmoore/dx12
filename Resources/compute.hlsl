// SV_POSITION  - 
// SV_TARGET	- 

struct CSInput {
	float4 position : SV_POSITION;
	float2 uv		: TEXCOORD;
};

cbuffer ConstantBuffer : register(b0) {
	float red;
};

Texture2D g_texture    : register(t0);
SamplerState g_sampler : register(s0);

void CSMain(CSInput input) {

}
