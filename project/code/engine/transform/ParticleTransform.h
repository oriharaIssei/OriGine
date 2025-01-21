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

		this->uvScale 	= other.uvScale;
		this->uvRotate 	= other.uvRotate;
		this->uvTranslate = other.uvTranslate;

		color = other.color;

		UpdateMatrix();
	}
	~ParticleTransform() = default;

	Vec3f scale,rotate,translate;
	Matrix4x4 worldMat;
    Matrix4x4* parentWorldMat = nullptr;

	Vec3f uvScale,uvRotate,uvTranslate;
	Matrix4x4 uvMat;

	Vec4f color;

	void UpdateMatrix();
public:
	struct ConstantBuffer{
		Matrix4x4 worldMat;
		Matrix4x4 uvMat;
		Vec4f color;

		ConstantBuffer& operator=(const ParticleTransform& transform){
			worldMat = transform.worldMat;
			uvMat    = transform.uvMat;
			color 	 = transform.color;
			return *this;
		}
	};
};

