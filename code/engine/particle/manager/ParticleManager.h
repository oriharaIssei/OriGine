#pragma once

#include <memory>

#include "../emitter/Emitter.h"
#include "directX12/DxSrvArray.h"
#include "directX12/IConstantBuffer.h"
#include "transform/CameraTransform.h"

class ParticleManager{
public:
	void Init();
	void Update();
	void Draw(const IConstantBuffer<CameraTransform> camera);

private:
	std::shared_ptr<DxSrvArray> dxSrvArray_;
	std::list<std::unique_ptr<Emitter>> emitter_;
};

