#include "chapter07.h"


void mapVertexBuffer(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices) {
	unsigned char* vertexBufferMap = nullptr;
	auto result = vertexBuffer->Map(0, nullptr, (void**)&vertexBufferMap);
	std::copy(std::begin(vertices), std::end(vertices), vertexBufferMap);
	vertexBuffer->Unmap(0, nullptr);
}


D3D12_VERTEX_BUFFER_VIEW createVertexBufferView(ID3D12Resource* vertexBuffer, std::vector<unsigned char> vertices, size_t pmdvertex_size) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = vertices.size();
	vertexBufferView.StrideInBytes = pmdvertex_size;
	return vertexBufferView;
}