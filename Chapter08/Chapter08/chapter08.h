#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <d3dx12.h>
#include <DirectXTex.h>
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
	MaterialForHlsl materialForHlsl;
	unsigned char toonIdx;
	unsigned char edgeFlg;
	unsigned int indicesNum;
	std::string texFilePath;
};


// テクスチャリソース（バッファ）
struct TextureResources {
	std::vector<ID3D12Resource*> normalTex;
	std::vector<ID3D12Resource*> sph;
	std::vector<ID3D12Resource*> spa;
	std::vector<ID3D12Resource*> toon;
};


// PMDモデル構造体
struct PMD_MODEL_08 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
	std::vector<PMDMaterial> materials;
};


// ファイル名関連関数
std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath);
std::string GetExtension(const std::string& path);
std::wstring GetExtension(const std::wstring& path);
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*');
std::wstring GetWideStringFromString(const std::string& str);

// PMDファイルのマテリアル情報を読み込む
std::vector<PMDMaterial> readPmdMaterials(FILE* fp);

// PMDファイルを読み込む（マテリアル追加版）
PMD_MODEL_08 readPmdFile08(std::string pmdFileName);

// PMDMaterialからMaterialへマテリアル情報をコピー
std::vector<Material> transformMaterials(std::vector<PMDMaterial> pmdMaterials);

// テクスチャファイルを読み込み、バッファを作成（複数拡張子対応版）
ID3D12Resource* loadTextureAndCreateBuffer(ID3D12Device* dev, std::string textureFilename);

// テクスチャファイルを読み込む
ID3D12Resource* loadTexture(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, std::string& texPath);

// トゥーンテクスチャファイルを読み込む
ID3D12Resource* loadToonTexture(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, unsigned char toonIdx);

// テクスチャリソース（バッファ）を作成
TextureResources createTextureResources(ID3D12Device* dev, std::vector<Material> materials, std::string modelPath);

// トゥーン以外のテクスチャファイルを読み込む
void loadTextureExceptToon(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, TextureResources& textureResources, int material_idx, std::string texFileName, std::string modelPath);

// ルートシグネチャを作成（マテリアル追加版）
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);
