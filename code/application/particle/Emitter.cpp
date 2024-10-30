#include "Emitter.h"

#include "directX12/DxSrvArrayManager.h"

#include "directX12/DxFunctionHelper.h"
#include "directX12/DxResource.h"
#include "directX12/ShaderManager.h"
#include "logger/Logger.h"
#include "material/TextureManager.h"
#include "System.h"

#include <numbers>

#include "imgui/imgui.h"


void Emitter::Init(uint32_t instanceValue,MaterialManager* materialManager){
	originPos_ = {};

	material_ = materialManager->Create("Particle");

	std::random_device randomDevice;
	randomEngine_ = std::mt19937(randomDevice());

	particleSize_ = instanceValue;
	CreatePso();

	DxDevice* device = System::getInstance()->getDxDevice();

	// command系の初期化
	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(device->getDevice(),"main","main");

	dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);
	
	particleBuff_.CreateBuffer(device->getDevice(),instanceValue);
	
	Vector3 scale = {1.0f,1.0f,1.0f};
	Vector3 rotate = {0.0f,0.0f,0.0f};
	for(size_t i = 0; i < instanceValue; i++){
		particleBuff_.openData_[i].transform = MakeMatrix::Affine(scale,rotate,{i * 0.1f,i * 0.1f,i * 0.1f});
		particleBuff_.openData_[i].color = {1.0f,1.0f,1.0f,1.0f};
	}

	// viewの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDesc.Buffer.FirstElement = 0;
	viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	viewDesc.Buffer.NumElements = instanceValue;
	viewDesc.Buffer.StructureByteStride = sizeof(Transform::ConstantBuffer);

	dxSrvArray_->CreateView(device->getDevice(),viewDesc,particleBuff_.getResource().getResource());

	meshBuff_ = std::make_unique<TextureObject3dMesh>();
	meshBuff_->Create(4,6);

	meshBuff_->vertData[0].pos = {0.0f,0.0f,0.f,1.0f};
	meshBuff_->vertData[0].normal = {0.0f,0.0f,0.0f};
	meshBuff_->vertData[0].texCoord = {0.0f,0.0f};
	meshBuff_->vertData[1].pos = {1.0f,0.0f,0.0f,1.0f};
	meshBuff_->vertData[1].normal = {1.0f,0.0f,0.0f};
	meshBuff_->vertData[1].texCoord = {1.0f,0.0f};
	meshBuff_->vertData[2].pos = {0.0f,-1.0f,0.0f,1.0f};
	meshBuff_->vertData[2].normal = {0.0f,-1.0f,0.0f};
	meshBuff_->vertData[2].texCoord = {0.0f,1.0f};
	meshBuff_->vertData[3].pos = {1.0f,-1.0f,0.0f,1.0f};
	meshBuff_->vertData[3].normal = {1.0f,-1.0f,0.0f};
	meshBuff_->vertData[3].texCoord = {1.0f,1.0f};

	meshBuff_->indexData[0] = 0;
	meshBuff_->indexData[1] = 1;
	meshBuff_->indexData[2] = 2;
	meshBuff_->indexData[3] = 1;
	meshBuff_->indexData[4] = 3;
	meshBuff_->indexData[5] = 2;

	for(size_t i = 0; i < instanceValue; i++){
		particles_.emplace_back(new Particle());
		particles_.back()->Init(randomEngine_);
	}
}

void Emitter::Update(const CameraTransform& cameraTransform){
	int32_t index = 0;

	for(auto& particle : particles_){
		if(!particle->isAlive_){
			particle->Init(randomEngine_);
		}
		particle->Update();
	}

}

void Emitter::Finalize(){
	meshBuff_.reset();
	dxCommand_.reset();
	dxSrvArray_.reset();
}

void Emitter::Draw(const IConstantBuffer<CameraTransform>& CameraTransform){
	auto* commandList = dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
	commandList->SetPipelineState(pso_->pipelineState.Get());

	commandList->IASetVertexBuffers(0,1,&meshBuff_->vbView);
	commandList->IASetIndexBuffer(&meshBuff_->ibView);

	ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	commandList->SetDescriptorHeaps(1,ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(
		4,
		TextureManager::getDescriptorGpuHandle(0)
	);

	Matrix4x4 cameraRotation = CameraTransform.openData_.viewMat;
	for(size_t i = 0; i < 3; i++){
		cameraRotation[3][i] = 0.0f;
	}
	cameraRotation[3][3] = 1.0f;

	// view から world (plane.obj等の場合，rotateY(pi)を乗算したりする
	Matrix4x4 rotateMat = cameraRotation.Inverse();

	for(size_t i = 0; i < particles_.size(); i++){
		particleBuff_.openData_[i].transform = MakeMatrix::Scale({1.0f,1.0f,1.0f}) * rotateMat * MakeMatrix::Translate(originPos_ + particles_[i]->pos);
	}
	particleBuff_.ConvertToBuffer();

	commandList->SetGraphicsRootDescriptorTable(0,DxHeap::getInstance()->getSrvGpuHandle(dxSrvArray_->getLocationOnHeap(srvIndex_)));

	CameraTransform.SetForRootParameter(commandList,1);

	material_->SetForRootParameter(commandList,2);
	System::getInstance()->getDirectionalLight()->SetForRootParameter(commandList,3);

	// 描画!!!
	commandList->DrawIndexedInstanced(6,particleSize_,0,0,0);
}

void Emitter::CreatePso(){
	ID3D12Device* device = System::getInstance()->getDxDevice()->getDevice();

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	ShaderManager* shaderManager  = ShaderManager::getInstance();
	shaderManager->LoadShader("Particle.VS","./resource/Shader",L"vs_6_0");
	shaderManager->LoadShader("Particle.PS","./resource/Shader",L"ps_6_0");

	ShaderInfo shaderInfo{};
	shaderInfo.vsKey = "Particle.VS";
	shaderInfo.psKey = "Particle.PS";
	///================================================
	/// Sampler の設定
	///================================================
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;// バイナリニアフィルタ
	// 0 ~ 1 の間をリピート
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;

	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MinLOD = 0;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	shaderInfo.pushBackSamplerDesc(staticSamplers[0]);

	///================================================
	/// RootParameter の設定
	///================================================
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	// SRV を扱うように設定
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	// offset を自動計算するように 設定
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE structuredRange[1] = {};
	structuredRange[0].BaseShaderRegister = 0;
	structuredRange[0].NumDescriptors = 1;
	// SRV を扱うように設定
	structuredRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	// offset を自動計算するように 設定
	structuredRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	D3D12_ROOT_PARAMETER rootParameters[5] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[0].DescriptorTable.pDescriptorRanges = structuredRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(structuredRange);

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 1;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].Descriptor.ShaderRegister = 0;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].Descriptor.ShaderRegister = 1;

	// DescriptorTable を使う
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[4].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	for(size_t i = 0; i < 5; i++){
		shaderInfo.pushBackRootParameter(rootParameters[i]);
	}
	shaderInfo.SetDescriptorRange2Parameter(structuredRange,1,0);
	shaderInfo.SetDescriptorRange2Parameter(descriptorRange,1,4);

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";/*Semantics*/
	inputElementDescs[0].SemanticIndex = 0;/*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//float 4
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";/*Semantics*/
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";/*Semantics*/
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	for(size_t i = 0; i < 3; i++){
		shaderInfo.pushBackInputElementDesc(inputElementDescs[i]);
	}
	shaderInfo.customDepthStencilDesc().DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	pso_ = shaderManager->CreatePso("Particle",shaderInfo,device);
}