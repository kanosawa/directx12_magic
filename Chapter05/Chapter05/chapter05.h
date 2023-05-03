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
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout();

// �e�N�X�`���q�[�v�v���p�e�B���쐬
D3D12_HEAP_PROPERTIES createTexHeapProperties();

// �e�N�X�`�����\�[�X�f�B�X�N���v�^���쐬
D3D12_RESOURCE_DESC createTexResourceDescriptor(TexMetadata metadata);

// �e�N�X�`���o�b�t�@���쐬
ID3D12Resource* createTexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES texHeapProperties, const DirectX::Image* img, TexMetadata metadata);

// �e�N�X�`���f�B�X�N���v�^�q�[�v���쐬�i�����_�[�^�[�Q�b�g�r���[�Ɠ��l�ɁA�V�F�[�_�[���\�[�X�r���[�̍쐬�ɕK�v�j
ID3D12DescriptorHeap* createTexDescriptorHeap(ID3D12Device* dev);

// �V�F�[�_�[���\�[�X�r���[���쐬
void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* texDescHeap, DXGI_FORMAT format);
