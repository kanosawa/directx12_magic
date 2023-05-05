#include "chapter03.h"
#include "chapter05.h"
#include "chapter07.h"


void readPmdHeader(FILE* fp) {
	char signature[3] = {};
	PMDHeader pmdheader = {};
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);
}


std::vector<PMD_VERTEX> readPmdVertices(FILE* fp) {
	size_t pmdvertex_size = 38;
	unsigned int vertNum;
	fread(&vertNum, sizeof(vertNum), 1, fp);
	std::vector<PMD_VERTEX> vertices(vertNum);
	for (auto i = 0; i < vertNum; i++) {
		fread(&vertices[i], pmdvertex_size, 1, fp);
	}
	return vertices;
}


std::vector<unsigned short> readPmdIndices(FILE* fp) {
	unsigned int indicesNum;
	fread(&indicesNum, sizeof(indicesNum), 1, fp);
	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);
	return indices;
}


PMD_MODEL_07 readPmdFile07(std::string pmdFileName) {

	FILE* fp;
	fopen_s(&fp, pmdFileName.c_str(), "rb");

	PMD_MODEL_07 pmd_model = {};
	readPmdHeader(fp);
	pmd_model.vertices = readPmdVertices(fp);
	pmd_model.indices = readPmdIndices(fp);
	fclose(fp);

	return pmd_model;
}


void mapVertexBuffer07(ID3D12Resource* vertexBuffer, std::vector<PMD_VERTEX> vertices) {
	PMD_VERTEX* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


D3D12_VERTEX_BUFFER_VIEW createVertexBufferView07(ID3D12Resource* vertexBuffer, std::vector<PMD_VERTEX> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vertices.size() * sizeof(PMD_VERTEX);
	vertexBufferView.StrideInBytes = sizeof(PMD_VERTEX);
	return vertexBufferView;
}


std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout07() {
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BONE_NO", 0, DXGI_FORMAT_R16G16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WEIGHT", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "EDGE_FLG", 0, DXGI_FORMAT_R8_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
	return inputLayout;
}

ID3D12Resource* createConstBuffer07(ID3D12Device* dev) {
	ID3D12Resource* constBuffer = nullptr;
	auto constHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constResourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(MatricesData) + 0xff) & ~0xff);
	auto result = dev->CreateCommittedResource(&constHeapProperties, D3D12_HEAP_FLAG_NONE, &constResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer));
	return constBuffer;
}


ID3D12Resource* createDepthBuffer(ID3D12Device* dev, int windowWidth, int windowHeight) {
	D3D12_RESOURCE_DESC depthResDesc = {};
	depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResDesc.Width = windowWidth;
	depthResDesc.Height = windowHeight;
	depthResDesc.DepthOrArraySize = 1;
	depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResDesc.SampleDesc.Count = 1;
	depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES depthHeapProp = {};
	depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
	depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_CLEAR_VALUE depthClearValue = {};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

	ID3D12Resource* depthBuffer = nullptr;
	auto result = dev->CreateCommittedResource(&depthHeapProp, D3D12_HEAP_FLAG_NONE, &depthResDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthClearValue, IID_PPV_ARGS(&depthBuffer));

	return depthBuffer;
}


ID3D12DescriptorHeap* createDepthDescriptorHeap(ID3D12Device* dev, ID3D12Resource* depthBuffer) {
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ID3D12DescriptorHeap* dsvHeap = nullptr;
	auto result = dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvHeap));
	return dsvHeap;
}


void createDepthBufferView(ID3D12Device* dev, ID3D12Resource* depthBuffer, ID3D12DescriptorHeap* dsvHeap) {
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dev->CreateDepthStencilView(depthBuffer, &dsvDesc, dsvHeap->GetCPUDescriptorHandleForHeapStart());
}


void render07(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
	D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState,
	D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap, ID3D12DescriptorHeap* depthDescriptorHeap, unsigned int indicesNum)
{
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

	// デプスバッファの設定（Chapter07で追加）
	auto depthHandle = depthDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, true, &depthHandle);

	// クリア
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);  // デプスバッファのクリア

	// レンダリング設定（Chapter04まで）
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// レンダリング設定（Chapter05で追加）
	commandList->SetDescriptorHeaps(1, &basicDescHeap);
	commandList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

	// レンダリング
	commandList->DrawIndexedInstanced(indicesNum, 1, 0, 0, 0);

	// バリアによる完了待ち
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}

