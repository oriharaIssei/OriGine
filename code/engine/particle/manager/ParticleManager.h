#pragma once

#include <array>
#include <memory>
#include <string>
#include <unordered_map>

#include "directX12/DxCommand.h"
#include "directX12/DxSrvArray.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
#include "transform/CameraTransform.h"

class Emitter;
class ParticleManager{
	friend class Emitter;
public:
	static ParticleManager* getInstance();
	void Init();
	void Finalize();
	void PreDraw();

	void Draw(const IConstantBuffer<CameraTransform>& cameraTransform);
private:
	void CreatePso();
private:
	int32_t srvNum_ = 5;
	std::shared_ptr<DxSrvArray> dxSrvArray_;

	std::unique_ptr<DxCommand> dxCommand_;
	std::string psoKey_;
	PipelineStateObj* pso_;

	std::unordered_map<std::string,std::unique_ptr<Emitter>> emitters_;
public:
	Emitter* getEmitter(const std::string& name)const;
};