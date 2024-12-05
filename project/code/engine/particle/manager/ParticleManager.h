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


#ifdef _DEBUG
	void Edit();
	void DrawDebug(const IConstantBuffer<CameraTransform>& cameraTransform);
#endif // _DEBUG
private:
	void CreatePso();
private:
	int32_t srvNum_ = 16;
	std::shared_ptr<DxSrvArray> dxSrvArray_;

	std::unique_ptr<DxCommand> dxCommand_;
	std::string psoKey_;
	PipelineStateObj* pso_;

	bool emitterWindowedState_ = false;

	std::unordered_map<std::string,std::unique_ptr<Emitter>> emitters_;

#ifdef _DEBUG // Editor 用 変数
	// 新しい Emitterを 作成する ための もの
	bool isOpenedCrateWindow_ = false;
	std::string newInstanceName_ = "NULL";

	// 現在変更している Emitter
	Emitter* currentEditEmitter_ = nullptr;
	bool isUpdateCurrentEmitter_ = false;

#endif // _DEBUG
public:
	Emitter* getEmitter(const std::string& name)const;
};