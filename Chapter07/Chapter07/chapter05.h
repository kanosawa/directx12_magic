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


// ���_�o�b�t�@���}�b�v�iVertex�Łj
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices);

// ���_�o�b�t�@�r���[���쐬�iVertex�Łj
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices);

// �C���v�b�g���C�A�E�g���쐬�iTEXCOORD�ǉ��Łj
// ��chapter07�ōĒ�`
// std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();

// �e�N�X�`���t�@�C����ǂݍ��݁A�o�b�t�@���쐬
ID3D12Resource* loadTextureAndCreateBuffer(ID3D12Device* dev, const wchar_t* textureFilename);

// �e�N�X�`���f�B�X�N���v�^�q�[�v���쐬�i�����_�[�^�[�Q�b�g�r���[�Ɠ��l�ɁA�V�F�[�_�[���\�[�X�r���[�̍쐬�ɕK�v�j
ID3D12DescriptorHeap* createTexDescriptorHeap(ID3D12Device* dev);

// �V�F�[�_�[���\�[�X�r���[���쐬
void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* texDescHeap);

// �o���A���쐬�i�r������̂��߂̎d�g�݁j
D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer);

// �����_�����O�����i�̃R�}���h���X�g�ւ̓o�^�j
// basicDescHeap��Chapter05�̎��_�ł̓e�N�X�`����p�����AChapter06�ڍs�Œ萔�ƌ��p�ɂȂ�
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
	D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState,
	D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap);
