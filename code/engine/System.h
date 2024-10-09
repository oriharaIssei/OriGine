#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "input/Input.h"
#include "winApp/WinApp.h"

#include "directX12/dxCommand/DxCommand.h"
#include "directX12/dxDevice/DxDevice.h"
#include "directX12/dxFence/DxFence.h"
#include "directX12/dxResource/dsv/DxDsv.h"
#include "directX12/dxResource/rtv/DxRtvArray.h"
#include "directX12/dxResource/srv/DxSrvArray.h"
#include "directX12/dxSwapChain/DxSwapChain.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/shaderManager/ShaderManager.h"

#include "directX12/buffer/light/DirectionalLight.h"
#include "directX12/buffer/light/PointLight.h"
#include "directX12/buffer/light/SpotLight.h"
#include "directX12/buffer/Material.h"
#include "model/Model.h"

#include "Matrix4x4.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

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
	std::unique_ptr<WinApp> window_;
	Input* input_;

	std::unique_ptr<DxDevice> dxDevice_;

	std::unique_ptr<DxCommand> dxCommand_;
	std::unique_ptr<DxSwapChain> dxSwapChain_;
	std::unique_ptr<DxFence> dxFence_;

	std::unique_ptr<DxDsv> dxDsv_;

	std::array<PipelineStateObj*,kBlendNum> texturePso_;
	std::array<std::string,kBlendNum> texturePsoKeys_;

	std::unique_ptr<MaterialManager> materialManager_;

	std::unique_ptr<DirectionalLight> directionalLight_;
	std::unique_ptr<PointLight> pointLight_;
	std::unique_ptr<SpotLight> spotLight_;
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

	DirectionalLight* getDirectionalLight()const{ return directionalLight_.get(); }
	PointLight* getPointLight()const{ return pointLight_.get(); }
	SpotLight* getSpotLight()const{ return spotLight_.get(); }
};

const std::string defaultReosurceFolder = "./resource";