#include "chapter04.h"


D3D12_HEAP_PROPERTIES createHeapProperties() {
	D3D12_HEAP_PROPERTIES heapProperties = {};
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	return heapProperties;
}


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


void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	XMFLOAT3* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<XMFLOAT3> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}


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


void mapIndexBuffer(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	unsigned short* indexBufferMap = nullptr;
	auto result = indexBuffer->Map(0, nullptr, (void**)&indexBufferMap);
	std::copy(std::begin(indices), std::end(indices), indexBufferMap);
	indexBuffer->Unmap(0, nullptr);
}


D3D12_INDEX_BUFFER_VIEW createIndexBufferView(ID3D12Resource* indexBuffer, std::vector<unsigned short> indices) {
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);
	return indexBufferView;
}


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


std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout() {
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	return inputLayout;
}


ID3D12PipelineState* createGraphicsPipelineState(ID3D12Device* dev, ID3DBlob* vertexShaderBlob, ID3DBlob* pixelShaderBlob, ID3D12RootSignature* rootSignature, std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout) {

	// レンダーターゲットブレンドディスクリプタ
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDescriptor = {};
	renderTargetBlendDescriptor.BlendEnable = false;
	renderTargetBlendDescriptor.LogicOpEnable = false;
	renderTargetBlendDescriptor.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// グラフィックスパイプラインステートディスクリプタ
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
	gPipelineStateDescriptor.InputLayout.pInputElementDescs = &inputLayout[0];
	gPipelineStateDescriptor.InputLayout.NumElements = inputLayout.size();
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


D3D12_RECT createScissorRect(int windowWidth, int windowHeight) {
	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = scissorRect.left + windowWidth;
	scissorRect.bottom = scissorRect.top + windowHeight;
	return scissorRect;
}