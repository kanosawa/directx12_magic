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


// �e�N�X�`���ƒ萔�p�̃f�B�X�N���v�^�q�[�v���쐬
ID3D12DescriptorHeap* createBasicDescriptorHeap(ID3D12Device* dev, UINT64 numDescriptors);

// �萔�o�b�t�@���쐬
// ��chapter07�ōĒ�`
// ID3D12Resource* createConstBuffer(ID3D12Device* dev);

// �萔�o�b�t�@�r���[���쐬
// idx : descriptorHeap�̉��Ԗڂ�
void createConstantBufferView(ID3D12Device* dev, ID3D12Resource* constBuffer, ID3D12DescriptorHeap* basicDescriptorHeap, UINT64 idx);

// ���[�g�V�O�l�`�����쐬
// ��chapter08�ōĒ�`
// ID3D12RootSignature* createRootSignature(ID3D12Device* dev);