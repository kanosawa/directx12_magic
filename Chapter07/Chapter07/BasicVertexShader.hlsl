#include "BasicShaderHeader.hlsli"

Output BasicVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD, min16uint2 boneno : BONE_NO, min16uint weight : WEIGHT) {
	Output output;
	output.svpos = mul(mul(viewproj, world), pos);
	normal.w = 0;
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}