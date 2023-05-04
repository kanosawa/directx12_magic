#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <d3dx12.h>

using namespace DirectX;

struct MatricesData {
	XMMATRIX world;
	XMMATRIX viewproj;
};

#pragma pack(push, 1)
struct PMD_VERTEX
{
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	uint16_t bone_no[2];
	uint8_t  weight;
	uint8_t  EdgeFlag;
	uint16_t dummy;
};
#pragma pack(pop)

struct PMDHeader {
	float version;
	char model_name[20];
	char comment[256];
};

struct PMD_MODEL_07 {
	std::vector<PMD_VERTEX> vertices;
	std::vector<unsigned short> indices;
};

// PMDファイルを読み込む
PMD_MODEL_07 readPmdFile(std::string pmdFileName);

// 頂点バッファをマップ（PMDファイル版）
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<PMD_VERTEX> vertices);

// 頂点バッファビューを作成（PMDファイル版）
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<PMD_VERTEX> vertices);

// インプットレイアウトを作成（PMDファイル版）
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();

// 定数バッファを作成（PMDファイル版）
ID3D12Resource* createConstBuffer(ID3D12Device* dev);

// デプスバッファを作成
ID3D12Resource* createDepthBuffer(ID3D12Device* dev, int windowWidth, int windowHeight);

// デプスディスクリプタヒープを作成
ID3D12DescriptorHeap* createDepthDescriptorHeap(ID3D12Device* dev, ID3D12Resource* depthBuffer);

// デプスバッファビューを作成
void createDepthBufferView(ID3D12Device* dev, ID3D12Resource* depthBuffer, ID3D12DescriptorHeap* dsvHeap);

// レンダリング処理（のコマンドリストへの登録）
// デプス付き
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
	D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState,
	D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap, ID3D12DescriptorHeap* depthDescriptorHeap, unsigned int indicesNum);