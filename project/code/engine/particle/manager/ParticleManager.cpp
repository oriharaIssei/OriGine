#include "ParticleManager.h"

#include "../emitter/Emitter.h"
#include "directX12/DxSrvArrayManager.h"
#include "Engine.h"

ParticleManager* ParticleManager::getInstance(){
	static ParticleManager instance;
	return &instance;
}

void ParticleManager::Init(){
	dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(srvNum_);

	psoKey_ = "Particle_Alpha";

	CreatePso();
}

void ParticleManager::Finalize(){
	if(dxCommand_){
		dxCommand_->Finalize();
	}
	if(dxSrvArray_){
		dxSrvArray_->Finalize();
	}
}

void ParticleManager::PreDraw(){
	auto* commandList = dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
	commandList->SetPipelineState(pso_->pipelineState.Get());
}

void ParticleManager::Draw(const IConstantBuffer<CameraTransform>& cameraTransform){
	for(auto& [name,emitter] : emitters_){
		emitter->Draw(cameraTransform);
	}
}

void ParticleManager::CreatePso(){
	ShaderManager* shaderManager = ShaderManager::getInstance();
	///=================================================
	/// shader読み込み
	///=================================================

	shaderManager->LoadShader("Particle.VS");
	shaderManager->LoadShader("Particle.PS",shaderDirectory,L"ps_6_0");

	///=================================================
	/// shader情報の設定
	///=================================================
	ShaderInfo shaderInfo;
	shaderInfo.vsKey = "Particle.VS";
	shaderInfo.psKey = "Particle.PS";

#pragma region"RootParameter"
	D3D12_ROOT_PARAMETER rootParameter[4]{};
	// 0 ... ParticleData
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[0].Descriptor.ShaderRegister = 0;
	shaderInfo.pushBackRootParameter(rootParameter[0]);

	D3D12_DESCRIPTOR_RANGE structuredRange[1] = {};
	structuredRange[0].BaseShaderRegister = 0;
	structuredRange[0].NumDescriptors = 1;
	structuredRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	structuredRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	shaderInfo.SetDescriptorRange2Parameter(structuredRange,1,0);
	// 1 ... ViewProjection
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[1].Descriptor.ShaderRegister = 1;
	shaderInfo.pushBackRootParameter(rootParameter[1]);
	// 2 ... Material
	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].Descriptor.ShaderRegister = 0;
	shaderInfo.pushBackRootParameter(rootParameter[2]);
	// 3 ... Texture
	// DescriptorTable を使う
	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	size_t rootParameterIndex = shaderInfo.pushBackRootParameter(rootParameter[3]);

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	shaderInfo.SetDescriptorRange2Parameter(descriptorRange,1,rootParameterIndex);

#pragma endregion

#pragma region"Input Element"
	D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
	inputElementDesc.SemanticName = "POSITION";/*Semantics*/
	inputElementDesc.SemanticIndex = 0;/*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//float 4
	inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	shaderInfo.pushBackInputElementDesc(inputElementDesc);
	inputElementDesc.SemanticName = "NORMAL";/*Semantics*/
	inputElementDesc.SemanticIndex = 0;/*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;//float 4
	inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	shaderInfo.pushBackInputElementDesc(inputElementDesc);
	inputElementDesc.SemanticName = "TEXCOORD";/*Semantics*/
	inputElementDesc.SemanticIndex = 0;/*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
	inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;//float 2
	inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	shaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

	///=================================================
	/// Sampler
	D3D12_STATIC_SAMPLER_DESC staticSampler = {};
	staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;// バイナリニアフィルタ
	// 0 ~ 1 の間をリピート
	staticSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSampler.MinLOD = 0;
	staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
	staticSampler.ShaderRegister = 0;
	staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	shaderInfo.pushBackSamplerDesc(staticSampler);
	/// Sampler
	///=================================================

	///=================================================
	/// BlendMode ごとの Pso作成
	///=================================================
	shaderInfo.blendMode_ = BlendMode::Normal;
	pso_ = shaderManager->CreatePso(psoKey_,shaderInfo,Engine::getInstance()->getDxDevice()->getDevice());
}

Emitter* ParticleManager::getEmitter(const std::string& name) const{
	auto emitter = emitters_.find(name);
	if(emitter != emitters_.end()){
		return emitter->second.get();
	}
	return nullptr;
}
