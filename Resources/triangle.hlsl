// SV_POSITION  - 
// SV_TARGET	- 
cbuffer ConstantBuffer : register(b0) {
	float b0_red;
};
cbuffer ConstantBuffer : register(b1) {
	float b1_green;
};
cbuffer ConstantBuffer : register(b2) {
	float b2_blue;
};

struct VSInput {
	float3 position	: POSITION;
	float4 color	: COLOR;
	float2 uv		: TEXCOORD;
};
struct PSInput {
	float4 position : SV_POSITION;
	float4 color	: COLOR;
	float2 uv	    : TEXCOORD;
};

Texture2D g_texture    : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(VSInput input) {
	PSInput result;
	result.position = float4(input.position, 1);
	result.color = float4(b0_red, b1_green, b2_blue, 1);
	result.uv = input.uv;
	return result;
}
float4 PSMain(PSInput input) : SV_TARGET {
	return g_texture.Sample(g_sampler, input.uv) * input.color;
}
