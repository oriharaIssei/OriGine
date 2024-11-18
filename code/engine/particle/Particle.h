#pragma once

#include <stdint.h>

#include "directX12/IConstantBuffer.h"
#include "transform/CameraTransform.h"

class Model;
class Transform;
class Particle{
public:
	Particle() = default;
	~Particle(){}

	void Init(Model* _model,uint32_t _textureIndex,Transform* _transform,float _lifeTime);
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