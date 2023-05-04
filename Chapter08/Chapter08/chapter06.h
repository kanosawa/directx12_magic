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
#include <d3dx12.h>

using namespace DirectX;


// テクスチャと定数用のディスクリプタヒープを作成
ID3D12DescriptorHeap* createBasicDescriptorHeap(ID3D12Device* dev, UINT64 numDescriptors);

// 定数バッファを作成
// ※chapter07で再定義
// ID3D12Resource* createConstBuffer(ID3D12Device* dev);

// 定数バッファビューを作成
// idx : descriptorHeapの何番目か
void createConstantBufferView(ID3D12Device* dev, ID3D12Resource* constBuffer, ID3D12DescriptorHeap* basicDescriptorHeap, UINT64 idx);

// ルートシグネチャを作成
// ※chapter08で再定義
// ID3D12RootSignature* createRootSignature(ID3D12Device* dev);