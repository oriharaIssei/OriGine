#pragma once

#include "Particle.h"

#include <random>

#include <memory>

#include <vector>

#include "directX12/dxCommand/DxCommand.h"
#include "directX12/dxResource/srv/DxSrvArray.h"
#include "directX12/PipelineStateObj.h"
#include "directX12/dxResource/srv/DxSrvArray.h"
#include "directX12/PipelineStateObj.h"

#include "directX12/buffer/Material.h"
#include "directX12/buffer/Object3dMesh.h"
#include "directX12/buffer/CameraBuffer.h"

#include "Vector3.h"
#include <stdint.h>

class Emitter{
public:
	~Emitter(){ Finalize(); }
	void Init(uint32_t instanceValue,MaterialManager *materialManager);
	void Update();
	void Draw(const CameraBuffer &CameraBuffer);
	void Finalize();
private:
	void CreatePso();
private:
	static std::unique_ptr<PipelineStateObj> pso_;

	Vector3 originPos_;

	std::mt19937 randomEngine_;

	std::vector<std::unique_ptr<Particle>> particles_;

	ParticleStructuredBuffer *mappingData_;

	uint32_t srvIndex_;
	uint32_t particleSize_;

	Material *material_;

	std::shared_ptr<DxSrvArray> dxSrvArray_;
	std::unique_ptr<TextureObject3dMesh> meshBuff_;
	std::unique_ptr<DxCommand> dxCommand_;

};