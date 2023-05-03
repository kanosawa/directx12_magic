#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <DirectXTex.h>
#include "chapter03.h"
#include "chapter04.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

using namespace DirectX;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

struct TexRGBA {
	unsigned char R, G, B, A;
};


ID3D12RootSignature* createRootSignature(ID3D12Device* dev) {

	D3D12_DESCRIPTOR_RANGE descTableRange = {};
	descTableRange.NumDescriptors = 1;
	descTableRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTableRange.BaseShaderRegister = 0;
	descTableRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam.DescriptorTable.pDescriptorRanges = &descTableRange;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;

	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = &rootParam;
	rootSignatureDesc.NumParameters = 1;
	rootSignatureDesc.pStaticSamplers = &samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 1;

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDesc,
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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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


D3D12_HEAP_PROPERTIES createTexHeapProperties() {
	D3D12_HEAP_PROPERTIES texHeapProperties = {};
	texHeapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	texHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	texHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
	texHeapProperties.CreationNodeMask = 0;
	texHeapProperties.VisibleNodeMask = 0;
	return texHeapProperties;
}


D3D12_RESOURCE_DESC createTexResourceDescriptor(TexMetadata metadata) {
	D3D12_RESOURCE_DESC texResourceDesc = {};
	texResourceDesc.Format = metadata.format;
	texResourceDesc.Width = metadata.width;
	texResourceDesc.Height = metadata.height;
	texResourceDesc.DepthOrArraySize = metadata.arraySize;
	texResourceDesc.SampleDesc.Count = 1;
	texResourceDesc.SampleDesc.Quality = 0;
	texResourceDesc.MipLevels = metadata.mipLevels;
	texResourceDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	texResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	return texResourceDesc;
}


ID3D12Resource* createTexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES texHeapProperties, const DirectX::Image* img, TexMetadata metadata) {

	auto texResourceDesc = createTexResourceDescriptor(metadata);

	ID3D12Resource* texBuffer = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&texResourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texBuffer)
	);

	result = texBuffer->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
	return texBuffer;
}


ID3D12DescriptorHeap* createTexDescriptorHeap(ID3D12Device* dev) {
	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC texDescHeapDesc = {};
	texDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	texDescHeapDesc.NodeMask = 0;
	texDescHeapDesc.NumDescriptors = 1;
	texDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = dev->CreateDescriptorHeap(&texDescHeapDesc, IID_PPV_ARGS(&texDescHeap));
	return texDescHeap;
}


void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* texDescHeap, DXGI_FORMAT format) {
	
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(texBuffer, &shaderResourceViewDesc, texDescHeap->GetCPUDescriptorHandleForHeapStart());
}


void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices) {
	Vertex* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}


void main() {

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	std::vector<Vertex> vertices = {
		{{-0.4f, -0.7f, 0.0f}, {0.0f, 1.0f}}, //ç∂â∫
		{{-0.4f,  0.7f, 0.0f}, {0.0f, 0.0f}}, //ç∂è„
		{{ 0.4f, -0.7f, 0.0f}, {1.0f, 1.0f}}, //âEâ∫
		{{ 0.4f,  0.7f, 0.0f}, {1.0f, 0.0f}}  //âEè„
	};

	std::vector<TexRGBA> texturedata(256 * 256);
	for (auto& rgba : texturedata) {
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}

	std::vector<unsigned short> indices = { 0,1,2, 2,1,3 };

	auto windowClass = createWindowClass();
	auto hwnd = createWindowHandle(windowClass, windowWidth, windowHeight);
	auto dev = createDevice();
	auto dxgiFactory = createFactory();
	auto commandAllocator = createCommandAllocator(dev);
	auto commandList = createCommandList(dev, commandAllocator);
	auto commandQueue = createCommandQueue(dev);
	auto swapChain = createSwapChain(hwnd, dxgiFactory, commandQueue, windowWidth, windowHeight);
	auto descriptorHeap = createRenderTargetViewDescriptorHeap(dev);
	auto backBuffers = createRenderTargetViewAndGetBuckBuffers(dev, swapChain, descriptorHeap);

	auto heapProperties = createHeapProperties();
	auto resourceDesc = createResourceDescriptor(sizeof(vertices[0]) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, heapProperties, resourceDesc);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);
	resourceDesc.Width = sizeof(indices[0]) * indices.size();
	auto indexBuffer = createIndexBuffer(dev, heapProperties, resourceDesc);
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();
	auto rootSignature = createRootSignature(dev);
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

	auto texHeapProperties = createTexHeapProperties();

	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	result = LoadFromWICFile(L"textest.png", WIC_FLAGS_NONE, &metadata, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);

	auto texBuffer = createTexBuffer(dev, texHeapProperties, img, metadata);
	auto texDescHeap = createTexDescriptorHeap(dev);
	createShaderResourceView(dev, texBuffer, texDescHeap, metadata.format);

	


	ID3D12Fence* fence = nullptr;
	UINT64 fenceVal = 0;
	result = dev->CreateFence(fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));

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
		commandList->SetDescriptorHeaps(1, &texDescHeap);
		commandList->SetGraphicsRootDescriptorTable(0, texDescHeap->GetGPUDescriptorHandleForHeapStart());

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