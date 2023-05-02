#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>


#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")


LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}


WNDCLASSEX createWindowClass() {
	WNDCLASSEX windowClass = {};
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.lpfnWndProc = (WNDPROC)WindowProcedure;
	windowClass.lpszClassName = _T("DX12Sample");
	windowClass.hInstance = GetModuleHandle(nullptr);
	RegisterClassEx(&windowClass);
	return windowClass;
}


HWND createWindowHandle(WNDCLASSEX windowClass, LONG windowWidth, LONG windowHeight) {

	RECT wrc = { 0, 0, windowWidth, windowHeight };
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	auto hwnd = CreateWindow(
		windowClass.lpszClassName,
		_T("DX12Sample"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		windowClass.hInstance,
		nullptr
	);

	return hwnd;
}


// Direct3Dデバイスを初期化（色々な処理に必要）
ID3D12Device* createDevice() {
	ID3D12Device* dev = nullptr;
	auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dev));
	return dev;
}


// DXGIファクトリを初期化（スワップチェーンの作成に必要）
IDXGIFactory6* createFactory() {
	IDXGIFactory6* dxgiFactory = nullptr;
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory));
	return dxgiFactory;
}


// コマンドアロケーターを作成（コマンドオブジェクト本体）
ID3D12CommandAllocator* createCommandAllocator(ID3D12Device* dev) {
	ID3D12CommandAllocator* commandAllocator = nullptr;
	auto result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	return commandAllocator;
}


// コマンドリストを作成（コマンドのインターフェース。コマンド本体はコマンドアロケーターに蓄積される）
ID3D12GraphicsCommandList* createCommandList(ID3D12Device* dev, ID3D12CommandAllocator* commandAllocator) {
	ID3D12GraphicsCommandList* commandList = nullptr;
	auto result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	return commandList;
}


// コマンドキューを作成（コマンド実行のためのインターフェース）
ID3D12CommandQueue* createCommandQueue(ID3D12Device* dev) {

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {};
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ID3D12CommandQueue* commandQueue = nullptr;
	auto result = dev->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));

	return commandQueue;
}


// スワップチェーンを作成（ダブルバッファリングのための仕組み）
IDXGISwapChain4* createSwapChain(HWND hwnd, IDXGIFactory6* dxgiFactory, ID3D12CommandQueue* commandQueue, LONG windowWidth, LONG windowHeight) {

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = windowWidth;
	swapChainDesc.Height = windowHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	IDXGISwapChain4* swapChain = nullptr;
	auto result = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue,
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&swapChain
	);

	return swapChain;
}


// レンダーターゲットビュー用のディスクリプタヒープを作成
ID3D12DescriptorHeap* createRenderTargetViewDescriptorHeap(ID3D12Device* dev) {

	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDescriptorHeapDesc.NodeMask = 0;
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	rtvDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	auto result = dev->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

	return descriptorHeap;
}


// レンダーターゲットビューを作成し、バックバッファを返す
std::vector<ID3D12Resource*> createRenderTargetViewAndGetBuckBuffers(ID3D12Device* dev, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* rtvDescriptorHeap) {

	// バッファ数を取得（ダブルバッファなら2）
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	auto result = swapChain->GetDesc(&swapChainDesc);
	auto bufferCount = swapChainDesc.BufferCount;

	std::vector<ID3D12Resource*> backBuffers(bufferCount);
	auto rtvDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < bufferCount; ++i) {
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		dev->CreateRenderTargetView(backBuffers[i], nullptr, rtvDescriptorHandle);
		rtvDescriptorHandle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return backBuffers;
}


// フェンスを作成（コマンド実行完了確認のための仕組み）
ID3D12Fence* createFence(ID3D12Device* dev) {
	ID3D12Fence* fence = nullptr;
	auto result = dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	return fence;
}


// バリアを作成（排他制御のための仕組み）
D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer) {
	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = backBuffer;
	resourceBarrier.Transition.Subresource = 0;
	return resourceBarrier;
}


// レンダリング処理（のコマンドリストへの登録）
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, IDXGISwapChain4* swapChain) {

	// バリアを設定
	ID3D12Resource* backBuffer;
	auto bufferIdx = swapChain->GetCurrentBackBufferIndex();
	auto result = swapChain->GetBuffer(bufferIdx, IID_PPV_ARGS(&backBuffer));
	auto resourceBarrier = createResourceBarrier(backBuffer);

	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &resourceBarrier);

	// これから使うレンダーターゲットビューとしてrtvHandleをセットする
	auto rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bufferIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	// レンダリング
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// バリアによる完了待ち
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}


int main() {
	
	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	auto windowClass = createWindowClass();
	auto hwnd = createWindowHandle(windowClass, windowWidth, windowHeight);
	auto dev = createDevice();
	auto dxgiFactory = createFactory();
	auto commandAllocator = createCommandAllocator(dev);
	auto commandList = createCommandList(dev, commandAllocator);
	auto commandQueue = createCommandQueue(dev);
	auto swapChain = createSwapChain(hwnd, dxgiFactory, commandQueue, windowWidth, windowHeight);
	auto rtvDescriptorHeap = createRenderTargetViewDescriptorHeap(dev);
	auto backBuffers = createRenderTargetViewAndGetBuckBuffers(dev, swapChain, rtvDescriptorHeap);
	auto fence = createFence(dev);
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	UINT64 fenceVal = 0;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) break;

		render(dev, rtvDescriptorHeap, commandList, swapChain);
		commandList->Close();

		ID3D12CommandList* constCommandList[] = { commandList };
		commandQueue->ExecuteCommandLists(1, constCommandList);

		// レンダリング完了待ち
		commandQueue->Signal(fence, ++fenceVal);
		auto event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);

		commandAllocator->Reset();
		commandList->Reset(commandAllocator, nullptr);

		swapChain->Present(1, 0);
	}

	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	
}