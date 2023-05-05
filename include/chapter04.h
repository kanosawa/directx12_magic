#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;


// 頂点バッファーを作成
ID3D12Resource* createVertexBuffer(ID3D12Device* dev, UINT64 datasize);

// 頂点座標情報をマップ
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices);

// 頂点バッファビューを作成
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices);

// インデックスバッファーを作成
ID3D12Resource* createIndexBuffer(ID3D12Device* dev, UINT64 datasize);

// インデックス情報をマップ
void mapIndexBuffer(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices);

// インデックスバッファビューを作成
D3D12_INDEX_BUFFER_VIEW createIndexBufferView(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices);

// 頂点シェーダーオブジェクトを作成
ID3DBlob* createVertexShaderBlob();

// ピクセルシェーダーオブジェクトを作成
ID3DBlob* createPixelShaderBlob();

// インプットレイアウトを作成
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout04();

// グラフィックスパイプラインステートを作成
ID3D12PipelineState* createGraphicsPipelineState(ID3D12Device* dev, ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ID3D12RootSignature* rootSignature, std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout);

// ビューポートを作成
D3D12_VIEWPORT createViewPort(int windowWidth, int windowHeight);

// シザー矩形を作成
D3D12_RECT createScissorRect(int windowWidth, int windowHeight);

// ルートシグネチャを作成（頂点情報以外のデータをシェーダーに送るための仕組み）
// Chapter04では頂点情報しか使わないので、空のルートシグネチャを作成する
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);

// レンダリング処理（のコマンドリストへの登録）
void render04(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect);
