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


// 定数バッファを作成
ID3D12Resource* createConstBuffer06(ID3D12Device* dev);

// 定数バッファビューを作成
// idx : descriptorHeapの何番目か
void createConstantBufferView(ID3D12Device* dev, ID3D12Resource* constBuffer, ID3D12DescriptorHeap* basicDescriptorHeap, UINT64 idx);

// ルートシグネチャを作成
ID3D12RootSignature* createRootSignature06(ID3D12Device* dev);