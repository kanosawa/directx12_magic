#include"BasicType.hlsli"

//�萔�o�b�t�@0
cbuffer SceneData : register(b0) {
	matrix world;//���[���h�ϊ��s��
	matrix view;
	matrix proj;//�r���[�v���W�F�N�V�����s��
	float3 eye;
};

//���_�V�F�[�_���s�N�Z���V�F�[�_�ւ̂����Ɏg�p����
//�\����
struct Output {
	float4 svpos:SV_POSITION;//�V�X�e���p���_���W
	float4 pos:POSITION;//�V�X�e���p���_���W
	float4 normal:NORMAL0;//�@���x�N�g��
	float4 vnormal:NORMAL1;//�@���x�N�g��
	float2 uv:TEXCOORD;//UV�l
	float3 ray:VECTOR;//�x�N�g��
};

BasicType BasicVS(float4 pos : POSITION , float4 normal : NORMAL, float2 uv : TEXCOORD) {
	BasicType output;//�s�N�Z���V�F�[�_�֓n���l
	pos = mul(world, pos);
	output.svpos = mul(mul(proj,view),pos);//�V�F�[�_�ł͗�D��Ȃ̂Œ���
	output.pos = mul(view, pos);
	normal.w = 0;//�����d�v(���s�ړ������𖳌��ɂ���)
	output.normal = mul(world,normal);//�@���ɂ����[���h�ϊ����s��
	output.vnormal = mul(view, output.normal);
	output.uv = uv;
	output.ray = normalize(pos.xyz - mul(view,eye));//�����x�N�g��

	return output;
}
