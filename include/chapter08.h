#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <map>
#include <d3dx12.h>
#include <DirectXTex.h>
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
	MaterialForHlsl materialForHlsl;
	unsigned char toonIdx;
	unsigned char edgeFlg;
	unsigned int indicesNum;
	std::string texFilePath;
};


// �e�N�X�`�����\�[�X�i�o�b�t�@�j
struct TextureResources {
	std::vector<ID3D12Resource*> normalTex;
	std::vector<ID3D12Resource*> sph;
	std::vector<ID3D12Resource*> spa;
	std::vector<ID3D12Resource*> toon;
};


// PMD���f���\����
struct PMD_MODEL_08 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
	std::vector<PMDMaterial> materials;
};


// �t�@�C�����֘A�֐�
std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath);
std::string GetExtension(const std::string& path);
std::wstring GetExtension(const std::wstring& path);
std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter = '*');
std::wstring GetWideStringFromString(const std::string& str);

// PMD�t�@�C���̃}�e���A������ǂݍ���
std::vector<PMDMaterial> readPmdMaterials(FILE* fp);

// PMD�t�@�C����ǂݍ��ށi�}�e���A���ǉ��Łj
PMD_MODEL_08 readPmdFile08(std::string pmdFileName);

// PMDMaterial����Material�փ}�e���A�������R�s�[
std::vector<Material> transformMaterials(std::vector<PMDMaterial> pmdMaterials);

// �e�N�X�`���t�@�C����ǂݍ��݁A�o�b�t�@���쐬�i�����g���q�Ή��Łj
ID3D12Resource* loadTextureAndCreateBuffer08(ID3D12Device* dev, std::string textureFilename);

// �e�N�X�`���t�@�C����ǂݍ���
ID3D12Resource* loadTexture(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, std::string& texPath);

// �g�D�[���e�N�X�`���t�@�C����ǂݍ���
ID3D12Resource* loadToonTexture(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, unsigned char toonIdx);

// �e�N�X�`�����\�[�X�i�o�b�t�@�j���쐬
TextureResources createTextureResources(ID3D12Device* dev, std::vector<Material> materials, std::string modelPath);

// �g�D�[���ȊO�̃e�N�X�`���t�@�C����ǂݍ���
void loadTextureExceptToon(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, TextureResources& textureResources, int material_idx, std::string texFileName, std::string modelPath);

// �}�e���A���o�b�t�@���쐬
ID3D12Resource* createMaterialBuffer(ID3D12Device* dev, UINT64 datasize);

// �}�e���A���o�b�t�@���}�b�v
void mapMaterialBuffer(ID3D12Resource* materialBuffer, std::vector<Material> materials);

// �}�e���A���o�b�t�@�r���[���쐬
void createMaterialBufferView(ID3D12Device* dev, ID3D12Resource* materialBuffer, ID3D12DescriptorHeap* descriptorHeap, TextureResources textureResources, UINT64 materialNum);

// ���[�g�V�O�l�`�����쐬�i�}�e���A���ǉ��Łj
ID3D12RootSignature* createRootSignature08(ID3D12Device* dev);

// �����_�����O�i�}�e���A���ǉ��Łj
void render08(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView,
	IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap,
	ID3D12DescriptorHeap* materialDescHeap, ID3D12DescriptorHeap* depthDescriptorHeap, std::vector<Material> materials);