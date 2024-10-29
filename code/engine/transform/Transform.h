#pragma once

#include <array>
#include <d3d12.h>
#include <functional>
#include <string>
#include <wrl.h>

#include "Matrix4x4.h"

struct Transform{
public:
	Transform(){}
	Transform(const Vector3& _scale,const Vector3& _rotate,const Vector3& _translate):scale(_scale),rotate(_rotate),translate(_translate){}
	~Transform(){}

	void Init();
	void UpdateMatrix();

	/// <summary>
	/// ImGuiでの要素表示関数(Matrixの更新はしない)
	/// </summary>
	void Debug(const std::string& transformName);

	Vector3   scale     = {1.0f,1.0f,1.0f};
	Vector3   rotate    = {0.0f,0.0f,0.0f};
	Vector3   translate = {0.0f,0.0f,0.0f};
	Matrix4x4 worldMat;

	Transform* parent = nullptr;
private:
	Matrix4x4 CalculateWithParent(const Transform* parent);

public:
	struct ConstantBuffer{
		Matrix4x4 world;
		ConstantBuffer& operator=(const Transform& transform){ world = transform.worldMat; return *this; }
	};
};