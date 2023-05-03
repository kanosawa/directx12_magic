#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	return float4(tex.Sample(smp, input.uv));
}