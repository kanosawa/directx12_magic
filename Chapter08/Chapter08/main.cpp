//コンスタントバッファで行列を転送
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


//トゥーンのためのグラデーションテクスチャ
ID3D12Resource*
CreateGrayGradationTexture() {
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 256;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	ID3D12Resource* gradBuff = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&gradBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	
	//上が白くて下が黒いテクスチャデータを作成
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c=0xff;
	for (; it != data.end();it+=4) {
		auto col = (0xff << 24) | RGB(c,c,c);//RGBAが逆並びしているためRGBマクロと0xff<<24を用いて表す。
		//auto col = (0xff << 24) | (c<<16)|(c<<8)|c;//これでもOK
		std::fill(it, it+4, col);
		--c;
	}

	result = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int)*static_cast<UINT>(data.size()));
	return gradBuff;
}

ID3D12Resource*
CreateWhiteTexture() {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* whiteBuff = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&whiteBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0xff);

	result = whiteBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return whiteBuff;
}

ID3D12Resource*
CreateBlackTexture() {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//特殊な設定なのでdefaultでもuploadでもなく
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//ライトバックで
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//転送がL0つまりCPU側から直で
	texHeapProp.CreationNodeMask = 0;//単一アダプタのため0
	texHeapProp.VisibleNodeMask = 0;//単一アダプタのため0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//幅
	resDesc.Height = 4;//高さ
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//レイアウトについては決定しない
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//とくにフラグなし

	ID3D12Resource* blackBuff = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//特に指定なし
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&blackBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), 0x00);

	result = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return blackBuff;
}


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

	ShowWindow(hwnd, SW_SHOW);//ウィンドウ表示

	string strModelPath = "model/巡音ルカ.pmd";
	auto pmdModel = readPmdFile08(strModelPath);
	auto vertices = pmdModel.vertices;
	auto indices = pmdModel.indices;
	auto materials = transformMaterials(pmdModel.materials);
	auto materialNum = materials.size();

	vector<ID3D12Resource*> textureResources(materialNum);
	vector<ID3D12Resource*> sphResources(materialNum);
	vector<ID3D12Resource*> spaResources(materialNum);
	vector<ID3D12Resource*> toonResources(materialNum);

	map<string, ID3D12Resource*> resourceTable;

	for (int material_idx = 0; material_idx < materialNum; ++material_idx) {

		toonResources[material_idx] = loadToonTexture(dev, resourceTable, materials[material_idx].toonIdx);

		if (materials[material_idx].texFilePath.length() == 0) {
			continue;
		}

		vector<string> texFilePathes;
		if (count(materials[material_idx].texFilePath.begin(), materials[material_idx].texFilePath.end(), '*') > 0) { //スプリッタがある
			auto namepair = SplitFileName(materials[material_idx].texFilePath);
			texFilePathes.push_back(string("model/") + namepair.first);
			texFilePathes.push_back(string("model/") + namepair.second);
		}
		else {
			texFilePathes.push_back(string("model/") + materials[material_idx].texFilePath);
		}

		for (auto path_idx = 0; path_idx < texFilePathes.size(); ++path_idx) {
			auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFilePathes[path_idx].c_str());
			auto ext = GetExtension(texFilePath);
			if (ext == "sph") {
				sphResources[material_idx] = loadTexture(dev, resourceTable, texFilePath);
			}
			else if (ext == "spa") {
				spaResources[material_idx] = loadTexture(dev, resourceTable, texFilePath);
			}
			else {
				textureResources[material_idx] = loadTexture(dev, resourceTable, texFilePath);
			}
		}
	}
	
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

	// Chapter05, 06
	auto basicDescriptorHeap = createBasicDescriptorHeap(dev, 2);
	auto constBuffer = createConstBuffer(dev);
	createConstantBufferView(dev, constBuffer, basicDescriptorHeap, 0);

	// デプスバッファ
	auto depthBuffer = createDepthBuffer(dev, windowWidth, windowHeight);
	auto depthDescriptorHeap = createDepthDescriptorHeap(dev, depthBuffer);
	createDepthBufferView(dev, depthBuffer, depthDescriptorHeap);

	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	auto result = dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	auto whiteTex = CreateWhiteTexture();
	auto blackTex = CreateBlackTexture();
	auto gradTex = CreateGrayGradationTexture();

	//マテリアルバッファを作成
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff)&~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materialNum);//勿体ないけど仕方ないですね
	ID3D12Resource* materialBuff = nullptr;
	result = dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&materialBuff)
	);

	//マップマテリアルにコピー
	char* mapMaterial = nullptr;
	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : materials) {
		*((MaterialForHlsl*)mapMaterial) = m.materialForHlsl;//データコピー
		mapMaterial += materialBuffSize;//次のアライメント位置まで進める
	}
	materialBuff->Unmap(0,nullptr);


	ID3D12DescriptorHeap* materialDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = materialNum * 5;//マテリアル数ぶん(定数1つ、テクスチャ3つ)
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;
	
	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//デスクリプタヒープ種別
	result = dev->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(&materialDescHeap));//生成

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize);

	////通常テクスチャビュー作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//後述
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2Dテクスチャ
	srvDesc.Texture2D.MipLevels = 1;//ミップマップは使用しないので1

	auto matDescHeapH = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize= dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (size_t i = 0; i < materialNum; ++i) {
		//マテリアル固定バッファビュー
		dev->CreateConstantBufferView(&matCBVDesc,matDescHeapH);
		matDescHeapH.ptr += incSize;
		matCBVDesc.BufferLocation += materialBuffSize;
		if (textureResources[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			dev->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
		}else{
			srvDesc.Format = textureResources[i]->GetDesc().Format;
			dev->CreateShaderResourceView(textureResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (sphResources[i] == nullptr) {
			srvDesc.Format = whiteTex->GetDesc().Format;
			dev->CreateShaderResourceView(whiteTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = sphResources[i]->GetDesc().Format;
			dev->CreateShaderResourceView(sphResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

		if (spaResources[i] == nullptr) {
			srvDesc.Format = blackTex->GetDesc().Format;
			dev->CreateShaderResourceView(blackTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = spaResources[i]->GetDesc().Format;
			dev->CreateShaderResourceView(spaResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;


		if (toonResources[i] == nullptr) {
			srvDesc.Format = gradTex->GetDesc().Format;
			dev->CreateShaderResourceView(gradTex, &srvDesc, matDescHeapH);
		}
		else {
			srvDesc.Format = toonResources[i]->GetDesc().Format;
			dev->CreateShaderResourceView(toonResources[i], &srvDesc, matDescHeapH);
		}
		matDescHeapH.ptr += incSize;

	}

	ID3DBlob* errorBlob = nullptr;


	//シェーダ側に渡すための基本的な環境データ
	struct SceneData {
		XMMATRIX world;//ワールド行列
		XMMATRIX view;//ビュープロジェクション行列
		XMMATRIX proj;//
		XMFLOAT3 eye;//視点座標
	};

	//定数バッファ作成
	XMMATRIX worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4,//画角は45°
		static_cast<float>(windowWidth) / static_cast<float>(windowHeight),//アス比
		1.0f,//近い方
		100.0f//遠い方
	);

	SceneData* mapScene=nullptr;//マップ先を示すポインタ
	result = constBuffer->Map(0,nullptr,(void**)&mapScene);//マップ
	//行列の内容をコピー
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
		//もうアプリケーションが終わるって時にmessageがWM_QUITになる
		if (msg.message == WM_QUIT) {
			break;
		}

		//DirectX処理
		//バックバッファのインデックスを取得
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->SetPipelineState(pipelineState);


		//レンダーターゲットを指定
		auto rtvH = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		
		commandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
		commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//画面クリア

		float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//白色
		commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		commandList->SetGraphicsRootSignature(rootSignature);
		
		//WVP変換行列
		commandList->SetDescriptorHeaps(1, &basicDescriptorHeap);
		commandList->SetGraphicsRootDescriptorTable(0, basicDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//マテリアル
		commandList->SetDescriptorHeaps(1, &materialDescHeap);

		auto materialH = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
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

		//命令のクローズ
		commandList->Close();



		//コマンドリストの実行
		ID3D12CommandList* cmdlists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, cmdlists);
		////待ち
		++_fenceVal;
		commandQueue->Signal(_fence,_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObjectEx(event, INFINITE,false);
			CloseHandle(event);
		}


		//フリップ
		swapChain->Present(0, 0);
		commandAllocator->Reset();//キューをクリア
		commandList->Reset(commandAllocator, pipelineState);//再びコマンドリストをためる準備

	}
	//もうクラス使わんから登録解除してや
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	return 0;
}