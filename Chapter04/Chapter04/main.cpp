#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include "chapter03.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

using namespace DirectX;


// ヒーププロパティを作成（頂点座標とインデックス用）
D3D12_HEAP_PROPERTIES createHeapProperties() {
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	return heapProperties;
}


// リソースディスクリプタを作成（頂点座標とインデックス用）
D3D12_RESOURCE_DESC createResourceDescriptor(UINT64 dataSize) {
	D3D12_RESOURCE_DESC resourceDescriptor = {};
	resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDescriptor.Width = dataSize;
	resourceDescriptor.Height = 1;
	resourceDescriptor.DepthOrArraySize = 1;
	resourceDescriptor.MipLevels = 1;
	resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
	resourceDescriptor.SampleDesc.Count = 1;
	resourceDescriptor.Flags = D3D12_RESOURCE_FLAG_NONE;
	resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	return resourceDescriptor;
}


// 頂点バッファーを作成
ID3D12Resource* createVertexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES vertexHeapProperties, D3D12_RESOURCE_DESC vertexResourceDescriptor) {
	ID3D12Resource* vertexBuffer = nullptr;
	auto result = dev->CreateCommittedResource(
		&vertexHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexResourceDescriptor,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);
	return vertexBuffer;
}


// 頂点座標情報をマップ
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	XMFLOAT3* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


// 頂点バッファビューを作成
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}


// インデックスバッファーを作成
ID3D12Resource* createIndexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES indexHeapProperties, D3D12_RESOURCE_DESC indexResourceDescriptor) {
	ID3D12Resource* indexBuffer = nullptr;
	auto result = dev->CreateCommittedResource(
		&indexHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&indexResourceDescriptor,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer));
	return indexBuffer;
}


// インデックス情報をマップ
void mapIndexBuffer(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	unsigned short* indexBufferMap = nullptr;
	auto result = indexBuffer->Map(0, nullptr, (void**)&indexBufferMap);
	std::copy(std::begin(indices), std::end(indices), indexBufferMap);
	indexBuffer->Unmap(0, nullptr);
}


// インデックスバッファビューを作成
D3D12_INDEX_BUFFER_VIEW createIndexBufferView(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);
	return indexBufferView;
}


// 頂点シェーダーオブジェクトを作成
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


// ピクセルシェーダーオブジェクトを作成
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


// ルートシグネチャを作成（頂点情報以外のデータをシェーダーに送るための仕組み）
// Chapter04では頂点情報しか使わないので、空のルートシグネチャを作成する
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


// グラフィックスパイプラインステートを作成
ID3D12PipelineState* createGraphicsPipelineState(ID3D12Device* dev, ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ID3D12RootSignature* rootSignature) {

	// レンダーターゲットブレンドディスクリプタ
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDescriptor = {};
	renderTargetBlendDescriptor.BlendEnable = false;
	renderTargetBlendDescriptor.LogicOpEnable = false;
	renderTargetBlendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// グラフィックスパイプラインステートディスクリプタ
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
	gPipelineStateDescriptor.BlendState.RenderTarget[0] = renderTargetBlendDescriptor;
	gPipelineStateDescriptor.InputLayout.pInputElementDescs = inputLayout;
	gPipelineStateDescriptor.InputLayout.NumElements = _countof(inputLayout);
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


// ビューポートを作成
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


// シザー矩形を作成
D3D12_RECT createScissorRect(int windowWidth, int windowHeight) {
	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = scissorRect.left + windowWidth;
	scissorRect.bottom = scissorRect.top + windowHeight;
	return scissorRect;
}


// レンダリング処理（のコマンドリストへの登録）
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect) {

	// バリアを設定
	ID3D12Resource* backBuffer;
	auto bufferIdx = swapChain->GetCurrentBackBufferIndex();
	auto result = swapChain->GetBuffer(bufferIdx, IID_PPV_ARGS(&backBuffer));
	auto resourceBarrier = createResourceBarrier(backBuffer);

	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &resourceBarrier);

	// パイプラインステートをセット
	commandList->SetPipelineState(pipelineState);

	// これから使うレンダーターゲットビューとしてrtvHandleをセット
	auto rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bufferIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	// クリア
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// レンダリング設定
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// レンダリング
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// バリアによる完了待ち
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}


int main() {

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	std::vector<XMFLOAT3> vertices = {
		{-0.4f, -0.7f, 0.0f} , // 左下
		{-0.4f, 0.7f,  0.0f} , // 左上
		{0.4f,  -0.7f, 0.0f} , // 右下
		{0.4f,  0.7f,  0.0f} , // 右上
	};
	std::vector<unsigned short> indices = { 0,1,2, 2,1,3 };

	// Chapter03
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
	
	// 頂点座標
	auto vertexHeapProperties = createHeapProperties();
	auto vertexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(vertices[0])) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, vertexHeapProperties, vertexResourceDescriptor);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);

	// インデックス
	auto indexHeapProperties = createHeapProperties();
	auto indexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(indices[0])) * indices.size());
	auto indexBuffer = createIndexBuffer(dev, indexHeapProperties, indexResourceDescriptor);
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);

	// シェーダー
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();

	// パイプライン
	auto rootSignature = createRootSignature(dev);
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

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

		render(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState, viewport, scissorRect);
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