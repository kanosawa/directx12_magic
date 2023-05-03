#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <d3dx12.h>


// 頂点バッファをマップ（PMDファイル版）
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices);

// 頂点バッファビューを作成（PMDファイル版）
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices, size_t pmdvertex_size);

// インプットレイアウトを作成（PMDファイル版）
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();