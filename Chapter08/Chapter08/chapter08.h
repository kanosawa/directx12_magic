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


// PMD�t�@�C���t�H�[�}�b�g�̃}�e���A���\����
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


// HLSL�p�}�e���A���\����
struct MaterialForHlsl {
	// �f�B�t���[�Y
	XMFLOAT3 diffuse;
	float alpha;
	// �X�y�L�����[
	XMFLOAT3 specular;
	float specularity;
	// �A���r�G���g
	XMFLOAT3 ambient;
};


// �����_�����O�p�}�e���A���\����
struct Material {
	unsigned int indicesNum;
	MaterialForHlsl materialForHlsl;
};


// PMD���f���\����
struct PMD_MODEL_08 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
	std::vector<PMDMaterial> materials;
};


// PMD�t�@�C���̃}�e���A������ǂݍ���
std::vector<PMDMaterial> readPmdMaterials(FILE* fp);

// PMD�t�@�C����ǂݍ��ށi�}�e���A���ǉ��Łj
PMD_MODEL_08 readPmdFile08(std::string pmdFileName);

// ���[�g�V�O�l�`�����쐬�i�}�e���A���ǉ��Łj
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);