#include "sprite/Sprite.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "System.h"
#include "texture/TextureManager.h"
#include "logger/Logger.h"
#include <directX12/ShaderCompiler.h>

std::array<PipelineStateObj *,kBlendNum> Sprite::pso_;
std::unique_ptr<DxCommand> Sprite::dxCommand_;
Matrix4x4 Sprite::viewPortMat_;

BlendMode Sprite::currentBlend_ = BlendMode::Alpha;

void Sprite::Init(){
	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(System::getInstance()->getDxDevice()->getDevice(),"main","main");
	CreatePSO();
	WinApp *window = System::getInstance()->getWinApp();
	viewPortMat_ = MakeMatrix::Orthographic(0,0,(float)window->getWidth(),(float)window->getHeight(),0.0f,100.0f);
}

void Sprite::Finalize(){
	dxCommand_->Finalize();
}

Sprite *Sprite::Create(const Vector2 &pos,const Vector2 &size,const std::string &textureFilePath){
	Sprite *result = new Sprite();
	result->th_ = TextureManager::LoadTexture(textureFilePath);

	result->meshBuff_ = std::make_unique<SpriteMesh>();
	result->meshBuff_->Init();

	result->meshBuff_->vertexData[0] = {{0.0f,size.y,0.0f,1.0f},{0.0f,1.0f}};
	result->meshBuff_->vertexData[1] = {{0.0f,0.0f,0.0f,1.0f},{0.0f,0.0f}};
	result->meshBuff_->vertexData[2] = {{size.x,size.y,0.0f,1.0f},{1.0f,1.0f}};
	result->meshBuff_->vertexData[3] = {{size.x,0.0f,0.0f,1.0f},{1.0f,0.0f}};

	result->mappingConstBufferData_ = nullptr;
	result->constBuff_.CreateBufferResource(System::getInstance()->getDxDevice(),sizeof(SpritConstBuffer));

	result->constBuff_.getResource()->Map(
		0,nullptr,reinterpret_cast<void **>(&result->mappingConstBufferData_)
	);
	result->mappingConstBufferData_->color_ = {1.0f,1.0f,1.0f,1.0f};

	result->worldMat_ = MakeMatrix::Affine({1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{pos.x,pos.y,1.0f});

	return result;
}

void Sprite::CreatePSO(){
	ShaderManager *shaderManager = ShaderManager::getInstance();

	shaderManager->LoadShader("Sprite.VS");
	shaderManager->LoadShader("Sprite.PS",shaderDirectory,L"ps_6_0");

	ShaderInformation shaderInfo{};
	shaderInfo.vsKey = "Sprite.VS";
	shaderInfo.psKey = "Sprite.PS";

	///================================================
	/// Sampler の設定
	///================================================
	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;// バイナリニアフィルタ
	// 0 ~ 1 の間をリピート
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.MinLOD = 0;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	shaderInfo.pushBackSamplerDesc(sampler);

	///================================================
	/// RootParameter の設定
	///================================================
	D3D12_ROOT_PARAMETER rootParameters[2] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].Descriptor.ShaderRegister = 0;
	shaderInfo.pushBackRootParameter(rootParameters[0]);

	D3D12_DESCRIPTOR_RANGE descriptorRange = {};
	descriptorRange.BaseShaderRegister = 0;
	descriptorRange.NumDescriptors = 1;
	// SRV を扱うように設定
	descriptorRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	// offset を自動計算するように 設定
	descriptorRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	// DescriptorTable を使う
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	size_t rootIndex = shaderInfo.pushBackRootParameter(rootParameters[1]);
	shaderInfo.SetDescriptorRange2Parameter(&descriptorRange,1,rootIndex);

	///================================================
	/// InputElement の設定
	///================================================
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";/*Semantics*/
	inputElementDescs[0].SemanticIndex = 0;/*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//float 4
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	shaderInfo.pushBackInputElementDesc(inputElementDescs[0]);

	inputElementDescs[1].SemanticName = "TEXCOORD";/*Semantics*/
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	shaderInfo.pushBackInputElementDesc(inputElementDescs[1]);

	std::string psoKeys[kBlendNum] = {
		"Sprite_None",
		"Sprite_Alpha",
		"Sprite_Add",
		"Sprite_Sub",
		"Sprite_Multiply",
		"Sprite_Screen"
	};

	for(size_t i = 0; i < kBlendNum; i++){
		pso_[i] = shaderManager->CreatePso(psoKeys[i],shaderInfo,System::getInstance()->getDxDevice()->getDevice());
	}
}

void Sprite::Draw(){
	auto commandList = dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(pso_[(int)currentBlend_]->rootSignature.Get());
	commandList->SetPipelineState(pso_[(int)currentBlend_]->pipelineState.Get());
	dxCommand_->getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0,1,&meshBuff_->vbView);
	commandList->IASetIndexBuffer(&meshBuff_->ibView);

	mappingConstBufferData_->mat_ = worldMat_ * viewPortMat_;
	mappingConstBufferData_->uvMat_ = MakeMatrix::Affine(uvScale,uvRotate,uvTranslate);

	commandList->SetGraphicsRootConstantBufferView(
		0,constBuff_.getResource()->GetGPUVirtualAddress()
	);

	ID3D12DescriptorHeap *ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	commandList->SetDescriptorHeaps(1,ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(
		1,
		TextureManager::getDescriptorGpuHandle(th_)
	);
	commandList->SetGraphicsRootDescriptorTable(1,TextureManager::getDescriptorGpuHandle(th_));
	commandList->SetGraphicsRootConstantBufferView(0,constBuff_.getResource()->GetGPUVirtualAddress());

	commandList->DrawIndexedInstanced(6,1,0,0,0);
}

void Sprite::setSize(const Vector2 &size){
	meshBuff_->vertexData[0].pos = {0.0f,size.y,0.0f,1.0f};
	meshBuff_->vertexData[1].pos = {0.0f,0.0f,0.0f,1.0f};
	meshBuff_->vertexData[2].pos = {size.x,size.y,0.0f,1.0f};
	meshBuff_->vertexData[3].pos = {size.x,0.0f,0.0f,1.0f};
}

void Sprite::setPos(const Vector2 &pos){
	worldMat_ = MakeMatrix::Affine({1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{pos,0.0f});
}

void Sprite::SpriteMesh::Init(){
	const size_t vertexBufferSize = sizeof(SpriteVertexData) * 4;
	const size_t indexBufferSize = sizeof(uint32_t) * 6;

	// バッファのリソースを作成
	auto dxDevice = System::getInstance()->getDxDevice();
	vertBuff.CreateBufferResource(dxDevice,vertexBufferSize);
	indexBuff.CreateBufferResource(dxDevice,indexBufferSize);

	// 頂点バッファビューの設定
	vbView.BufferLocation = vertBuff.getResource()->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertexBufferSize;
	vbView.StrideInBytes = sizeof(SpriteVertexData);

	// 頂点バッファをマップ
	vertBuff.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&vertexData));

	// インデックスバッファビューの設定
	ibView.BufferLocation = indexBuff.getResource()->GetGPUVirtualAddress();
	ibView.SizeInBytes = indexBufferSize;
	ibView.Format = DXGI_FORMAT_R32_UINT;

	// インデックスバッファをマップ
	indexBuff.getResource()->Map(0,nullptr,reinterpret_cast<void **>(&indexData));

	// インデックスデータの設定
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;
}