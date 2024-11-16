#pragma once

#include <stdint.h>

#include "directX12/IConstantBuffer.h"
#include "transform/CameraTransform.h"

class Model;
class Transform;
class Particle{
public:
	void Init();
	void Update();
	void Draw(IConstantBuffer<CameraTransform> camera);
private:
	// 形状
	Model* model_;
	// Texture
	uint32_t textureIndex_;
	// 位置，サイズ
	Transform* transform_;

	float lifeTime_;
};