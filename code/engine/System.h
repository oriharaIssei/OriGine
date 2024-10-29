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
#include "material/light/DirectionalLight.h"
#include "material/light/PointLight.h"
#include "material/light/SpotLight.h"
#include "model/Model.h"
#include "winApp/WinApp.h"

#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

class Material;
class MaterialManager;
class System{
	friend class PrimitiveDrawer;
public:
	static System* getInstance();
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
	System() = default;
	~System(){};
	System(const System&) = delete;
	const System& operator=(const System&) = delete;
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
	std::unique_ptr<IConstantBuffer<DirectionalLight>> directionalLight_;
	std::unique_ptr<IConstantBuffer<PointLight>>       pointLight_;
	std::unique_ptr<IConstantBuffer<SpotLight>>        spotLight_;

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

	float getDeltaTime()const{return deltaTime_->getDeltaTime();}

	IConstantBuffer<DirectionalLight>* getDirectionalLight()const{ return directionalLight_.get(); }
	IConstantBuffer<PointLight>*       getPointLight()const{ return pointLight_.get(); }
	IConstantBuffer<SpotLight>*        getSpotLight()const{ return spotLight_.get(); }
};

const std::string defaultResourceFolder = "./resource";