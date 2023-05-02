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


// �q�[�v�v���p�e�B���쐬�i���_���W�ƃC���f�b�N�X�p�j
D3D12_HEAP_PROPERTIES createHeapProperties() {
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	return heapProperties;
}


// ���\�[�X�f�B�X�N���v�^���쐬�i���_���W�ƃC���f�b�N�X�p�j
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


// ���_�o�b�t�@�[���쐬
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


// ���_���W�����}�b�v
void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	XMFLOAT3* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


// ���_�o�b�t�@�r���[���쐬
D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}


// �C���f�b�N�X�o�b�t�@�[���쐬
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


// �C���f�b�N�X�����}�b�v
void mapIndexBuffer(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	unsigned short* indexBufferMap = nullptr;
	auto result = indexBuffer->Map(0, nullptr, (void**)&indexBufferMap);
	std::copy(std::begin(indices), std::end(indices), indexBufferMap);
	indexBuffer->Unmap(0, nullptr);
}


// �C���f�b�N�X�o�b�t�@�r���[���쐬
D3D12_INDEX_BUFFER_VIEW createIndexBufferView(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);
	return indexBufferView;
}


// ���_�V�F�[�_�[�I�u�W�F�N�g���쐬
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


// �s�N�Z���V�F�[�_�[�I�u�W�F�N�g���쐬
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


// ���[�g�V�O�l�`�����쐬�i���_���ȊO�̃f�[�^���V�F�[�_�[�ɑ��邽�߂̎d�g�݁j
// Chapter04�ł͒��_��񂵂��g��Ȃ��̂ŁA��̃��[�g�V�O�l�`�����쐬����
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


// �O���t�B�b�N�X�p�C�v���C���X�e�[�g���쐬
ID3D12PipelineState* createGraphicsPipelineState(ID3D12Device* dev, ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ID3D12RootSignature* rootSignature) {

	// �����_�[�^�[�Q�b�g�u�����h�f�B�X�N���v�^
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDescriptor = {};
	renderTargetBlendDescriptor.BlendEnable = false;
	renderTargetBlendDescriptor.LogicOpEnable = false;
	renderTargetBlendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// �C���v�b�g���C�A�E�g
	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// �O���t�B�b�N�X�p�C�v���C���X�e�[�g�f�B�X�N���v�^
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


// �r���[�|�[�g���쐬
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


// �V�U�[��`���쐬
D3D12_RECT createScissorRect(int windowWidth, int windowHeight) {
	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = scissorRect.left + windowWidth;
	scissorRect.bottom = scissorRect.top + windowHeight;
	return scissorRect;
}


// �����_�����O�����i�̃R�}���h���X�g�ւ̓o�^�j
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect) {

	// �o���A��ݒ�
	ID3D12Resource* backBuffer;
	auto bufferIdx = swapChain->GetCurrentBackBufferIndex();
	auto result = swapChain->GetBuffer(bufferIdx, IID_PPV_ARGS(&backBuffer));
	auto resourceBarrier = createResourceBarrier(backBuffer);

	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &resourceBarrier);

	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetPipelineState(pipelineState);

	// ���ꂩ��g�������_�[�^�[�Q�b�g�r���[�Ƃ���rtvHandle���Z�b�g
	auto rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bufferIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	// �N���A
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// �����_�����O�ݒ�
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// �����_�����O
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// �o���A�ɂ�銮���҂�
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}


int main() {

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	std::vector<XMFLOAT3> vertices = {
		{-0.4f, -0.7f, 0.0f} , // ����
		{-0.4f, 0.7f,  0.0f} , // ����
		{0.4f,  -0.7f, 0.0f} , // �E��
		{0.4f,  0.7f,  0.0f} , // �E��
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
	
	// ���_���W
	auto vertexHeapProperties = createHeapProperties();
	auto vertexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(vertices[0])) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, vertexHeapProperties, vertexResourceDescriptor);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);

	// �C���f�b�N�X
	auto indexHeapProperties = createHeapProperties();
	auto indexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(indices[0])) * indices.size());
	auto indexBuffer = createIndexBuffer(dev, indexHeapProperties, indexResourceDescriptor);
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);

	// �V�F�[�_�[
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();

	// �p�C�v���C��
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

		// �����_�����O�����҂�
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