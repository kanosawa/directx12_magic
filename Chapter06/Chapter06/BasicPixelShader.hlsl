#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	// �e�N�X�`��tex����T���v���[sap���g����uv�̐F���T���v������
	return float4(tex.Sample(smp, input.uv));
}