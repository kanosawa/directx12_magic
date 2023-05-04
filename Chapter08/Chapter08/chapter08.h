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


// PMDファイルフォーマットのマテリアル構造体
#pragma pack(1)
struct PMDMaterial {
	XMFLOAT3 diffuse;
	float alpha;
	float specularity;
	XMFLOAT3 specular;
	XMFLOAT3 ambient;
	unsigned char toonIdx;
	unsigned char edgeFlg;
	//2バイトのパディング
	unsigned int indicesNum;
	char texFilePath[20];
};
#pragma pack()


// HLSL用マテリアル構造体
struct MaterialForHlsl {
	// ディフューズ
	XMFLOAT3 diffuse;
	float alpha;
	// スペキュラー
	XMFLOAT3 specular;
	float specularity;
	// アンビエント
	XMFLOAT3 ambient;
};


// レンダリング用マテリアル構造体
struct Material {
	unsigned int indicesNum;
	MaterialForHlsl materialForHlsl;
};


// PMDモデル構造体
struct PMD_MODEL_08 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
	std::vector<PMDMaterial> materials;
};


// PMDファイルのマテリアル情報を読み込む
std::vector<PMDMaterial> readPmdMaterials(FILE* fp);

// PMDファイルを読み込む（マテリアル追加版）
PMD_MODEL_08 readPmdFile08(std::string pmdFileName);

// ルートシグネチャを作成（マテリアル追加版）
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);