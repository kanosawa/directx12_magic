#include <functional>
#include <map>
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


std::vector<Material> copyMaterials(std::vector<PMDMaterial> pmdMaterials) {
	std::vector<Material> materials(pmdMaterials.size());
	for (int i = 0; i < pmdMaterials.size(); ++i) {
		materials[i].indicesNum = pmdMaterials[i].indicesNum;
		materials[i].materialForHlsl.diffuse = pmdMaterials[i].diffuse;
		materials[i].materialForHlsl.alpha = pmdMaterials[i].alpha;
		materials[i].materialForHlsl.specular = pmdMaterials[i].specular;
		materials[i].materialForHlsl.specularity = pmdMaterials[i].specularity;
		materials[i].materialForHlsl.ambient = pmdMaterials[i].ambient;
	}
	return materials;
}


ID3D12Resource* loadTextureAndCreateBuffer(ID3D12Device* dev, std::string textureFilename) {

	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	
	auto ext = GetExtension(textureFilename);
	auto wTextureFilename = GetWideStringFromString(textureFilename);
	HRESULT result;
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