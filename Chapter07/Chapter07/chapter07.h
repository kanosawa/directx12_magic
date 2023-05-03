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


// 頂点バッファをマップ（PMDファイル版）
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices);

// 頂点バッファビューを作成（PMDファイル版）
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices, size_t pmdvertex_size);

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