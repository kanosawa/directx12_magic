#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>


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


ID3D12Device* createDevice() {
	ID3D12Device* dev = nullptr;
	auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dev));
	return dev;
}


IDXGIFactory6* createFactory() {
	IDXGIFactory6* dxgiFactory = nullptr;
	auto result = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&dxgiFactory));
	return dxgiFactory;
}


ID3D12CommandAllocator* createCommandAllocator(ID3D12Device* dev) {
	ID3D12CommandAllocator* commandAllocator = nullptr;
	auto result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	return commandAllocator;
}


ID3D12GraphicsCommandList* createCommandList(ID3D12Device* dev, ID3D12CommandAllocator* commandAllocator) {
	ID3D12GraphicsCommandList* commandList = nullptr;
	auto result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	return commandList;
}


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


ID3D12Fence* createFence(ID3D12Device* dev) {
	ID3D12Fence* fence = nullptr;
	auto result = dev->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	return fence;
}


D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer) {
	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = backBuffer;
	resourceBarrier.Transition.Subresource = 0;
	return resourceBarrier;
}