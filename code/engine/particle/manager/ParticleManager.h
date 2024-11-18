#pragma once

#include <array>
#include <memory>
#include <string>

#include "../emitter/Emitter.h"
#include "directX12/DxCommand.h"
#include "directX12/DxSrvArray.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "transform/CameraTransform.h"

class ParticleManager{
public:
	static ParticleManager* getInstance();
	void Init();
	void PreDraw();
private:
	void CreatePso();
private:
	int32_t srvNum_ = 3;
	std::shared_ptr<DxSrvArray> dxSrvArray_;

	std::unique_ptr<DxCommand> dxCommand_;

	std::string psoKey_;
	PipelineStateObj* pso_;
};