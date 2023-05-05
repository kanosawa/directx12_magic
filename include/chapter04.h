#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;


// ���_�o�b�t�@�[���쐬
ID3D12Resource* createVertexBuffer(ID3D12Device* dev, UINT64 datasize);

// ���_���W�����}�b�v
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices);

// ���_�o�b�t�@�r���[���쐬
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices);

// �C���f�b�N�X�o�b�t�@�[���쐬
ID3D12Resource* createIndexBuffer(ID3D12Device* dev, UINT64 datasize);

// �C���f�b�N�X�����}�b�v
void mapIndexBuffer(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices);

// �C���f�b�N�X�o�b�t�@�r���[���쐬
D3D12_INDEX_BUFFER_VIEW createIndexBufferView(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices);

// ���_�V�F�[�_�[�I�u�W�F�N�g���쐬
ID3DBlob* createVertexShaderBlob();

// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g���쐬
ID3DBlob* createPixelShaderBlob();

// �C���v�b�g���C�A�E�g���쐬
std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout04();

// �O���t�B�b�N�X�p�C�v���C���X�e�[�g���쐬
ID3D12PipelineState* createGraphicsPipelineState(ID3D12Device* dev, ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ID3D12RootSignature* rootSignature, std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout);

// �r���[�|�[�g���쐬
D3D12_VIEWPORT createViewPort(int windowWidth, int windowHeight);

// �V�U�[��`���쐬
D3D12_RECT createScissorRect(int windowWidth, int windowHeight);

// ���[�g�V�O�l�`�����쐬�i���_���ȊO�̃f�[�^���V�F�[�_�[�ɑ��邽�߂̎d�g�݁j
// Chapter04�ł͒��_��񂵂��g��Ȃ��̂ŁA��̃��[�g�V�O�l�`�����쐬����
ID3D12RootSignature* createRootSignature(ID3D12Device* dev);

// �����_�����O�����i�̃R�}���h���X�g�ւ̓o�^�j
void render04(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect);
