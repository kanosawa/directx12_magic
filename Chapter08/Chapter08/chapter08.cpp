#include <functional>
#include <map>
#include "chapter04.h"
#include "chapter05.h"
#include "chapter08.h"

using namespace DirectX;


std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath) {
	//�t�@�C���̃t�H���_��؂��\��/�̓��ނ��g�p�����\��������
	//�Ƃ�����������\��/�𓾂���΂����̂ŁA�o����rfind���Ƃ��r����
	//int�^�ɑ�����Ă���̂͌�����Ȃ������ꍇ��rfind��epos(-1��0xffffffff)��Ԃ�����
	auto pathIndex1 = modelPath.rfind('/');
	auto pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex + 1);
	return folderPath + texPath;
}


std::string GetExtension(const std::string& path) {
	auto idx = path.rfind('.');
	return path.substr(idx + 1, path.length() - idx - 1);
}


std::wstring GetExtension(const std::wstring& path) {
	auto idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}


std::pair<std::string, std::string> SplitFileName(const std::string& path, const char splitter) {
	auto idx = path.find(splitter);
	std::pair<std::string, std::string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx + 1, path.length() - idx - 1);
	return ret;
}


std::wstring GetWideStringFromString(const std::string& str) {
	//�Ăяo��1���(�����񐔂𓾂�)
	auto num1 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, nullptr, 0);

	std::wstring wstr;//string��wchar_t��
	wstr.resize(num1);//����ꂽ�����񐔂Ń��T�C�Y

	//�Ăяo��2���(�m�ۍς݂�wstr�ɕϊ���������R�s�[)
	auto num2 = MultiByteToWideChar(CP_ACP,
		MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
		str.c_str(), -1, &wstr[0], num1);

	assert(num1 == num2);//�ꉞ�`�F�b�N
	return wstr;
}


std::vector<PMDMaterial> readPmdMaterials(FILE* fp) {
	unsigned int materialNum;
	fread(&materialNum, sizeof(materialNum), 1, fp);
	std::vector<PMDMaterial> materials(materialNum);
	fread(materials.data(), materials.size() * sizeof(PMDMaterial), 1, fp);
	return materials;
}


PMD_MODEL_08 readPmdFile08(std::string pmdFileName) {

	FILE* fp;
	fopen_s(&fp, pmdFileName.c_str(), "rb");

	PMD_MODEL_08 pmd_model = {};
	readPmdHeader(fp);
	pmd_model.vertices = readPmdVertices(fp);
	pmd_model.indices = readPmdIndices(fp);
	pmd_model.materials = readPmdMaterials(fp);
	fclose(fp);

	return pmd_model;
}


std::vector<Material> transformMaterials(std::vector<PMDMaterial> pmdMaterials) {
	std::vector<Material> materials(pmdMaterials.size());
	for (int i = 0; i < pmdMaterials.size(); ++i) {
		materials[i].materialForHlsl.diffuse = pmdMaterials[i].diffuse;
		materials[i].materialForHlsl.alpha = pmdMaterials[i].alpha;
		materials[i].materialForHlsl.specular = pmdMaterials[i].specular;
		materials[i].materialForHlsl.specularity = pmdMaterials[i].specularity;
		materials[i].materialForHlsl.ambient = pmdMaterials[i].ambient;
		materials[i].toonIdx = pmdMaterials[i].toonIdx;
		materials[i].edgeFlg = pmdMaterials[i].edgeFlg;
		materials[i].indicesNum = pmdMaterials[i].indicesNum;
		materials[i].texFilePath = pmdMaterials[i].texFilePath;
	}
	return materials;
}


ID3D12Resource* loadTextureAndCreateBuffer(ID3D12Device* dev, std::string textureFilename) {

	HRESULT result;
	
	auto ext = GetExtension(textureFilename);
	auto wTextureFilename = GetWideStringFromString(textureFilename);

	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	if (ext == "sph" || ext == "spa" || ext == "bmp" || ext == "png" || ext == "jpg") {
		result = LoadFromWICFile(wTextureFilename.c_str(), WIC_FLAGS_NONE , &metadata, scratchImg);
	}
	else if (ext == "tga") {
		result = LoadFromTGAFile(wTextureFilename.c_str(), &metadata, scratchImg);
	}
	else if (ext == "dds") {
		result = LoadFromDDSFile(wTextureFilename.c_str(), DDS_FLAGS_NONE, &metadata, scratchImg);
	}
	else {
		result = E_FAIL;
	}
	if (FAILED(result)) {
		return nullptr;
	}
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
	if (FAILED(result)) {
		return nullptr;
	}

	result = texBuffer->WriteToSubresource(0, nullptr, img->pixels, img->rowPitch, img->slicePitch);
	return texBuffer;
}


ID3D12Resource* loadTexture(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, std::string& texPath) {

	//�e�[�u���ɓ��ɂ������烍�[�h����̂ł͂Ȃ��}�b�v���̃��\�[�X��Ԃ�
	auto it = resourceTable.find(texPath);
	if (it != resourceTable.end()) {
		return resourceTable[texPath];
	}

	auto texbuff = loadTextureAndCreateBuffer(dev, texPath);
	resourceTable[texPath] = texbuff;

	return texbuff;
}


ID3D12Resource* loadToonTexture(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, unsigned char toonIdx) {
	char toonFileName[16];
	sprintf_s(toonFileName, 16, "toon%02d.bmp", toonIdx + 1);
	auto toonFilePath = std::string("toon/") + toonFileName;
	return loadTexture(dev, resourceTable, toonFilePath);
}


void loadTextureExceptToon(ID3D12Device* dev, std::map<std::string, ID3D12Resource*>& resourceTable, TextureResources& textureResources, int material_idx, std::string texFileName, std::string modelPath) {

	std::vector<std::string> texFilePathes;
	if (std::count(texFileName.begin(), texFileName.end(), '*') > 0) { //�X�v���b�^������
		auto namepair = SplitFileName(texFileName);
		texFilePathes.push_back(std::string("model/") + namepair.first);
		texFilePathes.push_back(std::string("model/") + namepair.second);
	}
	else {
		texFilePathes.push_back(std::string("model/") + texFileName);
	}

	for (auto path_idx = 0; path_idx < texFilePathes.size(); ++path_idx) {
		auto texFilePath = GetTexturePathFromModelAndTexPath(modelPath, texFilePathes[path_idx].c_str());
		auto ext = GetExtension(texFilePath);
		if (ext == "sph") {
			textureResources.sph[material_idx] = loadTexture(dev, resourceTable, texFilePath);
		}
		else if (ext == "spa") {
			textureResources.spa[material_idx] = loadTexture(dev, resourceTable, texFilePath);
		}
		else {
			textureResources.normalTex[material_idx] = loadTexture(dev, resourceTable, texFilePath);
		}
	}
}


TextureResources createTextureResources(ID3D12Device* dev, std::vector<Material> materials, std::string modelPath) {

	std::map<std::string, ID3D12Resource*> resourceTable;
	TextureResources textureResources;
	textureResources.normalTex.resize(materials.size());
	textureResources.sph.resize(materials.size());
	textureResources.spa.resize(materials.size());
	textureResources.toon.resize(materials.size());

	for (int material_idx = 0; material_idx < materials.size(); ++material_idx) {
		textureResources.toon[material_idx] = loadToonTexture(dev, resourceTable, materials[material_idx].toonIdx);
		if (materials[material_idx].texFilePath.length() == 0) continue;
		loadTextureExceptToon(dev, resourceTable, textureResources, material_idx, materials[material_idx].texFilePath, modelPath);
	}

	return textureResources;
}


ID3D12Resource* createMaterialBuffer(ID3D12Device* dev, UINT64 datasize) {
	// ���_�o�b�t�@�̏����𗬗p
	return createVertexBuffer(dev, datasize);
}


void mapMaterialBuffer(ID3D12Resource* materialBuffer, std::vector<Material> materials) {
	char* mapMaterial = nullptr;
	auto result = materialBuffer->Map(0, nullptr, (void**)&mapMaterial);
	auto materialBuffSize = (sizeof(MaterialForHlsl) + 0xff) & ~0xff;
	for (auto& m : materials) {
		*((MaterialForHlsl*)mapMaterial) = m.materialForHlsl;
		mapMaterial += materialBuffSize;
	}
	materialBuffer->Unmap(0, nullptr);
}


D3D12_RESOURCE_DESC createTexResourceDescriptorForDefaultTexture(UINT64 height) {
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;
	resDesc.Height = height;
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;
	resDesc.MipLevels = 1;
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	return resDesc;
}


ID3D12Resource* CreateMonoTexture(ID3D12Device* dev, int value) {

	auto texHeapProp = createTexHeapProperties();
	auto resDesc = createTexResourceDescriptorForDefaultTexture(4);

	ID3D12Resource* blackBuff = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&blackBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}
	std::vector<unsigned char> data(4 * 4 * 4);
	std::fill(data.begin(), data.end(), value);

	result = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return blackBuff;
}


ID3D12Resource* CreateGrayGradationTexture(ID3D12Device* dev) {

	auto texHeapProp = createTexHeapProperties();
	auto resDesc = createTexResourceDescriptorForDefaultTexture(256);

	ID3D12Resource* gradBuff = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&gradBuff)
	);
	if (FAILED(result)) {
		return nullptr;
	}

	//�オ�����ĉ��������e�N�X�`���f�[�^���쐬
	std::vector<unsigned int> data(4 * 256);
	auto it = data.begin();
	unsigned int c = 0xff;
	for (; it != data.end(); it += 4) {
		auto col = (0xff << 24) | RGB(c, c, c);//RGBA���t���т��Ă��邽��RGB�}�N����0xff<<24��p���ĕ\���B
		//auto col = (0xff << 24) | (c<<16)|(c<<8)|c;//����ł�OK
		std::fill(it, it + 4, col);
		--c;
	}

	result = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int) * static_cast<UINT>(data.size()));
	return gradBuff;
}


void createMaterialBufferView(ID3D12Device* dev, ID3D12Resource* materialBuffer, ID3D12DescriptorHeap* descriptorHeap, TextureResources textureResources, UINT64 materialNum) {

	auto whiteTex = CreateMonoTexture(dev, 255);
	auto blackTex = CreateMonoTexture(dev, 0);
	auto gradTex = CreateGrayGradationTexture(dev);

	auto materialBuffSize = (sizeof(MaterialForHlsl) + 0xff) & ~0xff;

	// �}�e���A���p�̃R���X�^���g�o�b�t�@�r���[�f�B�X�N���v�^
	// �����̃}�e���A���ɑ΂��āABufferLocation�����������Ȃ���ė��p����
	D3D12_CONSTANT_BUFFER_VIEW_DESC materialConstBufferViewDesc = {};
	materialConstBufferViewDesc.BufferLocation = materialBuffer->GetGPUVirtualAddress();
	materialConstBufferViewDesc.SizeInBytes = static_cast<UINT>(materialBuffSize);

	// �V�F�[�_�[���\�[�X�r���[�f�B�X�N���v�^
	// �����̃e�N�X�`���t�H�[�}�b�g�ɑ΂��āAFormat�����������Ȃ���ė��p����
	D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	auto materialDescriptorHeapHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (size_t i = 0; i < materialNum; ++i) {

		// �}�e���A���萔
		dev->CreateConstantBufferView(&materialConstBufferViewDesc, materialDescriptorHeapHandle);
		materialDescriptorHeapHandle.ptr += incSize;
		materialConstBufferViewDesc.BufferLocation += materialBuffSize;

		// ��ʃe�N�X�`��
		if (textureResources.normalTex[i] == nullptr) {
			shaderResourceViewDesc.Format = whiteTex->GetDesc().Format;
			dev->CreateShaderResourceView(whiteTex, &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		else {
			shaderResourceViewDesc.Format = textureResources.normalTex[i]->GetDesc().Format;
			dev->CreateShaderResourceView(textureResources.normalTex[i], &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		materialDescriptorHeapHandle.ptr += incSize;

		// SPH
		if (textureResources.sph[i] == nullptr) {
			shaderResourceViewDesc.Format = whiteTex->GetDesc().Format;
			dev->CreateShaderResourceView(whiteTex, &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		else {
			shaderResourceViewDesc.Format = textureResources.sph[i]->GetDesc().Format;
			dev->CreateShaderResourceView(textureResources.sph[i], &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		materialDescriptorHeapHandle.ptr += incSize;

		// SPA
		if (textureResources.spa[i] == nullptr) {
			shaderResourceViewDesc.Format = blackTex->GetDesc().Format;
			dev->CreateShaderResourceView(blackTex, &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		else {
			shaderResourceViewDesc.Format = textureResources.spa[i]->GetDesc().Format;
			dev->CreateShaderResourceView(textureResources.spa[i], &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		materialDescriptorHeapHandle.ptr += incSize;

		// �g�D�[��
		if (textureResources.toon[i] == nullptr) {
			shaderResourceViewDesc.Format = gradTex->GetDesc().Format;
			dev->CreateShaderResourceView(gradTex, &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		else {
			shaderResourceViewDesc.Format = textureResources.toon[i]->GetDesc().Format;
			dev->CreateShaderResourceView(textureResources.toon[i], &shaderResourceViewDesc, materialDescriptorHeapHandle);
		}
		materialDescriptorHeapHandle.ptr += incSize;
	}
}


ID3D12RootSignature* createRootSignature(ID3D12Device* dev) {

	// �f�B�X�N���v�^�e�[�u�������W�i�����̃f�B�X�N���v�^���܂Ƃ߂Ďg�p�ł���悤�ɂ��邽�߂̎d�g�݁j
	D3D12_DESCRIPTOR_RANGE descTableRange[3] = {};

	// �}�g���N�X�萔
	descTableRange[0].NumDescriptors = 1;
	descTableRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTableRange[0].BaseShaderRegister = 0;
	descTableRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// �}�e���A���萔
	descTableRange[1].NumDescriptors = 1;
	descTableRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	descTableRange[1].BaseShaderRegister = 1;
	descTableRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// �e�N�X�`��
	descTableRange[2].NumDescriptors = 4; // ��{, sph, spa, �g�D�[��
	descTableRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descTableRange[2].BaseShaderRegister = 0;
	descTableRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ���[�g�p�����[�^�[�i�f�B�X�N���v�^�e�[�u���̎��́B�f�B�X�N���v�^�e�[�u���̓e�N�X�`���Ȃǂ�CPU/GPU�ŋ��ʔF�����邽�߂̎d�g�݁j
	D3D12_ROOT_PARAMETER rootParam[2] = {};
	rootParam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[0].DescriptorTable.pDescriptorRanges = &descTableRange[0];
	rootParam[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// �}�e���A���萔�ƃe�N�X�`���̓s�N�Z���V�F�[�_�[���猩����΂悢
	rootParam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParam[1].DescriptorTable.pDescriptorRanges = &descTableRange[1];
	rootParam[1].DescriptorTable.NumDescriptorRanges = 2;
	rootParam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// �T���v���[�iuv�l�ɂ���ăe�N�X�`���f�[�^����ǂ��F�����o���������߂邽�߂̐ݒ�j
	D3D12_STATIC_SAMPLER_DESC samplerDesc[2] = {};
	samplerDesc[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���J��Ԃ�
	samplerDesc[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//�c�J��Ԃ�
	samplerDesc[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;//���s�J��Ԃ�
	samplerDesc[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;//�{�[�_�[�̎��͍�
	samplerDesc[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;//��Ԃ��Ȃ�(�j�A���X�g�l�C�o�[)
	samplerDesc[0].MaxLOD = D3D12_FLOAT32_MAX;//�~�b�v�}�b�v�ő�l
	samplerDesc[0].MinLOD = 0.0f;//�~�b�v�}�b�v�ŏ��l
	samplerDesc[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;//�I�[�o�[�T���v�����O�̍ۃ��T���v�����O���Ȃ��H
	samplerDesc[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_����̂݉�
	samplerDesc[0].ShaderRegister = 0;
	samplerDesc[1] = samplerDesc[0];//�ύX�_�ȊO���R�s�[
	samplerDesc[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;//
	samplerDesc[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc[1].ShaderRegister = 1;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rootSignatureDesc.pParameters = rootParam;
	rootSignatureDesc.NumParameters = 2;
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;

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


void render(ID3D12Device* dev, ID3D12DescriptorHeap* rtvDescriptorHeap, ID3D12GraphicsCommandList* commandList, D3D12_VERTEX_BUFFER_VIEW vertexBufferView, D3D12_INDEX_BUFFER_VIEW indexBufferView,
	IDXGISwapChain4* swapChain, ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState, D3D12_VIEWPORT viewport, D3D12_RECT scissorRect, ID3D12DescriptorHeap* basicDescHeap,
	ID3D12DescriptorHeap* materialDescHeap, ID3D12DescriptorHeap* depthDescriptorHeap, std::vector<Material> materials)
{
	// �o���A��ݒ�
	ID3D12Resource* backBuffer;
	auto bufferIdx = swapChain->GetCurrentBackBufferIndex();
	auto result = swapChain->GetBuffer(bufferIdx, IID_PPV_ARGS(&backBuffer));
	auto resourceBarrier = createResourceBarrier(backBuffer);

	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	commandList->ResourceBarrier(1, &resourceBarrier);

	// �p�C�v���C���X�e�[�g���Z�b�g
	commandList->SetPipelineState(pipelineState);

	// ���ꂩ��g�������_�[�^�[�Q�b�g�r���[�Ƃ���rtvHandle���Z�b�g
	auto rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += bufferIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// �f�v�X�o�b�t�@�̐ݒ�iChapter07�Œǉ��j
	auto depthHandle = depthDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	commandList->OMSetRenderTargets(1, &rtvHandle, true, &depthHandle);

	// �N���A
	float clearColor[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(depthHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);  // �f�v�X�o�b�t�@�̃N���A

	// �����_�����O�ݒ�iChapter04�܂Łj
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetIndexBuffer(&indexBufferView);

	// �����_�����O�ݒ�iChapter05�Œǉ��j
	commandList->SetDescriptorHeaps(1, &basicDescHeap);
	commandList->SetGraphicsRootDescriptorTable(0, basicDescHeap->GetGPUDescriptorHandleForHeapStart());

	//�}�e���A��
	commandList->SetDescriptorHeaps(1, &materialDescHeap);

	auto materialDescHeapHandle = materialDescHeap->GetGPUDescriptorHandleForHeapStart();
	unsigned int idxOffset = 0;

	auto cbvsrvIncSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * 5;
	for (auto& m : materials) {
		commandList->SetGraphicsRootDescriptorTable(1, materialDescHeapHandle);
		commandList->DrawIndexedInstanced(m.indicesNum, 1, idxOffset, 0, 0);
		materialDescHeapHandle.ptr += cbvsrvIncSize;
		idxOffset += m.indicesNum;
	}

	// �o���A�ɂ�銮���҂�
	resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &resourceBarrier);
}