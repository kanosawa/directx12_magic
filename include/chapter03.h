#pragma once

#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>


// ウィンドウ関連
LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
WNDCLASSEX createWindowClass();
HWND createWindowHandle(WNDCLASSEX windowClass, LONG windowWidth, LONG windowHeight);

// Direct3Dデバイスを初期化（色々な処理に必要）
ID3D12Device* createDevice();

// DXGIファクトリを初期化（スワップチェーンの作成に必要）
IDXGIFactory6* createFactory();

// コマンドアロケーターを作成（コマンドオブジェクト本体）
ID3D12CommandAllocator* createCommandAllocator(ID3D12Device* dev);

// コマンドリストを作成（コマンドのインターフェース。コマンド本体はコマンドアロケーターに蓄積される）
ID3D12GraphicsCommandList* createCommandList(ID3D12Device* dev, ID3D12CommandAllocator* commandAllocator);

// コマンドキューを作成（コマンド実行のためのインターフェース）
ID3D12CommandQueue* createCommandQueue(ID3D12Device* dev);

// スワップチェーンを作成（ダブルバッファリングのための仕組み）
IDXGISwapChain4* createSwapChain(HWND hwnd, IDXGIFactory6* dxgiFactory, ID3D12CommandQueue* commandQueue, LONG windowWidth, LONG windowHeight);

// レンダーターゲットビュー用のディスクリプタヒープを作成
ID3D12DescriptorHeap* createRenderTargetViewDescriptorHeap(ID3D12Device* dev);

// レンダーターゲットビューを作成し、バックバッファを返す
std::vector<ID3D12Resource*> createRenderTargetViewAndGetBuckBuffers(ID3D12Device* dev, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap);

// フェンスを作成（コマンド実行完了確認のための仕組み）
ID3D12Fence* createFence(ID3D12Device* dev);

// バリアを作成（排他制御のための仕組み）
D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer);

// レンダリング処理（のコマンドリストへの登録）
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, IDXGISwapChain4* swapChain);
