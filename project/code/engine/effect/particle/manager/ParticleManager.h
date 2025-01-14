#pragma once

///stl
//memory
#include <memory>
//container
#include <array>
#include <vector>
#include <unordered_map>
//string
#include <string>

///engine
//dx12Object
#include "directX12/DxCommand.h"
#include "directX12/DxSrvArray.h"
#include "directX12/IConstantBuffer.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/ShaderManager.h"
//module
#include "module/IModule.h"
//lib
#include "globalVariables/SerializedField.h"
//transform
#include "transform/CameraTransform.h"

class Emitter;
class ParticleManager
    : public IModule {
	friend class Emitter;
public:
	static ParticleManager* getInstance();
	void Init();
	void Finalize();
	void PreDraw();

	void Edit();
	void DrawDebug();

private:
    ParticleManager();
    ~ParticleManager();
    ParticleManager(const ParticleManager&) = delete;
    const ParticleManager& operator=(const ParticleManager&) = delete;

	void CreatePso();
private:
	int32_t srvNum_ = 16;
	std::shared_ptr<DxSrvArray> dxSrvArray_;

	std::unique_ptr<DxCommand> dxCommand_;
	std::string psoKey_;
	PipelineStateObj* pso_;

	bool emitterWindowedState_ = false;

	std::unordered_map<std::string,std::unique_ptr<Emitter>> emitters_;

	// 新しい Emitterを 作成する ための もの
	bool isOpenedCrateWindow_ = false;
	std::string newInstanceName_ = "NULL";
public:
	std::unique_ptr<Emitter> CreateEmitter(DxSrvArray* srvArray,const std::string& name)const;

};
