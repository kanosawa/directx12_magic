#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>


// �E�B���h�E�֘A
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDCLASSEX createWindowClass();
HWND createWindowHandle(WNDCLASSEX windowClass, LONG windowWidth, LONG windowHeight);

// Direct3D�f�o�C�X���������i�F�X�ȏ����ɕK�v�j
ID3D12Device* createDevice();

// DXGI�t�@�N�g�����������i�X���b�v�`�F�[���̍쐬�ɕK�v�j
IDXGIFactory6* createFactory();

// �R�}���h�A���P�[�^�[���쐬�i�R�}���h�I�u�W�F�N�g�{�́j
ID3D12CommandAllocator* createCommandAllocator(ID3D12Device* dev);

// �R�}���h���X�g���쐬�i�R�}���h�̃C���^�[�t�F�[�X�B�R�}���h�{�̂̓R�}���h�A���P�[�^�[�ɒ~�ς����j
ID3D12GraphicsCommandList* createCommandList(ID3D12Device* dev, ID3D12CommandAllocator* commandAllocator);

// �R�}���h�L���[���쐬�i�R�}���h���s�̂��߂̃C���^�[�t�F�[�X�j
ID3D12CommandQueue* createCommandQueue(ID3D12Device* dev);

// �X���b�v�`�F�[�����쐬�i�_�u���o�b�t�@�����O�̂��߂̎d�g�݁j
IDXGISwapChain4* createSwapChain(HWND hwnd, IDXGIFactory6* dxgiFactory, ID3D12CommandQueue* commandQueue, LONG windowWidth, LONG windowHeight);

// �����_�[�^�[�Q�b�g�r���[�p�̃f�B�X�N���v�^�q�[�v���쐬
ID3D12DescriptorHeap* createRenderTargetViewDescriptorHeap(ID3D12Device* dev);

// �����_�[�^�[�Q�b�g�r���[���쐬���A�o�b�N�o�b�t�@��Ԃ�
std::vector<ID3D12Resource*> createRenderTargetViewAndGetBuckBuffers(ID3D12Device* dev, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap);

// �t�F���X���쐬�i�R�}���h���s�����m�F�̂��߂̎d�g�݁j
ID3D12Fence* createFence(ID3D12Device* dev);

// �o���A���쐬�i�r������̂��߂̎d�g�݁j
D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer);