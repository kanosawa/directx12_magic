#include "chapter03.h"
#include "chapter04.h"
#include "chapter05.h"


void mapVertexBuffer05(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices) {
	Vertex* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


D3D12_VERTEX_BUFFER_VIEW createVertexBufferView05(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}


std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout05() {
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	return inputLayout;
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


ID3D12Resource* loadTextureAndCreateBuffer(ID3D12Device* dev, const wchar_t* textureFilename) {

	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto result = LoadFromWICFile(textureFilename, WIC_FLAGS_NONE, &metadata, scratchImg);
	auto img = scratchImg.GetImage(0, 0, 0);

	auto texHeapProperties = createTexHeapProperties();
	auto texResourceDesc = createTexResourceDescriptor(metadata);

	ID3D12Resource* texBuffer = nullptr;
	result = dev->CreateCommittedResource(
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


ID3D12DescriptorHeap* createCbvSrvUavDescriptorHeap(ID3D12Device* dev, UINT64 numDescriptors) {
	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC texDescHeapDesc = {};
	texDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	texDescHeapDesc.NodeMask = 0;
	texDescHeapDesc.NumDescriptors = numDescriptors;
	texDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = dev->CreateDescriptorHeap(&texDescHeapDesc, IID_PPV_ARGS(&texDescHeap));
	return texDescHeap;
}


void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* textureDescriptorHeap, UINT64 idx) {
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	auto heapHandle = textureDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	heapHandle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * idx;
	dev->CreateShaderResourceView(texBuffer, &shaderResourceViewDesc, heapHandle);
}


ID3D12RootSignature* createRootSignature05(ID3D12Device* dev) {

	// ディスクリプタテーブルレンジ（複数のディスクリプタをまとめて使用できるようにするための仕組み）
	D3D12_DESCRIPTOR_RANGE descTableRange = {};
	descTableRange.NumDescriptors = 1;
	descTableRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTableRange.BaseShaderRegister = 0;
	descTableRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ルートパラメーター（ディスクリプタテーブルの実体。ディスクリプタテーブルはテクスチャなどをCPU/GPUで共通認識するための仕組み）
	D3D12_ROOT_PARAMETER rootParam = {};
	rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParam.DescriptorTable.pDescriptorRanges = &descTableRange;
	rootParam.DescriptorTable.NumDescriptorRanges = 1;

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


void render05(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView,
	D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState,
	D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap)
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
	commandList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	// クリア
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

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
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// バリアによる完了待ち
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}
