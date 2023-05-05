#include "chapter03.h"
#include "chapter04.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


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
	auto vertexBuffer = createVertexBuffer(dev, UINT64(sizeof(vertices[0])) * vertices.size());
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);

	// �C���f�b�N�X
	auto indexBuffer = createIndexBuffer(dev, UINT64(sizeof(indices[0])) * indices.size());
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);

	// �V�F�[�_�[
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();

	// �p�C�v���C��
	auto rootSignature = createRootSignature(dev);
	auto inputLayout = createInputLayout04();
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
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

		render04(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState, viewport, scissorRect);
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