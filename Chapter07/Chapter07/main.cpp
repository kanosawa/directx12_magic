#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <DirectXTex.h>
#include <d3dx12.h>
#include "chapter03.h"
#include "chapter04.h"
#include "chapter05.h"
#include "chapter06.h"
#include "chapter07.h"

using namespace DirectX;

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTex.lib")


struct PMDHeader {
	float version;
	char model_name[20];
	char comment[256];
};

struct PMDVertex {
	XMFLOAT3 pos;
	XMFLOAT3 normal;
	XMFLOAT2 uv;
	unsigned short boneNo[2];
	unsigned char boneWeight;
	unsigned char edgeFlg;
};


ID3D12RootSignature* createRootSignature(ID3D12Device* dev) {

	// ディスクリプタテーブルレンジ（複数のディスクリプタをまとめて使用できるようにするための仕組み）
	D3D12_DESCRIPTOR_RANGE descTableRange[2] = {};
	descTableRange[0].NumDescriptors = 1;
	descTableRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTableRange[0].BaseShaderRegister = 0;
	descTableRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	descTableRange[1].NumDescriptors = 1;
	descTableRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTableRange[1].BaseShaderRegister = 0;
	descTableRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメーター（ディスクリプタテーブルの実体。ディスクリプタテーブルはテクスチャなどをCPU/GPUで共通認識するための仕組み）
	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParam.DescriptorTable.pDescriptorRanges = &descTableRange[0];
	rootParam.DescriptorTable.NumDescriptorRanges = 2;  // テクスチャと定数で2

	// サンプラー（uv値によってテクスチャデータからどう色を取り出すかを決めるための設定）
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


void main() {

	auto fp = fopen("Model/初音ミク.pmd", "rb");

	char signature[3] = {};
	PMDHeader pmdheader = {};
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);

	size_t pmdvertex_size = 38;
	std::vector<unsigned char> vertices(vertNum * pmdvertex_size);
	fread(vertices.data(), vertices.size(), 1, fp);

	unsigned int indicesNum;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	
	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);

	fclose(fp);

	auto result = CoInitializeEx(0, COINIT_MULTITHREADED);

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

	// Chapter04
	auto vertexHeapProperties = createHeapProperties();
	auto vertexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(vertices[0])) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, vertexHeapProperties, vertexResourceDescriptor);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices, pmdvertex_size);
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

	// Chapter05, 06（テクスチャ関連は削除）
	auto basicDescriptorHeap = createBasicDescriptorHeap(dev, 1); // テクスチャがないのでnumDescriptorsは1
	auto constBuffer = createConstBuffer(dev);
	createConstantBufferView(dev, constBuffer, basicDescriptorHeap);

	// デプスバッファ
	auto depthBuffer = createDepthBuffer(dev, windowWidth, windowHeight);
	auto depthDescriptorHeap = createDepthDescriptorHeap(dev, depthBuffer);
	createDepthBufferView(dev, depthBuffer, depthDescriptorHeap);
	
	auto worldMat = XMMatrixRotationY(XM_PIDIV4);
	XMFLOAT3 eye(0, 10, -15);
	XMFLOAT3 target(0, 10, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 1.0f, 100.0f);
	MatricesData* mapMatrix;
	result = constBuffer->Map(0, nullptr, (void**)&mapMatrix);
	mapMatrix->world = worldMat;
	mapMatrix->viewproj = viewMat * projMat;

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
		worldMat = XMMatrixRotationY(angle);
		mapMatrix->world = worldMat;
		mapMatrix->viewproj = viewMat * projMat;

		render(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState, viewport, scissorRect, basicDescriptorHeap, depthDescriptorHeap, indicesNum);
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