#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <d3dx12.h>


// ルートシグネチャを作成（マテリアル追加版）
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);