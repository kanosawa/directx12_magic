#include "chapter03.h"
#include "chapter04.h"
#include "chapter05.h"
#include "chapter06.h"
#include "chapter07.h"
#include "chapter08.h"

#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")

using namespace DirectX;
using namespace std;


int main() {

	// PMDファイル読み込み
	string strModelPath = "model/巡音ルカ.pmd";
	auto pmdModel = readPmdFile08(strModelPath);
	auto vertices = pmdModel.vertices;
	auto indices = pmdModel.indices;
	auto materials = transformMaterials(pmdModel.materials);
	auto materialNum = materials.size();

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

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

	ShowWindow(hwnd, SW_SHOW);

	// Chapter04
	auto vertexBuffer = createVertexBuffer(dev, UINT64(sizeof(vertices[0])) * vertices.size());
	mapVertexBuffer07(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView07(vertexBuffer, vertices);
	auto indexBuffer = createIndexBuffer(dev, UINT64(sizeof(indices[0])) * indices.size());
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();
	auto rootSignature = createRootSignature08(dev);
	auto inputLayout = createInputLayout07();
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

	// マテリアル
	auto materialBuffSize = (sizeof(MaterialForHlsl) + 0xff) & ~0xff;
	auto materialBuffer = createMaterialBuffer(dev, materialBuffSize * materials.size());
	mapMaterialBuffer(materialBuffer, materials);
	auto materialDescriptorHeap = createCbvSrvUavDescriptorHeap(dev, materialNum * 5);
	auto textureResources = createTextureResources(dev, materials, strModelPath);
	createMaterialBufferView(dev, materialBuffer, materialDescriptorHeap, textureResources, materialNum);

	// 座標変換マトリクス
	auto basicDescriptorHeap = createCbvSrvUavDescriptorHeap(dev, 1);
	auto constBuffer = createConstBuffer07(dev);
	createConstantBufferView(dev, constBuffer, basicDescriptorHeap, 0);

	// デプスバッファ
	auto depthBuffer = createDepthBuffer(dev, windowWidth, windowHeight);
	auto depthDescriptorHeap = createDepthDescriptorHeap(dev, depthBuffer);
	createDepthBufferView(dev, depthBuffer, depthDescriptorHeap);

	struct SceneData {
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX proj;
		XMFLOAT3 eye;
	};

	XMMATRIX worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 1.0f, 100.0f);

	SceneData* mapScene=nullptr;
	auto result = constBuffer->Map(0,nullptr,(void**)&mapScene);
	mapScene->world = worldMat;
	mapScene->view= viewMat;
	mapScene->proj = projMat;
	mapScene->eye = eye;

	MSG msg = {};
	UINT64 fenceVal = 0;
	float angle = 0.0f;
	auto fence = createFence(dev);
	auto dsvH = depthDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	while (true) {

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) break;

		worldMat = XMMatrixRotationY(angle);
		mapScene->world = worldMat;
		mapScene->view = viewMat;
		mapScene->proj = projMat;
		angle += 0.01f;

		render08(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState,
			viewport, scissorRect, basicDescriptorHeap, materialDescriptorHeap, depthDescriptorHeap, materials);
		commandList->Close();

		ID3D12CommandList* cmdlists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, cmdlists);

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
	return 0;
}