#include "chapter03.h"
#include "chapter04.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")


// ルートシグネチャを作成（頂点情報以外のデータをシェーダーに送るための仕組み）
// Chapter04では頂点情報しか使わないので、空のルートシグネチャを作成する
ID3D12RootSignature* createRootSignature(ID3D12Device* dev) {

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDescriptor = {};
	rootSignatureDescriptor.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* rootSignatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto result = D3D12SerializeRootSignature(
		&rootSignatureDescriptor,
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


// レンダリング処理（のコマンドリストへの登録）
void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView, IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect) {

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

	// レンダリング設定
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// レンダリング
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);

	// バリアによる完了待ち
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}


int main() {

	const unsigned int windowWidth = 1280;
	const unsigned int windowHeight = 720;

	std::vector<XMFLOAT3> vertices = {
		{-0.4f, -0.7f, 0.0f} , // 左下
		{-0.4f, 0.7f,  0.0f} , // 左上
		{0.4f,  -0.7f, 0.0f} , // 右下
		{0.4f,  0.7f,  0.0f} , // 右上
	};
	std::vector<unsigned short> indices = { 0,1,2, 2,1,3 };

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
	
	// 頂点座標
	auto vertexHeapProperties = createHeapProperties();
	auto vertexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(vertices[0])) * vertices.size());
	auto vertexBuffer = createVertexBuffer(dev, vertexHeapProperties, vertexResourceDescriptor);
	mapVertexBuffer(vertexBuffer, vertices);
	auto vertexBufferView = createVertexBufferView(vertexBuffer, vertices);

	// インデックス
	auto indexHeapProperties = createHeapProperties();
	auto indexResourceDescriptor = createResourceDescriptor(UINT64(sizeof(indices[0])) * indices.size());
	auto indexBuffer = createIndexBuffer(dev, indexHeapProperties, indexResourceDescriptor);
	mapIndexBuffer(indexBuffer, indices);
	auto indexBufferView = createIndexBufferView(indexBuffer, indices);

	// シェーダー
	auto vertexShaderBlob = createVertexShaderBlob();
	auto pixelShaderBlob = createPixelShaderBlob();

	// パイプライン
	auto rootSignature = createRootSignature(dev);
	auto inputLayout = createInputLayout();
	auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);

	auto fence = createFence(dev);
	ShowWindow(hwnd, SW_SHOW);

	MSG msg = {};
	UINT64 fenceVal = 0;
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT) break;

		render(dev, rtvDescriptorHeap, commandList, vertexBufferView, indexBufferView, swapChain, rootSignature, pipelineState, viewport, scissorRect);
		commandList->Close();

		ID3D12CommandList* constCommandList[] = { commandList };
		commandQueue->ExecuteCommandLists(1, constCommandList);

		// レンダリング完了待ち
		commandQueue->Signal(fence, ++fenceVal);
		auto event = CreateEvent(nullptr, false, false, nullptr);
		fence->SetEventOnCompletion(fenceVal, event);
		WaitForSingleObject(event, INFINITE);
		CloseHandle(event);

		commandAllocator->Reset();
		commandList->Reset(commandAllocator, nullptr);

		swapChain->Present(1, 0);
	}

	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);

}