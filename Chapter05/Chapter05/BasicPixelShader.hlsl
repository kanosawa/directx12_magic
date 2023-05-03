#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	// テクスチャtexからサンプラーsapを使ってuvの色をサンプルする
	return float4(tex.Sample(smp, input.uv));
}