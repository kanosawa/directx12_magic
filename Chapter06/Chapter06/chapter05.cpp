#include "chapter05.h"


void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices) {
	Vertex* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<Vertex> vertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(vertices[0]) * vertices.size();
	vertexBufferView.StrideInBytes = sizeof(vertices[0]);
	return vertexBufferView;
}


std::vector<D3D12_INPUT_ELEMENT_DESC> createInputLayout() {
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


ID3D12Resource* createTexBuffer(ID3D12Device* dev, D3D12_HEAP_PROPERTIES texHeapProperties, const DirectX::Image* img, TexMetadata metadata) {
	auto texResourceDesc = createTexResourceDescriptor(metadata);
	ID3D12Resource* texBuffer = nullptr;
	auto result = dev->CreateCommittedResource(
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


ID3D12DescriptorHeap* createTexDescriptorHeap(ID3D12Device* dev) {
	ID3D12DescriptorHeap* texDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC texDescHeapDesc = {};
	texDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	texDescHeapDesc.NodeMask = 0;
	texDescHeapDesc.NumDescriptors = 1;
	texDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = dev->CreateDescriptorHeap(&texDescHeapDesc, IID_PPV_ARGS(&texDescHeap));
	return texDescHeap;
}


void createShaderResourceView(ID3D12Device* dev, ID3D12Resource* texBuffer, ID3D12DescriptorHeap* texDescHeap, DXGI_FORMAT format) {

	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = format;
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	dev->CreateShaderResourceView(texBuffer, &shaderResourceViewDesc, texDescHeap->GetCPUDescriptorHandleForHeapStart());
}
