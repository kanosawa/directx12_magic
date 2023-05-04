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
	//2バイトのパディング
	unsigned int indicesNum;
	char texFilePath[20];
};
#pragma pack()

struct PMD_MODEL_08 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
	std::vector<PMDMaterial> materials;
};

//シェーダ側に投げられるマテリアルデータ
struct MaterialForHlsl {
	XMFLOAT3 diffuse; //ディフューズ色
	float alpha; // ディフューズα
	XMFLOAT3 specular; //スペキュラ色
	float specularity;//スペキュラの強さ(乗算値)
	XMFLOAT3 ambient; //アンビエント色
};
//それ以外のマテリアルデータ
struct AdditionalMaterial {
	std::string texPath;//テクスチャファイルパス
	int toonIdx; //トゥーン番号
	bool edgeFlg;//マテリアル毎の輪郭線フラグ
};
//まとめたもの
struct Material {
	unsigned int indicesNum;//インデックス数
	MaterialForHlsl material;
	AdditionalMaterial additional;
};

// PMDファイルのマテリアル情報を読み込む
std::vector<PMDMaterial> readPmdMaterials(FILE* fp);

// PMDファイルを読み込む（マテリアル追加版）
PMD_MODEL_08 readPmdFile08(std::string pmdFileName);

// ルートシグネチャを作成（マテリアル追加版）
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);