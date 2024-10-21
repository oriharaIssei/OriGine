#include "Emitter.h"

#include "directX12/dxResource/srv/DxSrvArrayManager.h"

#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "directX12/ShaderCompiler.h"
#include "logger/Logger.h"
#include "System.h"

#include "directX12/dxResource/DxResource.h"
#include "texture/TextureManager.h"

#include <numbers>

#include "imgui/imgui.h"

std::unique_ptr<PipelineStateObj> Emitter::pso_;


void Emitter::Init(uint32_t instanceValue,MaterialManager *materialManager){
	originPos_ = {};

	material_ = materialManager->Create("Particle");

	std::random_device randomDevice;
	randomEngine_ = std::mt19937(randomDevice());

	particleSize_ = instanceValue;
	// pso 作成
	pso_ = std::make_unique<PipelineStateObj>();
	CreatePso();

	DxDevice *device = System::getInstance()->getDxDevice();

	// command系の初期化
	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(device->getDevice(),"main","main");

	dxSrvArray_ = DxSrvArrayManager::getInstance()->Create(1);
	std::unique_ptr<DxResource> resource;
	/// Resource の作成
	resource->CreateBufferResource(device,sizeof(ParticleStructuredBuffer) * instanceValue);

	resource->getResource()->Map(0,nullptr,reinterpret_cast<void **>(&mappingData_));
	Vector3 scale = {1.0f,1.0f,1.0f};
	Vector3 rotate = {0.0f,0.0f,0.0f};
	for(size_t i = 0; i < instanceValue; i++){
		mappingData_[i].TransformBuffer = MakeMatrix::Affine(scale,rotate,{i * 0.1f,i * 0.1f,i * 0.1f});
		mappingData_[i].color = {1.0f,1.0f,1.0f,1.0f};
	}

	// viewの作成
	D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc{};
	viewDesc.Format = DXGI_FORMAT_UNKNOWN;
	viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	viewDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	viewDesc.Buffer.FirstElement = 0;
	viewDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	viewDesc.Buffer.NumElements = instanceValue;
	viewDesc.Buffer.StructureByteStride = sizeof(TransformBuffer::ConstantBuffer);

	dxSrvArray_->CreateView(device->getDevice(),viewDesc,resource->getResource());

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

void Emitter::Update(){
	for(auto &particle : particles_){
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
	pso_.reset();
}

void Emitter::Draw(const CameraBuffer &CameraBuffer){
	auto *commandList = dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
	commandList->SetPipelineState(pso_->pipelineState.Get());

	commandList->IASetVertexBuffers(0,1,&meshBuff_->vbView);
	commandList->IASetIndexBuffer(&meshBuff_->ibView);

	ID3D12DescriptorHeap *ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	commandList->SetDescriptorHeaps(1,ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(
		4,
		TextureManager::getDescriptorGpuHandle(0)
	);

	Matrix4x4 cameraRotation = CameraBuffer.viewMat;
	for(size_t i = 0; i < 3; i++){
		cameraRotation[3][i] = 0.0f;
	}
	cameraRotation[3][3] = 1.0f;

	// view から world (plane.obj等の場合，rotateY(pi)を乗算したりする
	Matrix4x4 rotateMat = cameraRotation.Inverse();

	for(size_t i = 0; i < particles_.size(); i++){
		mappingData_[i].TransformBuffer = MakeMatrix::Scale({1.0f,1.0f,1.0f}) * rotateMat * MakeMatrix::Translate(originPos_ + particles_[i]->pos);
	}

	commandList->SetGraphicsRootDescriptorTable(0,DxHeap::getInstance()->getSrvGpuHandle(dxSrvArray_->getLocationOnHeap(srvIndex_)));

	CameraBuffer.SetForRootParameter(commandList,1);

	material_->SetForRootParameter(commandList,2);
	System::getInstance()->getDirectionalLight()->SetForRootParameter(commandList,3);

	// 描画!!!
	commandList->DrawIndexedInstanced(6,particleSize_,0,0,0);
}

void Emitter::CreatePso(){
	ID3D12Device *device = System::getInstance()->getDxDevice()->getDevice();

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

	HRESULT hr;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	D3D12_BLEND_DESC blendDesc{};
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

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

	// パラメーターをDESCにセット
	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	//シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

	hr = D3D12SerializeRootSignature(
		&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&signatureBlob,
		&errorBlob
	);
	if(FAILED(hr)){
		Logger::OutputLog(reinterpret_cast<char *>(errorBlob->GetBufferPointer()));
		assert(false);
	}

	//バイナリをもとに作成
	device->CreateRootSignature(
		0,
		signatureBlob->GetBufferPointer(),
		signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&pso_->rootSignature)
	);
	assert(SUCCEEDED(hr));

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

	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].LogicOpEnable = false;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	//裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	//三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	ShaderCompiler compiler;
	compiler.Init();

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = compiler.CompileShader(L"./Code/System/Shader/Particle.VS.hlsl",L"vs_6_0");
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = compiler.CompileShader(L"./Code/System/Shader/Particle.PS.hlsl",L"ps_6_0");
	assert(pixelShaderBlob != nullptr);

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = pso_->rootSignature.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = {
		vertexShaderBlob->GetBufferPointer(),
		vertexShaderBlob->GetBufferSize()
	};
	graphicsPipelineStateDesc.PS = {
		pixelShaderBlob->GetBufferPointer(),
		pixelShaderBlob->GetBufferSize()
	};

	// DepthStancilState の設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depth の機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込み
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual (近ければ描画する)
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// DepthStencil の設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	// 書き込む用のRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)タイプ。三角形を設定する
	graphicsPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	// 生成
	hr = device->CreateGraphicsPipelineState(
		&graphicsPipelineStateDesc,
		IID_PPV_ARGS(&pso_->pipelineState)
	);
	assert(SUCCEEDED(hr));
}