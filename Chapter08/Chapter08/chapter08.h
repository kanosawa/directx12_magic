#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <d3dx12.h>
#include "chapter07.h"

using namespace DirectX;

#pragma pack(1)
struct PMDMaterial {
	XMFLOAT3 diffuse;
	float alpha;
	float specularity;
	XMFLOAT3 specular;
	XMFLOAT3 ambient;
	unsigned char toonIdx;
	unsigned char edgeFlg;
	//2�o�C�g�̃p�f�B���O
	unsigned int indicesNum;
	char texFilePath[20];
};
#pragma pack()

struct PMD_MODEL_08 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
	std::vector<PMDMaterial> materials;
};

//�V�F�[�_���ɓ�������}�e���A���f�[�^
struct MaterialForHlsl {
	XMFLOAT3 diffuse; //�f�B�t���[�Y�F
	float alpha; // �f�B�t���[�Y��
	XMFLOAT3 specular; //�X�y�L�����F
	float specularity;//�X�y�L�����̋���(��Z�l)
	XMFLOAT3 ambient; //�A���r�G���g�F
};
//����ȊO�̃}�e���A���f�[�^
struct AdditionalMaterial {
	std::string texPath;//�e�N�X�`���t�@�C���p�X
	int toonIdx; //�g�D�[���ԍ�
	bool edgeFlg;//�}�e���A�����̗֊s���t���O
};
//�܂Ƃ߂�����
struct Material {
	unsigned int indicesNum;//�C���f�b�N�X��
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

// PMD�t�@�C���̃}�e���A������ǂݍ���
std::vector<PMDMaterial> readPmdMaterials(FILE* fp);

// PMD�t�@�C����ǂݍ��ށi�}�e���A���ǉ��Łj
PMD_MODEL_08 readPmdFile08(std::string pmdFileName);

// ���[�g�V�O�l�`�����쐬�i�}�e���A���ǉ��Łj
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);