#pragma once

#include "Matrix4x4.h"
#include "Vector3.h"
#include "Vector4.h"

struct ParticleTransform{
	ParticleTransform() = default;

	/// <summary>
	/// コピーコンストラクタ
	/// </summary>
	/// <param name="other"></param>
	ParticleTransform(const ParticleTransform& other){
		this->scale 	= other.scale;
		this->rotate 	= other.rotate;
		this->translate = other.translate;

		color = other.color;

		UpdateMatrix();
	}
	~ParticleTransform() = default;

	Vector3 scale,rotate,translate;
	Matrix4x4 worldMat;
	Vector4 color;

	ParticleTransform* parent = nullptr;

	void UpdateMatrix();
public:
	struct ConstantBuffer{
		Matrix4x4 world;
		Vector4 color;

		ConstantBuffer& operator=(const ParticleTransform& transform){
			world = transform.worldMat;
			color = transform.color;
			return *this;
		}
	};
};