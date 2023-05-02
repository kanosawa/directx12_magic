#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;


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


ID3D12DescriptorHeap* createDescriptorHeap(ID3D12Device* dev) {

	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.NodeMask = 0;
	descriptorHeapDesc.NumDescriptors = 2;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	auto result = dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));

	return descriptorHeap;
}


std::vector<ID3D12Resource*> getBufferAndLinkRenderTargetViews(ID3D12Device* dev, IDXGISwapChain4* swapChain, ID3D12DescriptorHeap* descriptorHeap) {

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	auto result = swapChain->GetDesc(&swapChainDesc);

	std::vector<ID3D12Resource*> backBuffers(swapChainDesc.BufferCount);
	auto handle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (size_t i = 0; i < swapChainDesc.BufferCount; ++i) {
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		dev->CreateRenderTargetView(backBuffers[i], nullptr, handle);
		handle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	}

	return backBuffers;
}


D3D12_RESOURCE_BARRIER createResourceBarrier(ID3D12Resource* backBuffer) {
	D3D12_RESOURCE_BARRIER resourceBarrier = {};
	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = backBuffer;
	resourceBarrier.Transition.Subresource = 0;
	return resourceBarrier;
}


D3D12_HEAP_PROPERTIES createHeapProperties() {
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	return heapProperties;
}


D3D12_RESOURCE_DESC createResourceDescriptor(int verticesSize) {
	D3D12_RESOURCE_DESC resourceDescriptor = {};
	resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescriptor.Width = verticesSize;
	resourceDescriptor.Height = 1;
	resourceDescriptor.DepthOrArraySize = 1;
	resourceDescriptor.MipLevels = 1;
	resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescriptor.SampleDesc.Count = 1;
	resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
	resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	return resourceDescriptor;
}

ID3D12Resource* createVertexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES heapProperties, D3D12_RESOURCE_DESC resourceDescriptor) {
	ID3D12Resource* vertexBuffer = nullptr;
	auto result = dev->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);
	return vertexBuffer;
}

void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	XMFLOAT3* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}

D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}

ID3D12Resource* createIndexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES heapProperties, D3D12_RESOURCE_DESC resourceDescriptor, int indexSize) {
	ID3D12Resource* indexBuffer = nullptr;
	//設定は、バッファのサイズ以外頂点バッファの設定を使いまわして
	//OKだと思います。
	resourceDescriptor.Width = indexSize;
	auto result = dev->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDescriptor,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer));
	return indexBuffer;
}

void mapIndexBuffer(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	unsigned short* indexBufferMap = nullptr;
	auto result = indexBuffer->Map(0, nullptr, (void**)&indexBufferMap);
	std::copy(std::begin(indices), std::end(indices), indexBufferMap);
	indexBuffer->Unmap(0, nullptr);
}

D3D12_INDEX_BUFFER_VIEW createIndexBufferView(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);
	return indexBufferView;
}

ID3DBlob* createVertexShaderBlob() {
	ID3DBlob* vertexShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3DCompileFromFile(
		L"BasicVertexShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicVS", "vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShaderBlob, &errorBlob
	);
	return vertexShaderBlob;
}

ID3DBlob* createPixelShaderBlob() {
	ID3DBlob* pixelShaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3DCompileFromFile(
		L"BasicPixelShader.hlsl",
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"BasicPS", "ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShaderBlob, &errorBlob
	);
	return pixelShaderBlob;
}

ID3D12RootSignature* createRootSignature(ID3D12Device* dev) {

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
	rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDescriptor,
		D3D_ROOT_SIGNATURE_VERSION_1_0,
		&rootSignatureBlob,
		&errorBlob
	);

	ID3D12RootSignature* rootSignature = nullptr;
	result = dev->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature)
	);
	rootSignatureBlob->Release();

	return rootSignature;
}

ID3D12PipelineState* createGraphicsPipelineState(ID3D12Device* dev, ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ID3D12RootSignature* rootSignature) {

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gPipelineStateDescriptor = {};
	gPipelineStateDescriptor.pRootSignature = nullptr;
	gPipelineStateDescriptor.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
	gPipelineStateDescriptor.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
	gPipelineStateDescriptor.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
	gPipelineStateDescriptor.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();
	gPipelineStateDescriptor.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	gPipelineStateDescriptor.RasterizerState.MultisampleEnable = false;
	gPipelineStateDescriptor.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gPipelineStateDescriptor.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gPipelineStateDescriptor.RasterizerState.DepthClipEnable = true;
	gPipelineStateDescriptor.BlendState.AlphaToCoverageEnable = false;
	gPipelineStateDescriptor.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDescriptor = {};
	renderTargetBlendDescriptor.BlendEnable = false;
	renderTargetBlendDescriptor.LogicOpEnable = false;
	renderTargetBlendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	gPipelineStateDescriptor.BlendState.RenderTarget[0] = renderTargetBlendDescriptor;

	D3D12_INPUT_ELEMENT_DESC inputElementDescriptor[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	gPipelineStateDescriptor.InputLayout.pInputElementDescs = inputElementDescriptor;
	gPipelineStateDescriptor.InputLayout.NumElements = _countof(inputElementDescriptor);

	gPipelineStateDescriptor.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	gPipelineStateDescriptor.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gPipelineStateDescriptor.NumRenderTargets = 1;
	gPipelineStateDescriptor.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gPipelineStateDescriptor.SampleDesc.Count = 1;
	gPipelineStateDescriptor.SampleDesc.Quality = 0;

	gPipelineStateDescriptor.pRootSignature = rootSignature;

	ID3D12PipelineState* pipelineState = nullptr;
	auto result = dev->CreateGraphicsPipelineState(&gPipelineStateDescriptor, IID_PPV_ARGS(&pipelineState));

	return pipelineState;
}

D3D12_VIEWPORT createViewPort(int windowWidth, int windowHeight) {
	D3D12_VIEWPORT viewport = {};
	viewport.Width = windowWidth;
	viewport.Height = windowHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1.0f;
	viewport.MinDepth = 0.0f;
	return viewport;
}

D3D12_RECT createScissorRect(int windowWidth, int windowHeight) {
	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = scissorRect.left + windowWidth;
	scissorRect.bottom = scissorRect.top + windowHeight;
	return scissorRect;
}



int main() {

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	std::vector<XMFLOAT3> vertices = {
		{-0.4f,-0.7f,0.0f} ,//左下
		{-0.4f,0.7f,0.0f} ,//左上
		{0.4f,-0.7f,0.0f} ,//右下
		{0.4f,0.7f,0.0f} ,//右上
	};

	std::vector<unsigned short> indices = { 0,1,2, 2,1,3 };

	auto windowClass = createWindowClass();
	auto hwnd = createWindowHandle(windowClass, windowWidth, windowHeight);
	auto dev = createDevice();
	auto dxgiFactory = createFactory();
	auto commandAllocator = createCommandAllocator(dev);
	auto commandList = createCommandList(dev, commandAllocator);
	auto commandQueue = createCommandQueue(dev);
	auto swapChain = createSwapChain(hwnd, dxgiFactory, commandQueue, windowWidth, windowHeight);
	auto descriptorHeap = createDescriptorHeap(dev);
	auto backBuffers = getBufferAndLinkRenderTargetViews(dev, swapChain, descriptorHeap);

	auto heapProperties = createHeapProperties();
	auto resourceDescriptor = createResourceDescriptor(sizeof(vertices[0]) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, heapProperties, resourceDescriptor);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);
	auto indexBuffer = createIndexBuffer(dev, heapProperties, resourceDescriptor, sizeof(indices[0]) * indices.size());
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();
	auto rootSignature = createRootSignature(dev);
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);


	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;
	auto result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};

	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) break;

		auto bufferIdx = swapChain->GetCurrentBackBufferIndex();
		auto resourceBarrier = createResourceBarrier(backBuffers[bufferIdx]);
		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		commandList->ResourceBarrier(1, &resourceBarrier);

		commandList->SetPipelineState(pipelineState);

		auto rtvHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvHandle.ptr += bufferIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

		float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
		commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		commandList->SetGraphicsRootSignature(rootSignature);

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

		commandList->IASetIndexBuffer(&indexBufferView);
		commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

		resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		commandList->ResourceBarrier(1, &resourceBarrier);

		commandList->Close();

		ID3D12CommandList* constCommandList[] = { commandList };
		commandQueue->ExecuteCommandLists(1, constCommandList);

		commandQueue->Signal(fence, ++fenceVal);
		if (fence->GetCompletedValue() != fenceVal)
		{
			auto event = CreateEvent(nullptr, false, false, nullptr);
			fence->SetEventOnCompletion(fenceVal, event);
			WaitForSingleObject(event, INFINITE);
			CloseHandle(event);
		}

		commandAllocator->Reset();
		commandList->Reset(commandAllocator, nullptr);

		swapChain->Present(1, 0);
	}

	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

}