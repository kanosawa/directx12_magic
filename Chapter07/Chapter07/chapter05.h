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
// ※chapter07で再定義
// std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();

// テクスチャファイルを読み込み、バッファを作成
ID3D12Resource* loadTextureAndCreateBuffer(ID3D12Device* dev, const wchar_t* textureFilename);

// テクスチャディスクリプタヒープを作成（レンダーターゲットビューと同様に、シェーダーリソースビューの作成に必要）
ID3D12DescriptorHeap* createTexDescriptorHeap(ID3D12Device* dev);

// シェーダーリソースビューを作成
void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* texDescHeap);

// バリアを作成（排他制御のための仕組み）
D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer);

// レンダリング処理（のコマンドリストへの登録）
// basicDescHeapはChapter05の時点ではテクスチャ専用だが、Chapter06移行で定数と兼用になる
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
	D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState,
	D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap);
