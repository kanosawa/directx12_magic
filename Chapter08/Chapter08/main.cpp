//�R���X�^���g�o�b�t�@�ōs���]��
#include<map>
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


ID3D12Device* dev = nullptr;
IDXGIFactory6* dxgiFactory = nullptr;
ID3D12CommandAllocator* commandAllocator = nullptr;
ID3D12GraphicsCommandList* commandList = nullptr;
ID3D12CommandQueue* commandQueue = nullptr;
IDXGISwapChain4* swapChain = nullptr;


int main() {

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	// Chapter03
	auto windowClass = createWindowClass();
	auto hwnd = createWindowHandle(windowClass, windowWidth, windowHeight);
	/*
	auto dev = createDevice();
	auto dxgiFactory = createFactory();
	auto commandAllocator = createCommandAllocator(dev);
	auto commandList = createCommandList(dev, commandAllocator);
	auto commandQueue = createCommandQueue(dev);
	auto swapChain = createSwapChain(hwnd, dxgiFactory, commandQueue, windowWidth, windowHeight);
	*/
	dev = createDevice();
	dxgiFactory = createFactory();
	commandAllocator = createCommandAllocator(dev);
	commandList = createCommandList(dev, commandAllocator);
	commandQueue = createCommandQueue(dev);
	swapChain = createSwapChain(hwnd, dxgiFactory, commandQueue, windowWidth, windowHeight);
	
	auto rtvDescriptorHeap = createRenderTargetViewDescriptorHeap(dev);
	auto backBuffers = createRenderTargetViewAndGetBuckBuffers(dev, swapChain, rtvDescriptorHeap);

	ShowWindow(hwnd, SW_SHOW);//�E�B���h�E�\��

	string strModelPath = "model/�������J.pmd";
	auto pmdModel = readPmdFile08(strModelPath);
	auto vertices = pmdModel.vertices;
	auto indices = pmdModel.indices;
	auto materials = transformMaterials(pmdModel.materials);
	auto materialNum = materials.size();

	auto textureResources = createTextureResources(dev, materials, strModelPath);

	// Chapter04
	auto vertexBuffer = createVertexBuffer(dev, UINT64(sizeof(vertices[0])) * vertices.size());
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);
	auto indexBuffer = createIndexBuffer(dev, UINT64(sizeof(indices[0])) * indices.size());
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();
	auto rootSignature = createRootSignature(dev);
	auto inputLayout = createInputLayout();
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

	// Chapter05, 06
	auto basicDescriptorHeap = createCbvSrvUavDescriptorHeap(dev, 1);
	auto constBuffer = createConstBuffer(dev);
	createConstantBufferView(dev, constBuffer, basicDescriptorHeap, 0);

	// �f�v�X�o�b�t�@
	auto depthBuffer = createDepthBuffer(dev, windowWidth, windowHeight);
	auto depthDescriptorHeap = createDepthDescriptorHeap(dev, depthBuffer);
	createDepthBufferView(dev, depthBuffer, depthDescriptorHeap);

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	auto result = dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	auto materialBuffSize = (sizeof(MaterialForHlsl) + 0xff) & ~0xff;
	auto materialBuffer = createMaterialBuffer(dev, materialBuffSize * materials.size());
	mapMaterialBuffer(materialBuffer, materials);
	auto materialDescriptorHeap = createCbvSrvUavDescriptorHeap(dev, materialNum * 5);
	createMaterialBufferView(dev, materialBuffer, materialDescriptorHeap, textureResources, materialNum);

	ID3DBlob* errorBlob = nullptr;


	//�V�F�[�_���ɓn�����߂̊�{�I�Ȋ��f�[�^
	struct SceneData {
		XMMATRIX world;//���[���h�s��
		XMMATRIX view;//�r���[�v���W�F�N�V�����s��
		XMMATRIX proj;//
		XMFLOAT3 eye;//���_���W
	};

	//�萔�o�b�t�@�쐬
	XMMATRIX worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4,//��p��45��
		static_cast<float>(windowWidth) / static_cast<float>(windowHeight),//�A�X��
		1.0f,//�߂���
		100.0f//������
	);

	SceneData* mapScene=nullptr;//�}�b�v��������|�C���^
	result = constBuffer->Map(0,nullptr,(void**)&mapScene);//�}�b�v
	//�s��̓��e���R�s�[
	mapScene->world = worldMat;
	mapScene->view= viewMat;
	mapScene->proj = projMat;
	mapScene->eye = eye;



	MSG msg = {};
	unsigned int frame = 0;
	float angle = 0.0f;
	auto dsvH = depthDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	while (true) {
		worldMat=XMMatrixRotationY(angle);
		mapScene->world = worldMat;
		mapScene->view= viewMat;
		mapScene->proj = projMat;
		angle += 0.01f;

		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//�����A�v���P�[�V�������I�����Ď���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}

		//DirectX����
		//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->SetPipelineState(pipelineState);


		//�����_�[�^�[�Q�b�g���w��
		auto rtvH = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		
		commandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
		commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//��ʃN���A

		float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//���F
		commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		commandList->SetGraphicsRootSignature(rootSignature);
		
		//WVP�ϊ��s��
		commandList->SetDescriptorHeaps(1, &basicDescriptorHeap);
		commandList->SetGraphicsRootDescriptorTable(0, basicDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//�}�e���A��
		commandList->SetDescriptorHeaps(1, &materialDescriptorHeap);

		auto materialH = materialDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		unsigned int idxOffset = 0;

		auto cbvsrvIncSize= dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)*5;
		for (auto& m : materials) {
			commandList->SetGraphicsRootDescriptorTable(1, materialH);
			commandList->DrawIndexedInstanced(m.indicesNum, 1,idxOffset, 0, 0);
			materialH.ptr += cbvsrvIncSize;
			idxOffset += m.indicesNum;			
		}

		barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		commandList->ResourceBarrier(1, &barrier);

		//���߂̃N���[�Y
		commandList->Close();



		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, cmdlists);
		////�҂�
		++_fenceVal;
		commandQueue->Signal(_fence,_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObjectEx(event, INFINITE,false);
			CloseHandle(event);
		}


		//�t���b�v
		swapChain->Present(0, 0);
		commandAllocator->Reset();//�L���[���N���A
		commandList->Reset(commandAllocator, pipelineState);//�ĂуR�}���h���X�g�����߂鏀��

	}
	//�����N���X�g��񂩂�o�^�������Ă�
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	return 0;
}