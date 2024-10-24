#include "System.h"

#include "Audio/Audio.h"
#include "directX12/dxFunctionHelper/DxFunctionHelper.h"
#include "directX12/dxHeap/DxHeap.h"
#include "imgui/imgui.h"
#include "imGuiManager/ImGuiManager.h"
#include "primitiveDrawer/PrimitiveDrawer.h"
#include "sprite/Sprite.h"
#include "texture/RenderTexture.h"
#include "texture/TextureManager.h"

#include "directX12/dxResource/rtv/DxRtvArrayManager.h"
#include "directX12/dxResource/srv/DxSrvArrayManager.h"

#include "logger/Logger.h"

#define _USE_MATH_DEFINES
#include <cmath>

//.hに書いてはいけない
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"dinput8.lib")

System* System::getInstance(){
	static System instance;
	return &instance;
}

void System::Init(){
	window_ = std::make_unique<WinApp>();
	window_->CreateGameWindow(L"title",WS_OVERLAPPEDWINDOW,1280,720);

	input_ = Input::getInstance();
	input_->Init();

	Audio::StaticInit();

	dxDevice_ = std::make_unique<DxDevice>();
	dxDevice_->Init();

	DxHeap* dxHeap = DxHeap::getInstance();
	dxHeap->Init(dxDevice_->getDevice());

	dxCommand_ = std::make_unique<DxCommand>();
	dxCommand_->Init(dxDevice_->getDevice(),"main","main");

	dxSwapChain_ = std::make_unique<DxSwapChain>();
	dxSwapChain_->Init(window_.get(),dxDevice_.get(),dxCommand_.get());

	dxDsv_ = std::make_unique<DxDsv>();
	dxDsv_->Init(dxDevice_->getDevice(),dxHeap->getDsvHeap(),window_->getWidth(),window_->getHeight());

	DxSrvArrayManager::getInstance()->Init();

	DxRtvArrayManager::getInstance()->Init();

	dxFence_ = std::make_unique<DxFence>();
	dxFence_->Init(dxDevice_->getDevice());

	ShaderManager::getInstance()->Init();

	texturePsoKeys_ = {
		"Tex_Blend_None",
		"Tex_Blend_Normal",
		"Tex_Blend_Add",
		"Tex_Blend_Sub",
		"Tex_Blend_Multiply",
		"Tex_Blend_Screen"
	};

	CreateTexturePSO();

	ImGuiManager::getInstance()->Init(window_.get(),dxDevice_.get(),dxSwapChain_.get());

	TextureManager::Init();

	directionalLight_ = std::make_unique<DirectionalLight>();
	directionalLight_->Init();
	directionalLight_->ConvertToBuffer();

	pointLight_ = std::make_unique<PointLight>();
	pointLight_->Init();
	pointLight_->ConvertToBuffer();

	spotLight_ = std::make_unique<SpotLight>();
	spotLight_->Init();
	spotLight_->ConvertToBuffer();

	PrimitiveDrawer::Init();
	Model::Init();
	Sprite::Init();
	RenderTexture::Awake();

	materialManager_ = std::make_unique<MaterialManager>();

	deltaTime_ = std::make_unique<DeltaTime>();
	deltaTime_->Init();
}

void System::Finalize(){
	directionalLight_->Finalize();
	pointLight_->Finalize();
	spotLight_->Finalize();
	materialManager_->Finalize();

	ShaderManager::getInstance()->Finalize();
	PrimitiveDrawer::Finalize();
	Sprite::Finalize();
	Model::Finalize();
	TextureManager::Finalize();

	DxSrvArrayManager::getInstance()->Finalize();
	DxRtvArrayManager::getInstance()->Finalize();
	dxDsv_->Finalize();

	DxHeap::getInstance()->Finalize();
	dxSwapChain_->Finalize();
	dxCommand_->Finalize();
	DxCommand::ResetAll();
	dxFence_->Finalize();
	dxDevice_->Finalize();

	input_->Finalize();
	Audio::StaticFinalize();

#ifdef _DEBUG
	ImGuiManager::getInstance()->Finalize();
#endif // _DEBUG
}

void System::CreateTexturePSO(){
	ShaderManager* shaderManager_ = ShaderManager::getInstance();
	///=================================================
	/// shader読み込み
	///=================================================
	shaderManager_->LoadShader("Object3dTexture.VS");
	shaderManager_->LoadShader("Object3dTexture.PS",shaderDirectory,L"ps_6_0");

	///=================================================
	/// shader情報の設定
	///=================================================
	ShaderInfo texShaderInfo{};
	texShaderInfo.vsKey = "Object3dTexture.VS";
	texShaderInfo.psKey = "Object3dTexture.PS";

#pragma region"RootParameter"
	D3D12_ROOT_PARAMETER rootParameter[7]{};
	// TransformBuffer ... 0
	rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameter[0].Descriptor.ShaderRegister = 0;
	texShaderInfo.pushBackRootParameter(rootParameter[0]);
	// CameraBuffer ... 1
	rootParameter[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameter[1].Descriptor.ShaderRegister = 2;
	texShaderInfo.pushBackRootParameter(rootParameter[1]);
	// Material ... 2
	rootParameter[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[2].Descriptor.ShaderRegister = 0;
	texShaderInfo.pushBackRootParameter(rootParameter[2]);
	// DirectionalLight ... 3
	rootParameter[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[3].Descriptor.ShaderRegister = 1;
	texShaderInfo.pushBackRootParameter(rootParameter[3]);
	// PointLight ... 4
	rootParameter[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[4].Descriptor.ShaderRegister = 3;
	texShaderInfo.pushBackRootParameter(rootParameter[4]);
	// SpotLight ... 5
	rootParameter[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameter[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameter[5].Descriptor.ShaderRegister = 4;
	texShaderInfo.pushBackRootParameter(rootParameter[5]);

	// Texture ... 6
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	// SRV を扱うように設定
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	// offset を自動計算するように 設定
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// DescriptorTable を使う
	rootParameter[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	size_t rootParameterIndex = texShaderInfo.pushBackRootParameter(rootParameter[6]);
	texShaderInfo.SetDescriptorRange2Parameter(descriptorRange,1,rootParameterIndex);
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
	texShaderInfo.pushBackSamplerDesc(staticSampler);
	/// Sampler
	///=================================================

#pragma region "InputElement"
	D3D12_INPUT_ELEMENT_DESC inputElementDesc = {};
	inputElementDesc.SemanticName = "POSITION";/*Semantics*/
	inputElementDesc.SemanticIndex = 0;/*Semanticsの横に書いてある数字(今回はPOSITION0なので 0 )*/
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;//float 4
	inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	texShaderInfo.pushBackInputElementDesc(inputElementDesc);

	inputElementDesc.SemanticName = "TEXCOORD";/*Semantics*/
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	texShaderInfo.pushBackInputElementDesc(inputElementDesc);

	inputElementDesc.SemanticName = "NORMAL";/*Semantics*/
	inputElementDesc.SemanticIndex = 0;
	inputElementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDesc.AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	texShaderInfo.pushBackInputElementDesc(inputElementDesc);
#pragma endregion

	///=================================================
	/// BlendMode ごとの Psoを作成
	///=================================================
	for(size_t i = 0; i < kBlendNum; ++i){
		texShaderInfo.blendMode_ = static_cast<BlendMode>(i);
		shaderManager_->CreatePso(texturePsoKeys_[i],texShaderInfo,dxDevice_->getDevice());
	}
}

bool System::ProcessMessage(){
	return window_->ProcessMessage();
}

void System::BeginFrame(){
	ImGuiManager::getInstance()->Begin();
	input_->Update();
	PrimitiveDrawer::ResetInstanceVal();

	PrimitiveDrawer::setBlendMode(BlendMode::Alpha);
	Sprite::setBlendMode(BlendMode::Alpha);
}

void System::EndFrame(){
}

void System::ScreenPreDraw(){
	DxFH::PreDraw(dxCommand_.get(),window_.get(),dxSwapChain_.get());
}

void System::ScreenPostDraw(){
	ImGuiManager::getInstance()->End();
	ImGuiManager::getInstance()->Draw();
	
	HRESULT hr;
	ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();
	///===============================================================
	///	バリアの更新(描画->表示状態)
	///===============================================================
	ResourceBarrierManager::Barrier(
		commandList,
		dxSwapChain_->getCurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT
	);
	///===============================================================

	// コマンドの受付終了 -----------------------------------
	hr = commandList->Close();
	//----------------------------------------------------

	///===============================================================
	/// コマンドリストの実行
	///===============================================================
	ID3D12CommandList* ppHeaps[] = {commandList};
	dxCommand_->getCommandQueue()->ExecuteCommandLists(1,ppHeaps);
	///===============================================================

	dxSwapChain_->Present();

	// Frame Lock
	deltaTime_->Update();
	if(deltaTime_->getDeltaTime() >= 1.0f / fps_)
	{
		while(deltaTime_->getDeltaTime() >= 1.0f / fps_)
		{
			deltaTime_->Update();
		}
	}
	///===============================================================
	/// コマンドリストの実行を待つ
	///===============================================================
	dxFence_->Signal(dxCommand_->getCommandQueue());
	dxFence_->WaitForFence();
	///===============================================================

	///===============================================================
	/// リセット
	///===============================================================
	dxCommand_->CommandReset();
	///===============================================================
}

int System::LoadTexture(const std::string& filePath){
	return TextureManager::LoadTexture(filePath);
}