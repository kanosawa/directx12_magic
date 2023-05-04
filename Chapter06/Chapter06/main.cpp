#include "chapter03.h"
#include "chapter04.h"
#include "chapter05.h"
#include "chapter06.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")

struct TexRGBA {
	unsigned char R, G, B, A;
};


void main() {

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	std::vector<Vertex> vertices = {
		{{-0.4f, -0.7f, 0.0f}, {0.0f, 1.0f}}, //左下
		{{-0.4f,  0.7f, 0.0f}, {0.0f, 0.0f}}, //左上
		{{ 0.4f, -0.7f, 0.0f}, {1.0f, 1.0f}}, //右下
		{{ 0.4f,  0.7f, 0.0f}, {1.0f, 0.0f}}  //右上
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

	// Chapter04
	auto vertexHeapProperties = createHeapProperties();
	auto vertexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(vertices[0])) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, vertexHeapProperties, vertexResourceDescriptor);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);
	auto indexHeapProperties = createHeapProperties();
	auto indexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(indices[0])) * indices.size());
	auto indexBuffer = createIndexBuffer(dev, indexHeapProperties, indexResourceDescriptor);
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();
	auto rootSignature = createRootSignature(dev);
	auto inputLayout = createInputLayout();
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

	// Chapter05
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	result = LoadFromWICFile(L"textest.png", WIC_FLAGS_NONE, &metadata, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);
	auto texHeapProperties = createTexHeapProperties();
	auto texBuffer = createTexBuffer(dev, texHeapProperties, img, metadata);
	auto basicDescriptorHeap = createBasicDescriptorHeap(dev, 2);
	createShaderResourceView(dev, texBuffer, basicDescriptorHeap, metadata.format);

	// 定数バッファ
	auto constBuffer = createConstBuffer(dev);
	createConstantBufferView(dev, constBuffer, basicDescriptorHeap, 1);  // ShaderResourceViewの後なのでidx=1
	
	// マトリクス作成
	XMFLOAT3 eye(0, 0, -5);
	XMFLOAT3 target(0, 0, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 1.0f, 10.0f);
	XMMATRIX* mapMatrix;
	result = constBuffer->Map(0, nullptr, (void**)&mapMatrix);

	auto fence = createFence(dev);
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	UINT64 fenceVal = 0;
	float angle = 0.0f;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) break;

		angle += 0.1f;
		auto worldMat = XMMatrixRotationY(angle);
		*mapMatrix = worldMat * viewMat * projMat;

		render(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState, viewport, scissorRect, basicDescriptorHeap);
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