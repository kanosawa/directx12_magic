#include "chapter06.h"


ID3D12DescriptorHeap* createBasicDescriptorHeap(ID3D12Device* dev, UINT64 numDescriptors) {
	ID3D12DescriptorHeap* basicDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC basicDescriptorHeapDesc = {};
	basicDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	basicDescriptorHeapDesc.NodeMask = 0;
	basicDescriptorHeapDesc.NumDescriptors = numDescriptors;
	basicDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	auto result = dev->CreateDescriptorHeap(&basicDescriptorHeapDesc, IID_PPV_ARGS(&basicDescriptorHeap));
	return basicDescriptorHeap;
}

/*
ID3D12Resource* createConstBuffer(ID3D12Device* dev) {
	ID3D12Resource* constBuffer = nullptr;
	auto constHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto constResourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(XMMATRIX) + 0xff) & ~0xff);
	auto result = dev->CreateCommittedResource(&constHeapProperties, D3D12_HEAP_FLAG_NONE, &constResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&constBuffer));
	return constBuffer;
}
*/

void createConstantBufferView(ID3D12Device* dev, ID3D12Resource* constBuffer, ID3D12DescriptorHeap* basicDescriptorHeap) {
	D3D12_CONSTANT_BUFFER_VIEW_DESC constantBufferViewDesc = {};
	constantBufferViewDesc.BufferLocation = constBuffer->GetGPUVirtualAddress();
	constantBufferViewDesc.SizeInBytes = static_cast<UINT>(constBuffer->GetDesc().Width);
	auto heapHandle = basicDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	heapHandle.ptr += dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	dev->CreateConstantBufferView(&constantBufferViewDesc, heapHandle);
}
