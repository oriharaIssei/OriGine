#pragma once

#include "Particle.h"

#include <random>

#include <memory>

#include <vector>

#include "directX12/DxCommand.h"
#include "directX12/DxSrvArray.h"
#include "directX12/Object3dMesh.h"
#include "directX12/PipelineStateObj.h"
#include "material/Material.h"
#include "transform/CameraTransform.h"

#include "Vector3.h"
#include <stdint.h>

class Emitter{
public:
	~Emitter(){ Finalize(); }
	void Init(uint32_t instanceValue,MaterialManager *materialManager);
	void Update(const CameraTransform& cameraTransform);
	void Draw(const IConstantBuffer<CameraTransform> &CameraTransform);
	void Finalize();
private:
	void CreatePso();
private:
	PipelineStateObj* pso_;
	std::unique_ptr<DxCommand> dxCommand_;

	Vector3 originPos_;

	std::mt19937 randomEngine_;

	std::vector<std::unique_ptr<Particle>> particles_;

	IStructuredBuffer<ParticleStructuredBuffer> particleBuff_;
	IConstantBuffer<Material>* material_;

	uint32_t srvIndex_;
	uint32_t particleSize_;

	std::shared_ptr<DxSrvArray> dxSrvArray_;
	std::unique_ptr<TextureObject3dMesh> meshBuff_;
};