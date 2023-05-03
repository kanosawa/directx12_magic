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


// ���_�o�b�t�@���}�b�v�iPMD�t�@�C���Łj
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices);

// ���_�o�b�t�@�r���[���쐬�iPMD�t�@�C���Łj
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices, size_t pmdvertex_size);

// �C���v�b�g���C�A�E�g���쐬�iPMD�t�@�C���Łj
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();

// �萔�o�b�t�@���쐬�iPMD�t�@�C���Łj
ID3D12Resource* createConstBuffer(ID3D12Device* dev);

// �f�v�X�o�b�t�@���쐬
ID3D12Resource* createDepthBuffer(ID3D12Device* dev, int windowWidth, int windowHeight);

// �f�v�X�f�B�X�N���v�^�q�[�v���쐬
ID3D12DescriptorHeap* createDepthDescriptorHeap(ID3D12Device* dev, ID3D12Resource* depthBuffer);

// �f�v�X�o�b�t�@�r���[���쐬
void createDepthBufferView(ID3D12Device* dev, ID3D12Resource* depthBuffer, ID3D12DescriptorHeap* dsvHeap);