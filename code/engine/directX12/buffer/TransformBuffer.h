#pragma once

#include <array>
#include <d3d12.h>
#include <functional>
#include <string>
#include <wrl.h>

#include "IConstantBuffer.h"
#include "Matrix4x4.h"

class TransformBuffer
	: public IConstantBuffer{
public:
	TransformBuffer():IConstantBuffer(){}
	~TransformBuffer()override{}

	void Init()override;
	void Update()override;
	void Finalize()override;
	void ConvertToBuffer()override;
	
	/// <summary>
	/// ImGuiでの要素表示関数(Matrixの更新はしない)
	/// </summary>
	void Debug(const std::string &transformName);

	Vector3   scale     = {1.0f,1.0f,1.0f};
	Vector3   rotate    = {0.0f,0.0f,0.0f};
	Vector3   translate = {0.0f,0.0f,0.0f};
	Matrix4x4 worldMat;

	TransformBuffer*parent = nullptr;

	struct ConstantBuffer
	{
		Matrix4x4 world;
	};
private:
	ConstantBuffer* mappingWorldMat_ = nullptr;
};