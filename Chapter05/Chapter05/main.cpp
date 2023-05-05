#include "chapter03.h"
#include "chapter04.h"
#include "chapter05.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")


struct TexRGBA {
	unsigned char R, G, B, A;
};


int main() {

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

	std::vector<TexRGBA> texturedata(256 * 256);
	for (auto& rgba : texturedata) {
		rgba.R = rand() % 256;
		rgba.G = rand() % 256;
		rgba.B = rand() % 256;
		rgba.A = 255;
	}

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
	auto vertexBuffer = createVertexBuffer(dev, UINT64(sizeof(vertices[0])) * vertices.size());
	mapVertexBuffer05(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView05(vertexBuffer, vertices);
	auto indexBuffer = createIndexBuffer(dev, UINT64(sizeof(indices[0])) * indices.size());
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();
	auto rootSignature = createRootSignature05(dev);
	auto inputLayout = createInputLayout05();
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

	// テクスチャ設定
	auto texBuffer = loadTextureAndCreateBuffer(dev, L"textest.png");
	auto texDescriptorHeap = createCbvSrvUavDescriptorHeap(dev, 1);
	createShaderResourceView(dev, texBuffer, texDescriptorHeap, 0);

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

		render05(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState, viewport, scissorRect, texDescriptorHeap);
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