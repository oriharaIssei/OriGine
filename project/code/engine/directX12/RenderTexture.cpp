#include "RenderTexture.h"

#include "directX12/DxFunctionHelper.h"
#include "directX12/DxHeap.h"
#include "directX12/ResourceBarrierManager.h"
#include "directX12/ShaderManager.h"

#include "Engine.h"

#include "Vector2.h"

PipelineStateObj* RenderTexture::pso_;

void RenderTexture::Awake(){
	ShaderManager* shaderManager = ShaderManager::getInstance();

	shaderManager->LoadShader("CopyImage.VS");
	shaderManager->LoadShader("CopyImage.PS",shaderDirectory,L"ps_6_0");

	ShaderInformation shaderInfo{};
	shaderInfo.vsKey = "CopyImage.VS";
	shaderInfo.psKey = "CopyImage.PS";

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
	// Texture だけ 
	D3D12_ROOT_PARAMETER rootParameter = {};
	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	// SRV を扱うように設定
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	// offset を自動計算するように 設定
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// DescriptorTable を使う
	rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	size_t rootParameterIndex = shaderInfo.pushBackRootParameter(rootParameter);
	shaderInfo.SetDescriptorRange2Parameter(descriptorRange,1,rootParameterIndex);

	///================================================
	/// InputElement の設定
	///================================================

	// 特に使わない

	///================================================
	/// depthStencil の設定
	///================================================
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = false;
	shaderInfo.setDepthStencilDesc(depthStencilDesc);

	pso_ = shaderManager->CreatePso("copyImage",shaderInfo,Engine::getInstance()->getDxDevice()->getDevice());
}

void RenderTexture::Init(const Vector2& textureSize,DXGI_FORMAT _format,const Vector4& _clearColor){
	textureSize_ = textureSize;
	clearColor_ = _clearColor;
	///===========================================================================
	/// RenderTexture Resource の作成
	///===========================================================================
	ID3D12Device* device = Engine::getInstance()->getDxDevice()->getDevice();

	resource_.CreateRenderTextureResource(device,static_cast<uint32_t>(textureSize_.x),static_cast<uint32_t>(textureSize_.y),_format,clearColor_);

	///===========================================================================
	///  RTV の作成
	///===========================================================================
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	rtvIndex_ = rtvArray_->CreateView(device,rtvDesc,resource_.getResource());

	///===========================================================================
	///  SRV の作成
	///===========================================================================
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	srvIndex_ = srvArray_->CreateView(device,srvDesc,resource_.getResource());

	ResourceBarrierManager::RegisterReosurce(resource_.getResource(),D3D12_RESOURCE_STATE_RENDER_TARGET);
}

void RenderTexture::PreDraw(){
	ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();

	DxHeap* heap = DxHeap::getInstance();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = heap->getRtvCpuHandle(rtvArray_->getLocationOnHeap(rtvIndex_));
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = heap->getDsvCpuHandle(0);

///=========================================
//	TransitionBarrier の 設定
///=========================================
	ResourceBarrierManager::Barrier(commandList,resource_.getResource(),D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->OMSetRenderTargets(1,&rtvHandle,FALSE,&dsvHandle);

///=========================================
//	Clear RTV
///=========================================
	float clearColor[4] = {
		clearColor_.x,
		clearColor_.y,
		clearColor_.z,
		clearColor_.w
	};
	commandList->ClearRenderTargetView(
		rtvHandle,clearColor,0,nullptr
	);

///=========================================
//	Clear DSV
///=========================================
	commandList->ClearDepthStencilView(
		dsvHandle,D3D12_CLEAR_FLAG_DEPTH,1.0f,0,0,nullptr
	);

///=========================================
//	ビューポート の 設定
///=========================================
	D3D12_VIEWPORT viewPort{};
	viewPort.Width = textureSize_.x;
	viewPort.Height = textureSize_.y;
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;

	commandList->RSSetViewports(1,&viewPort);
///=========================================
//	シザーレクト の 設定
///=========================================
	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = static_cast<LONG>(textureSize_.x);
	scissorRect.top = 0;
	scissorRect.bottom = static_cast<LONG>(textureSize_.y);

	commandList->RSSetScissorRects(1,&scissorRect);

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void RenderTexture::PostDraw(){
	HRESULT hr;
	ID3D12GraphicsCommandList* commandList = dxCommand_->getCommandList();
	DxFence* fence = Engine::getInstance()->getDxFence();

	///===============================================================
	///	バリアの更新(描画->表示状態)
	///===============================================================
	ResourceBarrierManager::Barrier(commandList,resource_.getResource(),D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	///===============================================================

	// コマンドの受付終了 -----------------------------------
	hr = commandList->Close();
	assert(SUCCEEDED(hr));
	//----------------------------------------------------

	///===============================================================
	/// コマンドリストの実行
	///===============================================================
	ID3D12CommandList* ppHeaps[] = {commandList};
	dxCommand_->getCommandQueue()->ExecuteCommandLists(1,ppHeaps);
	///===============================================================

	///===============================================================
	/// コマンドリストの実行を待つ
	///===============================================================
	fence->Signal(dxCommand_->getCommandQueue());
	fence->WaitForFence();
	///===============================================================

	///===============================================================
	/// リセット
	///===============================================================
	dxCommand_->CommandReset();
	///===============================================================
}

void RenderTexture::DrawTexture(){
	auto commandList = dxCommand_->getCommandList();

	commandList->SetGraphicsRootSignature(pso_->rootSignature.Get());
	commandList->SetPipelineState(pso_->pipelineState.Get());
	
	ID3D12DescriptorHeap* ppHeaps[] = {DxHeap::getInstance()->getSrvHeap()};
	commandList->SetDescriptorHeaps(1,ppHeaps);
	commandList->SetGraphicsRootDescriptorTable(
		0,
		DxHeap::getInstance()->getSrvGpuHandle(srvArray_->getLocationOnHeap(srvIndex_))
	);

	commandList->DrawInstanced(6,1,0,0);
}
