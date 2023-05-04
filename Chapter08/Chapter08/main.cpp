//�R���X�^���g�o�b�t�@�ōs���]��
#include<Windows.h>
#include<tchar.h>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<DirectXMath.h>
#include<vector>
#include<map>
#include<d3dcompiler.h>
#include<DirectXTex.h>
#include<d3dx12.h>
#include<dxgidebug.h>
#include "chapter03.h"
#include "chapter04.h"
#include "chapter05.h"
#include "chapter06.h"
#include "chapter07.h"


#ifdef _DEBUG
#include<iostream>
#endif

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


///���f���̃p�X�ƃe�N�X�`���̃p�X���獇���p�X�𓾂�
///@param modelPath �A�v���P�[�V�������猩��pmd���f���̃p�X
///@param texPath PMD���f�����猩���e�N�X�`���̃p�X
///@return �A�v���P�[�V�������猩���e�N�X�`���̃p�X
std::string GetTexturePathFromModelAndTexPath(const std::string& modelPath, const char* texPath) {
	//�t�@�C���̃t�H���_��؂��\��/�̓��ނ��g�p�����\��������
	//�Ƃ�����������\��/�𓾂���΂����̂ŁA�o����rfind���Ƃ��r����
	//int�^�ɑ�����Ă���̂͌�����Ȃ������ꍇ��rfind��epos(-1��0xffffffff)��Ԃ�����
	auto pathIndex1 = modelPath.rfind('/');
	auto pathIndex2 = modelPath.rfind('\\');
	auto pathIndex = max(pathIndex1, pathIndex2);
	auto folderPath = modelPath.substr(0, pathIndex+1);
	return folderPath + texPath;
}

///�t�@�C��������g���q���擾����
///@param path �Ώۂ̃p�X������
///@return �g���q
string
GetExtension(const std::string& path) {
	auto idx = path.rfind('.');
	return path.substr(idx+1, path.length() - idx-1);
}

///�t�@�C��������g���q���擾����(���C�h������)
///@param path �Ώۂ̃p�X������
///@return �g���q
wstring
GetExtension(const std::wstring& path) {
	auto idx = path.rfind(L'.');
	return path.substr(idx + 1, path.length() - idx - 1);
}

///�e�N�X�`���̃p�X���Z�p���[�^�����ŕ�������
///@param path �Ώۂ̃p�X������
///@param splitter ��؂蕶��
///@return �����O��̕�����y�A
pair<string,string> 
SplitFileName(const std::string& path, const char splitter='*') {
	auto idx = path.find(splitter);
	pair<string, string> ret;
	ret.first = path.substr(0, idx);
	ret.second = path.substr(idx+1, path.length()-idx-1);
	return ret;
}

///string(�}���`�o�C�g������)����wstring(���C�h������)�𓾂�
///@param str �}���`�o�C�g������
///@return �ϊ����ꂽ���C�h������
std::wstring
GetWideStringFromString(const std::string& str) {
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

//�g�D�[���̂��߂̃O���f�[�V�����e�N�X�`��
ID3D12Resource*
CreateGrayGradationTexture() {
	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//��
	resDesc.Height = 256;//����
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

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
	unsigned int c=0xff;
	for (; it != data.end();it+=4) {
		auto col = (0xff << 24) | RGB(c,c,c);//RGBA���t���т��Ă��邽��RGB�}�N����0xff<<24��p���ĕ\���B
		//auto col = (0xff << 24) | (c<<16)|(c<<8)|c;//����ł�OK
		std::fill(it, it+4, col);
		--c;
	}

	result = gradBuff->WriteToSubresource(0, nullptr, data.data(), 4 * sizeof(unsigned int), sizeof(unsigned int)*static_cast<UINT>(data.size()));
	return gradBuff;
}

ID3D12Resource*
CreateWhiteTexture() {
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//��
	resDesc.Height = 4;//����
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	ID3D12Resource* whiteBuff = nullptr;
	auto result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
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
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	resDesc.Width = 4;//��
	resDesc.Height = 4;//����
	resDesc.DepthOrArraySize = 1;
	resDesc.SampleDesc.Count = 1;
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = 1;//
	resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

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
	std::fill(data.begin(), data.end(), 0x00);

	result = blackBuff->WriteToSubresource(0, nullptr, data.data(), 4 * 4, static_cast<UINT>(data.size()));
	return blackBuff;
}
using LoadLambda_t = function<HRESULT(const wstring& path, TexMetadata*, ScratchImage&)>;
map < string, LoadLambda_t> loadLambdaTable;


//�t�@�C�����p�X�ƃ��\�[�X�̃}�b�v�e�[�u��
map<string, ID3D12Resource*> _resourceTable;

ID3D12Resource*
LoadTextureFromFile(std::string& texPath ) {
	auto it=_resourceTable.find(texPath);
	if (it != _resourceTable.end()) {
		//�e�[�u���ɓ��ɂ������烍�[�h����̂ł͂Ȃ��}�b�v����
		//���\�[�X��Ԃ�
		return _resourceTable[texPath];
	}
	

	//WIC�e�N�X�`���̃��[�h
	TexMetadata metadata = {};
	ScratchImage scratchImg = {};
	auto wtexpath = GetWideStringFromString(texPath);//�e�N�X�`���̃t�@�C���p�X
	auto ext = GetExtension(texPath);//�g���q���擾
	auto result = loadLambdaTable[ext](wtexpath,
						&metadata, 
						scratchImg);
	if (FAILED(result)) {
		return nullptr;
	}
	auto img = scratchImg.GetImage(0, 0, 0);//���f�[�^���o

	//WriteToSubresource�œ]������p�̃q�[�v�ݒ�
	D3D12_HEAP_PROPERTIES texHeapProp = {};
	texHeapProp.Type = D3D12_HEAP_TYPE_CUSTOM;//����Ȑݒ�Ȃ̂�default�ł�upload�ł��Ȃ�
	texHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;//���C�g�o�b�N��
	texHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;//�]����L0�܂�CPU�����璼��
	texHeapProp.CreationNodeMask = 0;//�P��A�_�v�^�̂���0
	texHeapProp.VisibleNodeMask = 0;//�P��A�_�v�^�̂���0

	D3D12_RESOURCE_DESC resDesc = {};
	resDesc.Format = metadata.format;
	resDesc.Width = static_cast<UINT>(metadata.width);//��
	resDesc.Height = static_cast<UINT>(metadata.height);//����
	resDesc.DepthOrArraySize = static_cast<UINT16>(metadata.arraySize);
	resDesc.SampleDesc.Count = 1;//�ʏ�e�N�X�`���Ȃ̂ŃA���`�F�����Ȃ�
	resDesc.SampleDesc.Quality = 0;//
	resDesc.MipLevels = static_cast<UINT16>(metadata.mipLevels);//�~�b�v�}�b�v���Ȃ��̂Ń~�b�v���͂P��
	resDesc.Dimension = static_cast<D3D12_RESOURCE_DIMENSION>(metadata.dimension);
	resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;//���C�A�E�g�ɂ��Ă͌��肵�Ȃ�
	resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;//�Ƃ��Ƀt���O�Ȃ�

	ID3D12Resource* texbuff = nullptr;
	result = dev->CreateCommittedResource(
		&texHeapProp,
		D3D12_HEAP_FLAG_NONE,//���Ɏw��Ȃ�
		&resDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&texbuff)
	);
	
	if (FAILED(result)) {
		return nullptr;
	}
	result = texbuff->WriteToSubresource(0,
		nullptr,//�S�̈�փR�s�[
		img->pixels,//���f�[�^�A�h���X
		static_cast<UINT>(img->rowPitch),//1���C���T�C�Y
		static_cast<UINT>(img->slicePitch)//�S�T�C�Y
	);
	if (FAILED(result)) {
		return nullptr;
	}

	_resourceTable[texPath] = texbuff;
	return texbuff;
}

///�f�o�b�O���C���[��L���ɂ���
void EnableDebugLayer() {
	ID3D12Debug* debugLayer = nullptr;
	auto result = D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
	debugLayer->Release();
	
}


#ifdef _DEBUG
int main() {
#else
#include<Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
#endif

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


	loadLambdaTable["sph"] = loadLambdaTable["spa"] = loadLambdaTable["bmp"] = loadLambdaTable["png"] = loadLambdaTable["jpg"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, meta, img);
	};

	loadLambdaTable["tga"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromTGAFile(path.c_str(), meta, img);
	};

	loadLambdaTable["dds"] = [](const wstring& path, TexMetadata* meta, ScratchImage& img)->HRESULT {
		return LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, meta, img);
	};



	// �f�v�X�o�b�t�@
	auto depthBuffer = createDepthBuffer(dev, windowWidth, windowHeight);
	auto depthDescriptorHeap = createDepthDescriptorHeap(dev, depthBuffer);
	createDepthBufferView(dev, depthBuffer, depthDescriptorHeap);



	ID3D12Fence* _fence = nullptr;
	UINT64 _fenceVal = 0;
	auto result = dev->CreateFence(_fenceVal, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));

	ShowWindow(hwnd, SW_SHOW);//�E�B���h�E�\��

	auto whiteTex = CreateWhiteTexture();
	auto blackTex = CreateBlackTexture();
	auto gradTex = CreateGrayGradationTexture();

	//PMD�w�b�_�\����
	struct PMDHeader {
		float version; //��F00 00 80 3F == 1.00
		char model_name[20];//���f����
		char comment[256];//���f���R�����g
	};
	char signature[3];
	PMDHeader pmdheader = {};
	//string strModelPath = "Model/hibiki/hibiki.pmd";
	//string strModelPath = "Model/satori/satori.pmd";
	//string strModelPath = "Model/reimu/reimu.pmd";
	string strModelPath = "Model/�������J.pmd";
	//string strModelPath = "Model/�����~�N.pmd";
	FILE* fp;
	fopen_s(&fp,strModelPath.c_str(), "rb");
	fread(signature, sizeof(signature), 1, fp);
	fread(&pmdheader, sizeof(pmdheader), 1, fp);

	unsigned int vertNum;//���_��
	fread(&vertNum, sizeof(vertNum), 1, fp);


#pragma pack(1)//��������1�o�C�g�p�b�L���O�c�A���C�����g�͔������Ȃ�
	//PMD�}�e���A���\����
	struct PMDMaterial {
		XMFLOAT3 diffuse; //�f�B�t���[�Y�F
		float alpha; // �f�B�t���[�Y��
		float specularity;//�X�y�L�����̋���(��Z�l)
		XMFLOAT3 specular; //�X�y�L�����F
		XMFLOAT3 ambient; //�A���r�G���g�F
		unsigned char toonIdx; //�g�D�[���ԍ�(��q)
		unsigned char edgeFlg;//�}�e���A�����̗֊s���t���O
		//2�o�C�g�̃p�f�B���O�������I�I
		unsigned int indicesNum; //���̃}�e���A�������蓖����C���f�b�N�X��
		char texFilePath[20]; //�e�N�X�`���t�@�C����(�v���X�A���t�@�c��q)
	};//70�o�C�g�̂͂��c�ł��p�f�B���O���������邽��72�o�C�g
#pragma pack()//1�o�C�g�p�b�L���O����

	//�V�F�[�_���ɓ�������}�e���A���f�[�^
	struct MaterialForHlsl{
		XMFLOAT3 diffuse; //�f�B�t���[�Y�F
		float alpha; // �f�B�t���[�Y��
		XMFLOAT3 specular; //�X�y�L�����F
		float specularity;//�X�y�L�����̋���(��Z�l)
		XMFLOAT3 ambient; //�A���r�G���g�F
	};
	//����ȊO�̃}�e���A���f�[�^
	struct AdditionalMaterial {
		std::string texPath;//�e�N�X�`���t�@�C���p�X
		int toonIdx; //�g�D�[���ԍ�
		bool edgeFlg;//�}�e���A�����̗֊s���t���O
	};
	//�܂Ƃ߂�����
	struct Material {
		unsigned int indicesNum;//�C���f�b�N�X��
		MaterialForHlsl material;
		AdditionalMaterial additional;
	};

	constexpr unsigned int pmdvertex_size = 38;//���_1������̃T�C�Y
	std::vector<PMD_VERTEX> vertices(vertNum);//�o�b�t�@�m��
	for (auto i = 0; i < vertNum; i++)
	{
		fread(&vertices[i], pmdvertex_size, 1, fp);
	}

	unsigned int indicesNum;//�C���f�b�N�X��
	fread(&indicesNum, sizeof(indicesNum), 1, fp);//

	std::vector<unsigned short> indices(indicesNum);
	fread(indices.data(), indices.size() * sizeof(indices[0]), 1, fp);//��C�ɓǂݍ���



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
	// auto rootSignature = createRootSignature(dev);
	// auto inputLayout = createInputLayout();
	// auto pipelineState = createGraphicsPipelineState(dev, vertexShaderBlob, pixelShaderBlob, rootSignature, inputLayout);
	auto viewport = createViewPort(windowWidth, windowHeight);
	auto scissorRect = createScissorRect(windowWidth, windowHeight);


	unsigned int materialNum;//�}�e���A����
	fread(&materialNum, sizeof(materialNum), 1, fp);
	std::vector<Material> materials(materialNum);

	vector<ID3D12Resource*> textureResources(materialNum);
	vector<ID3D12Resource*> sphResources(materialNum);
	vector<ID3D12Resource*> spaResources(materialNum);
	vector<ID3D12Resource*> toonResources(materialNum);
	{
		std::vector<PMDMaterial> pmdMaterials(materialNum);
		fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
		//�R�s�[
		for (int i = 0; i < pmdMaterials.size(); ++i) {
			materials[i].indicesNum = pmdMaterials[i].indicesNum;
			materials[i].material.diffuse = pmdMaterials[i].diffuse;
			materials[i].material.alpha = pmdMaterials[i].alpha;
			materials[i].material.specular = pmdMaterials[i].specular;
			materials[i].material.specularity = pmdMaterials[i].specularity;
			materials[i].material.ambient = pmdMaterials[i].ambient;
			materials[i].additional.toonIdx = pmdMaterials[i].toonIdx;
		}

		for (int i = 0; i < pmdMaterials.size(); ++i) {
			//�g�D�[�����\�[�X�̓ǂݍ���
			string toonFilePath = "toon/";
			char toonFileName[16];
			sprintf_s(toonFileName, 16,"toon%02d.bmp", pmdMaterials[i].toonIdx + 1);
			toonFilePath += toonFileName;
			toonResources[i] = LoadTextureFromFile(toonFilePath);

			if (strlen(pmdMaterials[i].texFilePath) == 0) {
				textureResources[i] = nullptr;
				continue;
			}

			string texFileName= pmdMaterials[i].texFilePath;
			string sphFileName="";
			string spaFileName="";
			if (count(texFileName.begin(), texFileName.end(), '*') > 0) {//�X�v���b�^������
				auto namepair=SplitFileName(texFileName);
				if (GetExtension(namepair.first) == "sph") {
					texFileName = string("model/") + namepair.second;
					sphFileName = string("model/") + namepair.first;
				}
				else if (GetExtension(namepair.first) == "spa") {
					texFileName = string("model/") + namepair.second;
					spaFileName = string("model/") + namepair.first;
				}
				else {
					texFileName = namepair.first;
					if (GetExtension(namepair.second) == "sph") {
						sphFileName = string("model/") + namepair.second;
					}
					else if (GetExtension(namepair.second) == "spa") {
						spaFileName = string("model/") + namepair.second;
					}
				}
			}
			else {
				if (GetExtension(pmdMaterials[i].texFilePath) == "sph") {
					sphFileName= string("model/") + pmdMaterials[i].texFilePath;
					texFileName = "";
				}else if (GetExtension(pmdMaterials[i].texFilePath) == "spa") {
					spaFileName = string("model/") + pmdMaterials[i].texFilePath;
					texFileName = "";
				}
				else {
					texFileName = string("model/") + pmdMaterials[i].texFilePath;
				}
			}
			//���f���ƃe�N�X�`���p�X����A�v���P�[�V��������̃e�N�X�`���p�X�𓾂�
			if (texFileName != "") {
				auto texFilePath = GetTexturePathFromModelAndTexPath(strModelPath, texFileName.c_str());
				textureResources[i] = LoadTextureFromFile(texFilePath);
			}
			if (sphFileName != "") {
				auto sphFilePath = GetTexturePathFromModelAndTexPath(strModelPath, sphFileName.c_str());
				sphResources[i] = LoadTextureFromFile(sphFilePath);
			}
			if (spaFileName != "") {
				auto spaFilePath = GetTexturePathFromModelAndTexPath(strModelPath, spaFileName.c_str());
				spaResources[i] = LoadTextureFromFile(spaFilePath);
			}
			

		}

	}
	fclose(fp);

	//�}�e���A���o�b�t�@���쐬
	auto materialBuffSize = sizeof(MaterialForHlsl);
	materialBuffSize = (materialBuffSize + 0xff)&~0xff;
	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(materialBuffSize * materialNum);//�ܑ̂Ȃ����ǎd���Ȃ��ł���
	ID3D12Resource* materialBuff = nullptr;
	result = dev->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&materialBuff)
	);

	//�}�b�v�}�e���A���ɃR�s�[
	char* mapMaterial = nullptr;
	result = materialBuff->Map(0, nullptr, (void**)&mapMaterial);
	for (auto& m : materials) {
		*((MaterialForHlsl*)mapMaterial) = m.material;//�f�[�^�R�s�[
		mapMaterial += materialBuffSize;//���̃A���C�����g�ʒu�܂Ői�߂�
	}
	materialBuff->Unmap(0,nullptr);


	ID3D12DescriptorHeap* materialDescHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC materialDescHeapDesc = {};
	materialDescHeapDesc.NumDescriptors = materialNum * 5;//�}�e���A�����Ԃ�(�萔1�A�e�N�X�`��3��)
	materialDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	materialDescHeapDesc.NodeMask = 0;
	
	materialDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;//�f�X�N���v�^�q�[�v���
	result = dev->CreateDescriptorHeap(&materialDescHeapDesc, IID_PPV_ARGS(&materialDescHeap));//����

	D3D12_CONSTANT_BUFFER_VIEW_DESC matCBVDesc = {};
	matCBVDesc.BufferLocation = materialBuff->GetGPUVirtualAddress();
	matCBVDesc.SizeInBytes = static_cast<UINT>(materialBuffSize);

	////�ʏ�e�N�X�`���r���[�쐬
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;//��q
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;//2D�e�N�X�`��
	srvDesc.Texture2D.MipLevels = 1;//�~�b�v�}�b�v�͎g�p���Ȃ��̂�1

	auto matDescHeapH = materialDescHeap->GetCPUDescriptorHandleForHeapStart();
	auto incSize= dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	for (size_t i = 0; i < materialNum; ++i) {
		//�}�e���A���Œ�o�b�t�@�r���[
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

	D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
		{ "POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "BONE_NO",0,DXGI_FORMAT_R16G16_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		{ "WEIGHT",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
		//{ "EDGE_FLG",0,DXGI_FORMAT_R8_UINT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 },
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeline = {};
	gpipeline.pRootSignature = nullptr;

	gpipeline.VS.pShaderBytecode = vertexShaderBlob->GetBufferPointer();
	gpipeline.VS.BytecodeLength = vertexShaderBlob->GetBufferSize();
	gpipeline.PS.pShaderBytecode = pixelShaderBlob->GetBufferPointer();
	gpipeline.PS.BytecodeLength = pixelShaderBlob->GetBufferSize();

	gpipeline.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;//���g��0xffffffff

	//
	gpipeline.BlendState.AlphaToCoverageEnable = false;
	gpipeline.BlendState.IndependentBlendEnable = false;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};

	//�ЂƂ܂����Z���Z�⃿�u�����f�B���O�͎g�p���Ȃ�
	renderTargetBlendDesc.BlendEnable = false;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


	//�ЂƂ܂��_�����Z�͎g�p���Ȃ�
	renderTargetBlendDesc.LogicOpEnable = false;
	
	gpipeline.BlendState.RenderTarget[0] = renderTargetBlendDesc;
	
	gpipeline.RasterizerState.MultisampleEnable = false;//�܂��A���`�F���͎g��Ȃ�
	gpipeline.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;//�J�����O���Ȃ�
	gpipeline.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;//���g��h��Ԃ�
	gpipeline.RasterizerState.DepthClipEnable = true;//�[�x�����̃N���b�s���O�͗L����

	gpipeline.RasterizerState.FrontCounterClockwise = false;
	gpipeline.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	gpipeline.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	gpipeline.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	gpipeline.RasterizerState.AntialiasedLineEnable = false;
	gpipeline.RasterizerState.ForcedSampleCount = 0;
	gpipeline.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


	gpipeline.DepthStencilState.DepthEnable = true;//�[�x�o�b�t�@���g����
	gpipeline.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;//�S�ď�������
	gpipeline.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;//�����������̗p
	gpipeline.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpipeline.DepthStencilState.StencilEnable = false;

	gpipeline.InputLayout.pInputElementDescs = inputLayout;//���C�A�E�g�擪�A�h���X
	gpipeline.InputLayout.NumElements = _countof(inputLayout);//���C�A�E�g�z��

	gpipeline.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;//�X�g���b�v���̃J�b�g�Ȃ�
	gpipeline.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;//�O�p�`�ō\��

	gpipeline.NumRenderTargets = 1;//���͂P�̂�
	gpipeline.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;//0�`1�ɐ��K�����ꂽRGBA

	gpipeline.SampleDesc.Count = 1;//�T���v�����O��1�s�N�Z���ɂ��P
	gpipeline.SampleDesc.Quality = 0;//�N�I���e�B�͍Œ�

	ID3D12RootSignature* rootsignature = nullptr;
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descTblRange[3] = {};//�e�N�X�`���ƒ萔�̂Q��


	//�萔�ЂƂ�(���W�ϊ��p)
	descTblRange[0].NumDescriptors = 1;//�萔�ЂƂ�
	descTblRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//��ʂ͒萔
	descTblRange[0].BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�萔�ӂ���(�}�e���A���p)
	descTblRange[1].NumDescriptors = 1;//�f�X�N���v�^�q�[�v�͂������񂠂邪��x�Ɏg���̂͂P��
	descTblRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;//��ʂ͒萔
	descTblRange[1].BaseShaderRegister = 1;//1�ԃX���b�g����
	descTblRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	//�e�N�X�`��1��(���̃}�e���A���ƃy�A)
	descTblRange[2].NumDescriptors = 4;//�e�N�X�`���S��(��{��sph��spa�ƃg�D�[��)
	descTblRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;//��ʂ̓e�N�X�`��
	descTblRange[2].BaseShaderRegister = 0;//0�ԃX���b�g����
	descTblRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootparam[2] = {};
	rootparam[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[0].DescriptorTable.pDescriptorRanges = &descTblRange[0];//�f�X�N���v�^�����W�̃A�h���X
	rootparam[0].DescriptorTable.NumDescriptorRanges = 1;//�f�X�N���v�^�����W��
	rootparam[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;//�S�ẴV�F�[�_���猩����

	rootparam[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootparam[1].DescriptorTable.pDescriptorRanges = &descTblRange[1];//�f�X�N���v�^�����W�̃A�h���X
	rootparam[1].DescriptorTable.NumDescriptorRanges = 2;//�f�X�N���v�^�����W��������
	rootparam[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;//�s�N�Z���V�F�[�_���猩����

	rootSignatureDesc.pParameters = rootparam;//���[�g�p�����[�^�̐擪�A�h���X
	rootSignatureDesc.NumParameters = 2;//���[�g�p�����[�^��

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
	rootSignatureDesc.pStaticSamplers = samplerDesc;
	rootSignatureDesc.NumStaticSamplers = 2;
	
	ID3DBlob* rootSigBlob = nullptr;
	result = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &rootSigBlob, &errorBlob);
	result = dev->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(&rootsignature));
	rootSigBlob->Release();

	gpipeline.pRootSignature = rootsignature;
	ID3D12PipelineState* _pipelinestate = nullptr;
	result = dev->CreateGraphicsPipelineState(&gpipeline, IID_PPV_ARGS(&_pipelinestate));
	


	// Chapter05, 06
	auto basicDescriptorHeap = createBasicDescriptorHeap(dev, 2);
	auto constBuffer = createConstBuffer(dev);
	createConstantBufferView(dev, constBuffer, basicDescriptorHeap, 0);



	//�V�F�[�_���ɓn�����߂̊�{�I�Ȋ��f�[�^
	struct SceneData {
		XMMATRIX world;//���[���h�s��
		XMMATRIX view;//�r���[�v���W�F�N�V�����s��
		XMMATRIX proj;//
		XMFLOAT3 eye;//���_���W
	};

	//�萔�o�b�t�@�쐬
	XMMATRIX worldMat = XMMatrixIdentity();
	XMFLOAT3 eye(0, 15, -15);
	XMFLOAT3 target(0, 15, 0);
	XMFLOAT3 up(0, 1, 0);
	auto viewMat = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
	auto projMat = XMMatrixPerspectiveFovLH(XM_PIDIV4,//��p��45��
		static_cast<float>(windowWidth) / static_cast<float>(windowHeight),//�A�X��
		1.0f,//�߂���
		100.0f//������
	);

	SceneData* mapScene=nullptr;//�}�b�v��������|�C���^
	result = constBuffer->Map(0,nullptr,(void**)&mapScene);//�}�b�v
	//�s��̓��e���R�s�[
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
		//�����A�v���P�[�V�������I�����Ď���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT) {
			break;
		}

		//DirectX����
		//�o�b�N�o�b�t�@�̃C���f�b�N�X���擾
		auto bbIdx = swapChain->GetCurrentBackBufferIndex();

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(backBuffers[bbIdx],
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		commandList->ResourceBarrier(1, &barrier);

		commandList->SetPipelineState(_pipelinestate);


		//�����_�[�^�[�Q�b�g���w��
		auto rtvH = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		rtvH.ptr += static_cast<ULONG_PTR>(bbIdx * dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
		
		commandList->OMSetRenderTargets(1, &rtvH, false, &dsvH);
		commandList->ClearDepthStencilView(dsvH, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		//��ʃN���A

		float clearColor[] = { 1.0f,1.0f,1.0f,1.0f };//���F
		commandList->ClearRenderTargetView(rtvH, clearColor, 0, nullptr);

		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);
		
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);

		commandList->SetGraphicsRootSignature(rootsignature);
		
		//WVP�ϊ��s��
		commandList->SetDescriptorHeaps(1, &basicDescriptorHeap);
		commandList->SetGraphicsRootDescriptorTable(0, basicDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		//�}�e���A��
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

		//���߂̃N���[�Y
		commandList->Close();



		//�R�}���h���X�g�̎��s
		ID3D12CommandList* cmdlists[] = { commandList };
		commandQueue->ExecuteCommandLists(1, cmdlists);
		////�҂�
		++_fenceVal;
		commandQueue->Signal(_fence,_fenceVal);

		if (_fence->GetCompletedValue() != _fenceVal) {
			auto event = CreateEvent(nullptr, false, false, nullptr);
			_fence->SetEventOnCompletion(_fenceVal, event);
			WaitForSingleObjectEx(event, INFINITE,false);
			CloseHandle(event);
		}


		//�t���b�v
		swapChain->Present(0, 0);
		commandAllocator->Reset();//�L���[���N���A
		commandList->Reset(commandAllocator, _pipelinestate);//�ĂуR�}���h���X�g�����߂鏀��

	}
	//�����N���X�g��񂩂�o�^�������Ă�
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
	return 0;
}