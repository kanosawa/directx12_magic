#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <DirectXTex.h>

using namespace DirectX;


struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};


// 頂点バッファをマップ（Vertex版）
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices);

// 頂点バッファビューを作成（Vertex版）
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices);

// インプットレイアウトを作成（TEXCOORD追加版）
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();

// テクスチャヒーププロパティを作成
D3D12_HEAP_PROPERTIES createTexHeapProperties();

// テクスチャリソースディスクリプタを作成
D3D12_RESOURCE_DESC createTexResourceDescriptor(TexMetadata metadata);

// テクスチャバッファを作成
ID3D12Resource* createTexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES texHeapProperties, const DirectX::Image* img, TexMetadata metadata);

// テクスチャディスクリプタヒープを作成（レンダーターゲットビューと同様に、シェーダーリソースビューの作成に必要）
ID3D12DescriptorHeap* createTexDescriptorHeap(ID3D12Device* dev);

// シェーダーリソースビューを作成
void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* texDescHeap, DXGI_FORMAT format);
