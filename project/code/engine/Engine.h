#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "deltaTime/DeltaTime.h"
#include "directX12/DxCommand.h"
#include "directX12/DxCommand.h"
#include "directX12/DxDevice.h"
#include "directX12/DxDsv.h"
#include "directX12/DxFence.h"
#include "directX12/DxRtvArray.h"
#include "directX12/DxSwapChain.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "input/Input.h"
#include "material/light/lightManager.h"
#include "object3d/Object3d.h"
#include "winApp/WinApp.h"

#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

struct Material;
class MaterialManager;
class Engine{
	friend class PrimitiveDrawer;
public:
	static Engine* getInstance();
public:
	void Init();
	void Finalize();
	bool ProcessMessage();
	void BeginFrame();
	void EndFrame();

	void ScreenPreDraw();
	void ScreenPostDraw();

	int LoadTexture(const std::string& filePath);
private:
	Engine() = default;
	~Engine(){};
	Engine(const Engine&) = delete;
	const Engine& operator=(const Engine&) = delete;

	void CreateTexturePSO();
private:
	// api
	std::unique_ptr<WinApp> window_;
	Input* input_;

	// directX
	std::unique_ptr<DxDevice> dxDevice_;
	std::unique_ptr<DxCommand> dxCommand_;
	std::unique_ptr<DxSwapChain> dxSwapChain_;
	std::unique_ptr<DxFence> dxFence_;
	std::unique_ptr<DxDsv> dxDsv_;

	// PipelineState Object
	std::array<PipelineStateObj*,kBlendNum> texturePso_;
	std::array<std::string,kBlendNum> texturePsoKeys_;

	// buffers
	std::unique_ptr<MaterialManager> materialManager_;
	std::unique_ptr<LightManager> lightManager_;

	// Time
	std::unique_ptr<DeltaTime> deltaTime_;
	float fps_ = 60.0f;
public:
	WinApp* getWinApp(){ return window_.get(); }

	DxDevice* getDxDevice()const{ return dxDevice_.get(); }
	DxCommand* getDxCommand()const{ return dxCommand_.get(); }
	DxSwapChain* getDxSwapChain()const{ return dxSwapChain_.get(); }
	DxFence* getDxFence()const{ return dxFence_.get(); }

	DxDsv* getDsv()const{ return dxDsv_.get(); }

	MaterialManager* getMaterialManager()const{ return materialManager_.get(); }

	PipelineStateObj* getTexturePso(BlendMode blend)const{ return texturePso_[static_cast<size_t>(blend)]; }

	const std::array<std::string,kBlendNum>& getTexturePsoKeys()const{ return texturePsoKeys_; }

	float getDeltaTime()const{ return deltaTime_->getDeltaTime(); }

	LightManager* getLightManager()const{ return lightManager_.get(); }
};