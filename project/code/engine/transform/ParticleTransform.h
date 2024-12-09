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

	Vector3 scale,rotate,translate;
	Matrix4x4 worldMat;
	Vector3 uvScale,uvRotate,uvTranslate;
	Matrix4x4 uvMat;

	Vector4 color;

	ParticleTransform* parent = nullptr;

	void UpdateMatrix();
public:
	struct ConstantBuffer{
		Matrix4x4 worldMat;
		Matrix4x4 uvMat;
		Vector4 color;

		ConstantBuffer& operator=(const ParticleTransform& transform){
			worldMat = transform.worldMat;
			uvMat    = transform.uvMat;
			color 	 = transform.color;
			return *this;
		}
	};
};

